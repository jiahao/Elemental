/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_SOLVE_HPP
#define EL_SOLVE_HPP

#include "El/lapack_like/factor.hpp"

namespace El {

// B := inv(A) B for a general square A
// ====================================
template<typename F>
void GaussianElimination( Matrix<F>& A, Matrix<F>& B );
template<typename F>
void GaussianElimination( AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B );

// min_{X,Y} || Y ||_F subject to D = A X + B Y
// ============================================
template<typename F>
void GLM
( Matrix<F>& A, Matrix<F>& B, Matrix<F>& D, Matrix<F>& Y );
template<typename F>
void GLM
( AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B, 
  AbstractDistMatrix<F>& D, AbstractDistMatrix<F>& Y );

// B := inv(A) B for a Hermitian A
// ===============================
template<typename F>
void HermitianSolve
( UpperOrLower uplo, Orientation orientation, 
  Matrix<F>& A, Matrix<F>& B, 
  const LDLPivotCtrl<Base<F>>& ctrl=LDLPivotCtrl<Base<F>>() );
template<typename F>
void HermitianSolve
( UpperOrLower uplo, Orientation orientation,
  AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B, 
  const LDLPivotCtrl<Base<F>>& ctrl=LDLPivotCtrl<Base<F>>() );

template<typename F>
void HermitianSolve
( const SparseMatrix<F>& A, Matrix<F>& X,
  const BisectCtrl& ctrl=BisectCtrl() );
template<typename F>
void HermitianSolve
( const DistSparseMatrix<F>& A, DistMultiVec<F>& X,
  const BisectCtrl& ctrl=BisectCtrl() );

// B := inv(A) for Hermitian Positive-Definite (HPD) A
// ===================================================
template<typename F>
void HPDSolve
( UpperOrLower uplo, Orientation orientation, 
  Matrix<F>& A, Matrix<F>& B );
template<typename F>
void HPDSolve
( UpperOrLower uplo, Orientation orientation,
  AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B );

// min_X || A X - B ||_F
// =====================
template<typename F>
void LeastSquares
( Orientation orientation, Matrix<F>& A, const Matrix<F>& B, 
  Matrix<F>& X );
template<typename F>
void LeastSquares
( Orientation orientation, AbstractDistMatrix<F>& A, 
  const AbstractDistMatrix<F>& B, AbstractDistMatrix<F>& X );

template<typename F>
void LeastSquares
( Orientation orientation,
  const SparseMatrix<F>& A, const Matrix<F>& Y, Matrix<F>& X,
  const BisectCtrl& ctrl=BisectCtrl() );
template<typename F>
void LeastSquares
( Orientation orientation,
  const DistSparseMatrix<F>& A, const DistMultiVec<F>& Y, DistMultiVec<F>& X,
  const BisectCtrl& ctrl=BisectCtrl() );

// min_X || A X - C ||_F subject to B X = D
// ========================================
template<typename F>
void LSE
( Matrix<F>& A, Matrix<F>& B, Matrix<F>& C, Matrix<F>& D, 
  Matrix<F>& X, bool computeResidual=false );
template<typename F>
void LSE
( AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B, 
  AbstractDistMatrix<F>& C, AbstractDistMatrix<F>& D, 
  AbstractDistMatrix<F>& X, bool computeResidual=false );

// Solve for B in op(H) B - B op(D) = X, where H is Hessenberg
// ===========================================================
template<typename F>
void MultiShiftHessSolve
( UpperOrLower uplo, Orientation orientation,
  F alpha, const Matrix<F>& H, const Matrix<F>& shifts,
  Matrix<F>& X );
template<typename F>
void MultiShiftHessSolve
( UpperOrLower uplo, Orientation orientation,
  F alpha, const AbstractDistMatrix<F>& H, const AbstractDistMatrix<F>& shifts,
  AbstractDistMatrix<F>& X );

// Ridge regression
// ================
// NOTE: This is simply Tikhonov regularization for cases where 
//       Gamma = alpha I

namespace RidgeAlgNS {
enum RidgeAlg {
    RIDGE_CHOLESKY,
    RIDGE_QR,
    RIDGE_SVD
};
}
using namespace RidgeAlgNS;

template<typename F>
void Ridge
( const Matrix<F>& A, const Matrix<F>& B, 
  Base<F> alpha, Matrix<F>& X, RidgeAlg alg=RIDGE_CHOLESKY );
template<typename F>
void Ridge
( const AbstractDistMatrix<F>& A, const AbstractDistMatrix<F>& B, 
  Base<F> alpha, AbstractDistMatrix<F>& X, RidgeAlg alg=RIDGE_CHOLESKY );

template<typename F>
void Ridge
( const SparseMatrix<F>& A, const Matrix<F>& B, Base<F> alpha,
        Matrix<F>& X, const BisectCtrl& ctrl=BisectCtrl() );
template<typename F>
void Ridge
( const DistSparseMatrix<F>& A, const DistMultiVec<F>& B, Base<F> alpha,
        DistMultiVec<F>& X, const BisectCtrl& ctrl=BisectCtrl() );

// B := inv(A) B for a symmetric A
// ===============================
template<typename F>
void SymmetricSolve
( UpperOrLower uplo, Orientation orientation, 
  Matrix<F>& A, Matrix<F>& B, bool conjugate=false, 
  const LDLPivotCtrl<Base<F>>& ctrl=LDLPivotCtrl<Base<F>>() );
template<typename F>
void SymmetricSolve
( UpperOrLower uplo, Orientation orientation,
  AbstractDistMatrix<F>& A, AbstractDistMatrix<F>& B, bool conjugate=false, 
  const LDLPivotCtrl<Base<F>>& ctrl=LDLPivotCtrl<Base<F>>() );

template<typename F>
void SymmetricSolve
( const SparseMatrix<F>& A, Matrix<F>& X, bool conjugate=false,
  const BisectCtrl& ctrl=BisectCtrl() );
template<typename F>
void SymmetricSolve
( const DistSparseMatrix<F>& A, DistMultiVec<F>& X, bool conjugate=false,
  const BisectCtrl& ctrl=BisectCtrl() );

// Tikhonov regularization
// =======================
// Solve arg min_X || op(A) X - B ||_2^2 + || Gamma X ||_2^2
// where op(A) is A, A^T, or A^H.

namespace TikhonovAlgNS {
enum TikhonovAlg {
    TIKHONOV_CHOLESKY,
    TIKHONOV_QR
};
}
using namespace TikhonovAlgNS;

template<typename F>
void Tikhonov
( const Matrix<F>& A, const Matrix<F>& B, 
  const Matrix<F>& Gamma, Matrix<F>& X, 
  TikhonovAlg alg=TIKHONOV_CHOLESKY );
template<typename F>
void Tikhonov
( const AbstractDistMatrix<F>& A, const AbstractDistMatrix<F>& B, 
  const AbstractDistMatrix<F>& Gamma, AbstractDistMatrix<F>& X, 
  TikhonovAlg alg=TIKHONOV_CHOLESKY );

template<typename F>
void Tikhonov
( const SparseMatrix<F>& A, const Matrix<F>& B,
  const SparseMatrix<F>& Gamma, Matrix<F>& X,
  const BisectCtrl& ctrl=BisectCtrl() );
template<typename F>
void Tikhonov
( const DistSparseMatrix<F>& A, const DistMultiVec<F>& B,
  const DistSparseMatrix<F>& Gamma, DistMultiVec<F>& X,
  const BisectCtrl& ctrl=BisectCtrl() );

// TODO: Generalized Tikhonov regularization

} // namespace El

#endif // ifndef EL_SOLVE_HPP
