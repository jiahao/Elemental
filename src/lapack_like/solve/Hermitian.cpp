/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {

namespace herm_solve {

template<typename F>
void Overwrite
( UpperOrLower uplo, Orientation orientation, 
  Matrix<F>& A, Matrix<F>& B, 
  const LDLPivotCtrl<Base<F>>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("herm_solve::Overwrite"))
    symm_solve::Overwrite( uplo, orientation, A, B, true, ctrl );
}

template<typename F>
void Overwrite
( UpperOrLower uplo, Orientation orientation, 
  AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B,
  const LDLPivotCtrl<Base<F>>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("herm_solve::Overwrite"))
    symm_solve::Overwrite( uplo, orientation, A, B, true, ctrl );
}

} // namespace herm_solve

template<typename F>
void HermitianSolve
( UpperOrLower uplo, Orientation orientation, 
  const Matrix<F>& A, Matrix<F>& B, 
  const LDLPivotCtrl<Base<F>>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("HermitianSolve"))
    SymmetricSolve( uplo, orientation, A, B, true, ctrl );
}

template<typename F>
void HermitianSolve
( UpperOrLower uplo, Orientation orientation, 
  const AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B,
  const LDLPivotCtrl<Base<F>>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("HermitianSolve"))
    SymmetricSolve( uplo, orientation, A, B, true, ctrl );
}

// TODO: Add iterative refinement parameter
template<typename F>
void HermitianSolve
( const SparseMatrix<F>& A, Matrix<F>& B, 
  bool tryLDL, const BisectCtrl& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("HermitianSolve"))
    SymmetricSolve( A, B, true, tryLDL, ctrl );
}

// TODO: Add iterative refinement parameter
template<typename F>
void HermitianSolve
( const DistSparseMatrix<F>& A, DistMultiVec<F>& B, 
  bool tryLDL, const BisectCtrl& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("HermitianSolve"))
    SymmetricSolve( A, B, true, tryLDL, ctrl );
}

#define PROTO(F) \
  template void herm_solve::Overwrite \
  ( UpperOrLower uplo, Orientation orientation, \
    Matrix<F>& A, Matrix<F>& B, \
    const LDLPivotCtrl<Base<F>>& ctrl ); \
  template void herm_solve::Overwrite \
  ( UpperOrLower uplo, Orientation orientation, \
    AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B, \
    const LDLPivotCtrl<Base<F>>& ctrl ); \
  template void HermitianSolve \
  ( UpperOrLower uplo, Orientation orientation, \
    const Matrix<F>& A, Matrix<F>& B, \
    const LDLPivotCtrl<Base<F>>& ctrl ); \
  template void HermitianSolve \
  ( UpperOrLower uplo, Orientation orientation, \
    const AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B, \
    const LDLPivotCtrl<Base<F>>& ctrl ); \
  template void HermitianSolve \
  ( const SparseMatrix<F>& A, Matrix<F>& B, \
    bool tryLDL, const BisectCtrl& ctrl ); \
  template void HermitianSolve \
  ( const DistSparseMatrix<F>& A, DistMultiVec<F>& B, \
    bool tryLDL, const BisectCtrl& ctrl );

#define EL_NO_INT_PROTO
#include "El/macros/Instantiate.h"

} // namespace El
