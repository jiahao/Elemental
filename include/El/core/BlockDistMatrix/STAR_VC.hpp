/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_BLOCKDISTMATRIX_STAR_VC_DECL_HPP
#define EL_BLOCKDISTMATRIX_STAR_VC_DECL_HPP

namespace El {

// Partial specialization to A[* ,VC].
//
// The rows of these distributed matrices are spread throughout the 
// process grid in a column-major fashion, while the columns are not 
// distributed.
template<typename T>
class BlockDistMatrix<T,STAR,VC> : public AbstractBlockDistMatrix<T>
{
public:
    // Typedefs
    // ========
    typedef AbstractBlockDistMatrix<T> absType;
    typedef BlockDistMatrix<T,STAR,VC> type;

    // Constructors and destructors
    // ============================

    // Create a 0 x 0 distributed matrix with default (and unpinned) block size
    BlockDistMatrix( const El::Grid& g=DefaultGrid(), int root=0 );
    // Create a 0 x 0 distributed matrix with fixed block size
    BlockDistMatrix
    ( const El::Grid& g, Int blockHeight, Int blockWidth, int root=0 );
    // Create a height x width distributed matrix with default block size
    BlockDistMatrix
    ( Int height, Int width, const El::Grid& g=DefaultGrid(), int root=0 );
    // Create a height x width distributed matrix with fixed block size
    BlockDistMatrix
    ( Int height, Int width, const El::Grid& g,
      Int blockHeight, Int blockWidth, int root=0 );
    // Create a copy of distributed matrix A (redistributing if necessary)
    BlockDistMatrix( const type& A );
    BlockDistMatrix( const absType& A );
    template<Dist U,Dist V> BlockDistMatrix( const BlockDistMatrix<T,U,V>& A );
    template<Dist U,Dist V> BlockDistMatrix( const DistMatrix<T,U,V>& A );
    // Move constructor
    BlockDistMatrix( type&& A ) EL_NOEXCEPT;
    // Destructor
    ~BlockDistMatrix();

    BlockDistMatrix<T,STAR,VC>* Construct
    ( const El::Grid& g, int root ) const override;
    BlockDistMatrix<T,VC,STAR>* ConstructTranspose
    ( const El::Grid& g, int root ) const override;
    BlockDistMatrix<T,VC,STAR>* ConstructDiagonal
    ( const El::Grid& g, int root ) const override;

    // Assignment and reconfiguration
    // ==============================
    template<Dist U,Dist V> type& operator=( const DistMatrix<T,U,V>& A );
    type& operator=( const absType& A );
    type& operator=( const BlockDistMatrix<T,MC,  MR  >& A );
    type& operator=( const BlockDistMatrix<T,MC,  STAR>& A );
    type& operator=( const BlockDistMatrix<T,STAR,MR  >& A );
    type& operator=( const BlockDistMatrix<T,MD,  STAR>& A );
    type& operator=( const BlockDistMatrix<T,STAR,MD  >& A );
    type& operator=( const BlockDistMatrix<T,MR,  MC  >& A );
    type& operator=( const BlockDistMatrix<T,MR,  STAR>& A );
    type& operator=( const BlockDistMatrix<T,STAR,MC  >& A );
    type& operator=( const BlockDistMatrix<T,VC,  STAR>& A );
    type& operator=( const BlockDistMatrix<T,STAR,VC  >& A );
    type& operator=( const BlockDistMatrix<T,VR,  STAR>& A );
    type& operator=( const BlockDistMatrix<T,STAR,VR  >& A );
    type& operator=( const BlockDistMatrix<T,STAR,STAR>& A );
    type& operator=( const BlockDistMatrix<T,CIRC,CIRC>& A );
    // Move assignment
    type& operator=( type&& A );

    // Basic queries
    // =============
    El::BlockDistData DistData() const override;

    Dist ColDist()             const override;
    Dist RowDist()             const override;
    Dist PartialColDist()      const override;
    Dist PartialRowDist()      const override;
    Dist PartialUnionColDist() const override;
    Dist PartialUnionRowDist() const override;
    Dist CollectedColDist()    const override;
    Dist CollectedRowDist()    const override;

    mpi::Comm DistComm()            const override;
    mpi::Comm CrossComm()           const override;
    mpi::Comm RedundantComm()       const override;
    mpi::Comm ColComm()             const override;
    mpi::Comm RowComm()             const override;
    mpi::Comm PartialRowComm()      const override;
    mpi::Comm PartialUnionRowComm() const override;

    int ColStride()             const override;
    int RowStride()             const override;
    int PartialRowStride()      const override;
    int PartialUnionRowStride() const override;
    int DistSize()              const override;
    int CrossSize()             const override;
    int RedundantSize()         const override;

private:
    // Friend declarations
    // ===================
    template<typename S,Dist U,Dist V> friend class DistMatrix;
    template<typename S,Dist U,Dist V> friend class BlockDistMatrix;
};

} // namespace El

#endif // ifndef EL_BLOCKDISTMATRIX_STAR_VC_DECL_HPP
