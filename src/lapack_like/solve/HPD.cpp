/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {

namespace hpd_solve {

template<typename F>
void Overwrite
( UpperOrLower uplo, Orientation orientation, 
  Matrix<F>& A, Matrix<F>& B )
{
    DEBUG_ONLY(CallStackEntry cse("hpd_solve::Overwrite"))
    Cholesky( uplo, A );
    cholesky::SolveAfter( uplo, orientation, A, B );
}

template<typename F>
void Overwrite
( UpperOrLower uplo, Orientation orientation, 
  AbstractDistMatrix<F>& APre, AbstractDistMatrix<F>& BPre )
{
    DEBUG_ONLY(CallStackEntry cse("hpd_solve::Overwrite"))

    auto APtr = ReadProxy<F,MC,MR>( &APre );  auto& A = *APtr;
    auto BPtr = WriteProxy<F,MC,MR>( &BPre ); auto& B = *BPtr;

    Cholesky( uplo, A );
    cholesky::SolveAfter( uplo, orientation, A, B );
}

} // namespace hpd_solve

template<typename F>
void HPDSolve
( UpperOrLower uplo, Orientation orientation, 
  const Matrix<F>& A, Matrix<F>& B )
{
    DEBUG_ONLY(CallStackEntry cse("HPDSolve"))
    Matrix<F> ACopy( A );
    hpd_solve::Overwrite( uplo, orientation, ACopy, B );
}

template<typename F>
void HPDSolve
( UpperOrLower uplo, Orientation orientation, 
  const AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B )
{
    DEBUG_ONLY(CallStackEntry cse("HPDSolve"))
    DistMatrix<F> ACopy( A );
    hpd_solve::Overwrite( uplo, orientation, ACopy, B );
}

// TODO: Add iterative refinement parameter
template<typename F>
void HPDSolve
( const SparseMatrix<F>& A, Matrix<F>& B,
  const BisectCtrl& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("HPDSolve"))
    SymmNodeInfo info;
    Separator rootSep;
    vector<Int> map, invMap;
    NestedDissection( A.LockedGraph(), map, rootSep, info, ctrl );
    InvertMap( map, invMap );

    SymmFront<F> front( A, map, info, true );
    LDL( info, front );

    // TODO: Extend ldl::SolveWithIterativeRefinement to support multiple
    //       right-hand sides
    /*
    ldl::SolveWithIterativeRefinement
    ( A, invMap, info, front, B, minReductionFactor, maxRefineIts );
    */
    ldl::SolveAfter( invMap, info, front, B );
}

// TODO: Add iterative refinement parameter
template<typename F>
void HPDSolve
( const DistSparseMatrix<F>& A, DistMultiVec<F>& B,
  const BisectCtrl& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("HPDSolve"))
    DistSymmNodeInfo info;
    DistSeparator rootSep;
    DistMap map, invMap;
    NestedDissection( A.LockedDistGraph(), map, rootSep, info, ctrl );
    InvertMap( map, invMap );

    DistSymmFront<F> front( A, map, rootSep, info, true );
    LDL( info, front );

    // TODO: Extend ldl::SolveWithIterativeRefinement to support multiple
    //       right-hand sides
    /*
    ldl::SolveWithIterativeRefinement
    ( A, invMap, info, front, B, minReductionFactor, maxRefineIts );
    */
    ldl::SolveAfter( invMap, info, front, B );
}

#define PROTO(F) \
  template void hpd_solve::Overwrite \
  ( UpperOrLower uplo, Orientation orientation, \
    Matrix<F>& A, Matrix<F>& B ); \
  template void hpd_solve::Overwrite \
  ( UpperOrLower uplo, Orientation orientation, \
    AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B ); \
  template void HPDSolve \
  ( UpperOrLower uplo, Orientation orientation, \
    const Matrix<F>& A, Matrix<F>& B ); \
  template void HPDSolve \
  ( UpperOrLower uplo, Orientation orientation, \
    const AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B ); \
  template void HPDSolve \
  ( const SparseMatrix<F>& A, Matrix<F>& B, const BisectCtrl& ctrl ); \
  template void HPDSolve \
  ( const DistSparseMatrix<F>& A, DistMultiVec<F>& B, const BisectCtrl& ctrl );

#define EL_NO_INT_PROTO
#include "El/macros/Instantiate.h"

} // namespace El
