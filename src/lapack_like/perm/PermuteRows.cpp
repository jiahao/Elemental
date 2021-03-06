/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {

template<typename T> 
void PermuteRows
( Matrix<T>& A, const Matrix<Int>& perm, const Matrix<Int>& invPerm )
{
    const Int b = perm.Height();
    DEBUG_ONLY(
        CallStackEntry cse("PermuteRows");
        if( A.Height() < b || b != invPerm.Height() )
            LogicError
            ("perm and invPerm must be vectors of equal length that are not "
             "taller than A.");
    )
    const Int m = A.Height();
    const Int n = A.Width();
    if( m == 0 || n == 0 )
        return;

    // TODO: Optimize this routine

    // Make a copy of the first b rows
    auto ARowPanView = A( IR(0,b), IR(0,n) );
    auto ARowPanCopy( ARowPanView );

    // Make a copy of the preimage rows
    Matrix<T> APreimageCopy( b, n );
    for( Int i=0; i<b; ++i ) 
    {
        const Int iPre = perm.Get(i,0);
        if( iPre >= b )
            for( Int j=0; j<n; ++j )
                APreimageCopy.Set(i,j,A.Get(iPre,j));
    }

    // Apply the permutations
    for( Int i=0; i<b; ++i )
    {
        const Int iPre = perm.Get(i,0);
        const Int iPost = invPerm.Get(i,0);
        // Move row[i] into row[image[i]]
        for( Int j=0; j<n; ++j )
            A.Set(iPost,j,ARowPanCopy.Get(i,j));
        if( iPre >= b )
        {
            // Move row[preimage[i]] into row[i]
            for( Int j=0; j<n; ++j )
                A.Set(i,j,APreimageCopy.Get(i,j));
        }
    }
}

template<typename T> 
void PermuteRows( Matrix<T>& A, const Matrix<Int>& perm )
{
    DEBUG_ONLY(CallStackEntry cse("PermuteRows"))
    Matrix<Int> invPerm;
    InvertPermutation( perm, invPerm );
    PermuteRows( A, perm, invPerm );
}

template<typename T> 
void InversePermuteRows( Matrix<T>& A, const Matrix<Int>& invPerm )
{
    DEBUG_ONLY(CallStackEntry cse("InversePermuteRows"))
    Matrix<Int> perm;
    InvertPermutation( invPerm, perm );
    PermuteRows( A, perm, invPerm );
}

template<typename T>
void PermuteRows( AbstractDistMatrix<T>& A, const PermutationMeta& oldMeta )
{
    DEBUG_ONLY(
        CallStackEntry cse("PermuteRows");
        if( A.ColComm() != oldMeta.comm )
            LogicError("Invalid communicator in metadata");
        if( A.ColAlign() != oldMeta.align )
            LogicError("Invalid alignment in metadata");
    )
    if( A.Height() == 0 || A.Width() == 0 || !A.Participating() )
        return;

    const Int localWidth = A.LocalWidth();
    const Int ldim = A.LDim();
    PermutationMeta meta = oldMeta;
    meta.ScaleUp( localWidth );

    // Fill vectors with the send data
    auto offsets = meta.sendDispls;
    const int totalSend = meta.TotalSend();
    vector<T> sendData( mpi::Pad(totalSend) );
    const int numSends = meta.sendIdx.size();
    for( int send=0; send<numSends; ++send )
    {
        const int iLoc = meta.sendIdx[send];
        const int rank = meta.sendRanks[send];
        
        StridedMemCopy
        ( &sendData[offsets[rank]], 1,
          A.LockedBuffer(iLoc,0),   ldim, localWidth );
        offsets[rank] += localWidth;
    }

    // Communicate all pivot rows
    const int totalRecv = meta.TotalRecv();
    vector<T> recvData( mpi::Pad(totalRecv) );
    mpi::AllToAll
    ( sendData.data(), meta.sendCounts.data(), meta.sendDispls.data(),
      recvData.data(), meta.recvCounts.data(), meta.recvDispls.data(),
      meta.comm );

    // Unpack the recv data
    offsets = meta.recvDispls;
    const int numRecvs = meta.recvIdx.size();
    for( int recv=0; recv<numRecvs; ++recv )
    {
        const int iLoc = meta.recvIdx[recv];
        const int rank = meta.recvRanks[recv];
        StridedMemCopy
        ( A.Buffer(iLoc,0),         ldim,
          &recvData[offsets[rank]], 1,    localWidth );
        offsets[rank] += localWidth;
    }
}

template<typename T,Dist U,Dist V>
void PermuteRows
( DistMatrix<T,U,V>& A,
  const AbstractDistMatrix<Int>& permPre,
  const AbstractDistMatrix<Int>& invPermPre )
{
    DEBUG_ONLY(CallStackEntry cse("PermuteRows"))

    ProxyCtrl ctrl;
    ctrl.rootConstrain = true;
    ctrl.colConstrain = true;
    ctrl.root = A.Root();
    ctrl.colAlign = A.ColAlign();

    auto permPtr    = ReadProxy<Int,U,Collect<V>()>( &permPre,    ctrl );
    auto invPermPtr = ReadProxy<Int,U,Collect<V>()>( &invPermPre, ctrl );
   
    if( A.Participating() )
    {
        PermutationMeta meta( *permPtr, *invPermPtr );
        PermuteRows( A, meta );
    }
}

template<typename T>
void PermuteRows
( AbstractDistMatrix<T>& APre,
  const AbstractDistMatrix<Int>& perm,
  const AbstractDistMatrix<Int>& invPerm )
{
    DEBUG_ONLY(CallStackEntry cse("PermuteRows"))
    const Dist U = APre.ColDist();
    const Dist V = APre.RowDist();
    #define GUARD(CDIST,RDIST) U == CDIST && V == RDIST
    #define PAYLOAD(CDIST,RDIST) \
        auto& A = dynamic_cast<DistMatrix<T,CDIST,RDIST>&>(APre); \
        PermuteRows( A, perm, invPerm );
    #include "El/macros/GuardAndPayload.h"
}

template<typename T>
void PermuteRows
( AbstractDistMatrix<T>& A, const AbstractDistMatrix<Int>& perm )
{
    DEBUG_ONLY(CallStackEntry cse("PermuteRows"))
    const Grid& g = A.Grid();
    DistMatrix<Int,VC,STAR> invPerm(g);
    InvertPermutation( perm, invPerm );
    PermuteRows( A, perm, invPerm );
}

template<typename T>
void InversePermuteRows
( AbstractDistMatrix<T>& A, const AbstractDistMatrix<Int>& invPerm )
{
    DEBUG_ONLY(CallStackEntry cse("InversePermuteRows"))
    const Grid& g = A.Grid();
    DistMatrix<Int,VC,STAR> perm(g);
    InvertPermutation( invPerm, perm );
    PermuteRows( A, perm, invPerm );
}

#define PROTO(T) \
  template void PermuteRows( Matrix<T>& A, const Matrix<Int>& perm ); \
  template void PermuteRows \
  ( AbstractDistMatrix<T>& A, const AbstractDistMatrix<Int>& perm ); \
  template void InversePermuteRows( Matrix<T>& A, const Matrix<Int>& perm ); \
  template void InversePermuteRows \
  ( AbstractDistMatrix<T>& A, const AbstractDistMatrix<Int>& perm ); \
  template void PermuteRows \
  ( Matrix<T>& A, const Matrix<Int>& perm, const Matrix<Int>& invPerm ); \
  template void PermuteRows \
  ( AbstractDistMatrix<T>& A, \
    const AbstractDistMatrix<Int>& perm, \
    const AbstractDistMatrix<Int>& invPerm ); \
  template void PermuteRows \
  ( AbstractDistMatrix<T>& A, const PermutationMeta& oldMeta );

#include "El/macros/Instantiate.h"

} // namespace El
