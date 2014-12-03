/*
   Copyright (c) 2009-2014, Jack Poulson, Lexing Ying,
   The University of Texas at Austin, Stanford University, and the
   Georgia Insitute of Technology.
   All rights reserved.
 
   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_OPTIMIZATION_REGLDL_PROCESSDISTTREE_HPP
#define EL_OPTIMIZATION_REGLDL_PROCESSDISTTREE_HPP

namespace El {
namespace reg_ldl {

template<typename F> 
inline void ProcessDistTree
( DistSymmInfo& info, DistSymmFrontTree<F>& L, Base<F> pivTol, 
  const DistNodalMultiVec<Base<F>>& regCand, DistNodalMultiVec<Base<F>>& reg )
{
    DEBUG_ONLY(CallStackEntry cse("reg_ldl::ProcessDistTree"))

    // The bottom front is already computed, so just view it
    SymmFront<F>& topLocFront = L.localFronts.back();
    DistSymmFront<F>& botDistFront = L.distFronts[0];
    const Grid& botGrid = *info.distNodes[0].grid;
    botDistFront.front2dL.LockedAttach
    ( topLocFront.frontL.Height(), topLocFront.frontL.Width(), botGrid, 0, 0, 
      topLocFront.frontL );
    botDistFront.work2d.LockedAttach
    ( topLocFront.work.Height(), topLocFront.work.Width(), botGrid, 0, 0, 
      topLocFront.work );
    botDistFront.diag1d.LockedAttach
    ( topLocFront.diag.Height(), topLocFront.diag.Width(), botGrid, 0, 0, 
      topLocFront.diag );

    // Perform the distributed portion of the factorization
    const Unsigned numDistNodes = info.distNodes.size();
    for( Unsigned s=1; s<numDistNodes; ++s )
    {
        const DistSymmNodeInfo& childNode = info.distNodes[s-1];
        const DistSymmNodeInfo& node = info.distNodes[s];
        const Int updateSize = node.lowerStruct.size();
        DistSymmFront<F>& childFront = L.distFronts[s-1];
        DistSymmFront<F>& front = L.distFronts[s];
        front.work2d.Empty();
        DEBUG_ONLY(
            if( front.front2dL.Height() != node.size+updateSize ||
                front.front2dL.Width() != node.size )
                LogicError("Front was not the proper size");
        )

        // Grab this front's grid information
        const Grid& grid = front.front2dL.Grid();
        mpi::Comm comm = grid.VCComm();
        const unsigned commSize = mpi::Size( comm );
        const unsigned gridHeight = grid.Height();
        const unsigned gridWidth = grid.Width();

        // Grab the child's grid information
        const Grid& childGrid = childFront.front2dL.Grid();
        const unsigned childGridHeight = childGrid.Height();
        const unsigned childGridWidth = childGrid.Width();

        // Pack our child's update
        const FactorCommMeta& commMeta = node.factorMeta;
        const DistMatrix<F>& childUpdate = childFront.work2d;
        const bool onLeft = childNode.onLeft;
        std::vector<int> sendCounts(commSize), sendDispls(commSize);
        Int sendBufferSize = 0;
        for( Unsigned proc=0; proc<commSize; ++proc )
        {
            const Int sendSize = commMeta.numChildSendInds[proc];
            sendCounts[proc] = sendSize;
            sendDispls[proc] = sendBufferSize;
            sendBufferSize += sendSize;
        }
        std::vector<F> sendBuffer( sendBufferSize );

        const std::vector<Int>& myChildRelInds = 
            ( onLeft ? node.leftRelInds : node.rightRelInds );
        const Int updateColShift = childUpdate.ColShift();
        const Int updateRowShift = childUpdate.RowShift();
        const Int updateLocHeight = childUpdate.LocalHeight();
        const Int updateLocWidth = childUpdate.LocalWidth();
        std::vector<int> packOffs = sendDispls;
        for( Int jChildLoc=0; jChildLoc<updateLocWidth; ++jChildLoc )
        {
            const Int jChild = updateRowShift + jChildLoc*childGridWidth;
            const int destGridCol = myChildRelInds[jChild] % gridWidth;
            Int localColShift;
            if( updateColShift > jChild )
                localColShift = 0;
            else if( (jChild-updateColShift) % childGridHeight == 0 )
                localColShift = (jChild-updateColShift)/childGridHeight;
            else
                localColShift = (jChild-updateColShift)/childGridHeight + 1;
            for( Int iChildLoc=localColShift; 
                     iChildLoc<updateLocHeight; ++iChildLoc )
            {
                const Int iChild = updateColShift + iChildLoc*childGridHeight;
                if( iChild >= jChild )
                {
                    const int destGridRow = myChildRelInds[iChild] % gridHeight;
                    const int destRank = destGridRow + destGridCol*gridHeight;
                    sendBuffer[packOffs[destRank]++] = 
                        childUpdate.GetLocal(iChildLoc,jChildLoc);
                }
            }
        }
        DEBUG_ONLY(
            for( unsigned proc=0; proc<commSize; ++proc )
            {
                if( packOffs[proc]-sendDispls[proc] != 
                    commMeta.numChildSendInds[proc] )
                    LogicError("Error in packing stage");
            }
        )
        SwapClear( packOffs );
        childFront.work2d.Empty();
        if( s == 1 )
            topLocFront.work.Empty();

        // Set up the recv buffer for the AllToAll
        const bool computeFactRecvInds = ( commMeta.childRecvInds.size() == 0 );
        if( computeFactRecvInds )
            ComputeFactRecvInds( node, childNode );
        std::vector<int> recvCounts(commSize), recvDispls(commSize);
        Int recvBufferSize=0;
        for( unsigned proc=0; proc<commSize; ++proc )
        {
            const Int recvSize = commMeta.childRecvInds[proc].size()/2;
            recvCounts[proc] = recvSize;
            recvDispls[proc] = recvBufferSize;
            recvBufferSize += recvSize;
        }
        std::vector<F> recvBuffer( recvBufferSize );
        DEBUG_ONLY(VerifySendsAndRecvs( sendCounts, recvCounts, comm ))

        // AllToAll to send and receive the child updates
        SparseAllToAll
        ( sendBuffer, sendCounts, sendDispls,
          recvBuffer, recvCounts, recvDispls, comm );
        SwapClear( sendBuffer );
        SwapClear( sendCounts );
        SwapClear( sendDispls );

        // Unpack the child udpates (with an Axpy)
        front.work2d.SetGrid( front.front2dL.Grid() );
        front.work2d.Align( node.size % gridHeight, node.size % gridWidth );
        Zeros( front.work2d, updateSize, updateSize );
        const Int leftLocWidth = front.front2dL.LocalWidth();
        const Int topLocHeight = Length( node.size, grid.Row(), gridHeight );
        for( unsigned proc=0; proc<commSize; ++proc )
        {
            const F* recvVals = &recvBuffer[recvDispls[proc]];
            const std::vector<Int>& recvInds = commMeta.childRecvInds[proc];
            const Int numRecvIndPairs = recvInds.size()/2;
            for( Int k=0; k<numRecvIndPairs; ++k )
            {
                const Int iFrontLoc = recvInds[2*k+0];
                const Int jFrontLoc = recvInds[2*k+1];
                const F value = recvVals[k];
                DEBUG_ONLY(
                    const Int iFront = grid.Row() + iFrontLoc*gridHeight;
                    const Int jFront = grid.Col() + jFrontLoc*gridWidth;
                    if( iFront < jFront )
                        LogicError("Tried to update upper triangle");
                )
                if( jFrontLoc < leftLocWidth )
                    front.front2dL.UpdateLocal( iFrontLoc, jFrontLoc, value );
                else
                    front.work2d.UpdateLocal
                    ( iFrontLoc-topLocHeight, jFrontLoc-leftLocWidth, value );
            }
        }
        SwapClear( recvBuffer );
        SwapClear( recvCounts );
        SwapClear( recvDispls );
        if( computeFactRecvInds )
            commMeta.EmptyChildRecvIndices();

        // Now that the frontal matrix is set up, perform the factorization
        ProcessFront
        ( front.front2dL, front.work2d, pivTol, 
          regCand.distNodes[s-1], reg.distNodes[s-1] );

        // Store the diagonal in a [VC,* ] distribution
        auto diag = front.front2dL.GetDiagonal();
        front.diag1d.SetGrid( grid );
        front.diag1d = diag;
        SetDiagonal( front.front2dL, F(1) );
    }
    L.localFronts.back().work.Empty();
    L.distFronts.back().work2d.Empty();
}

} // namespace reg_ldl
} // namespace El

#endif // ifndef EL_OPTIMIZATION_REGLDL_PROCESSDISTTREE_HPP
