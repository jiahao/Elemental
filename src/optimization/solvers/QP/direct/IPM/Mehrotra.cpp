/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"
#include "./util.hpp"

namespace El {
namespace qp {
namespace direct {

// The following solves a pair of quadratic programs in "direct" conic form:
//
//   min (1/2) x^T Q x + c^T x
//   s.t. A x = b, x >= 0,
//
//   max (1/2) (A^T y - z + c)^T pinv(Q) (A^T y - z + c) - b^T y
//   s.t. A^T y - z + c in range(Q), z >= 0,
//
// as opposed to the more general "affine" conic form:
//
//   min (1/2) x^T Q x + c^T x
//   s.t. A x = b, G x + s = h, s >= 0,
//
//   max (1/2) (A^T y + G^T z + c)^T pinv(Q) (A^T y + G^T z + c) - b^T y - h^T z
//   s.t. A^T y + G^T z + c in range(Q), z >= 0
//
// using a Mehrotra Predictor-Corrector scheme.
//

template<typename Real>
void Mehrotra
( const Matrix<Real>& QPre, const Matrix<Real>& APre,
  const Matrix<Real>& bPre, const Matrix<Real>& cPre,
        Matrix<Real>& x,          Matrix<Real>& y, 
        Matrix<Real>& z,
  const MehrotraCtrl<Real>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("qp::direct::Mehrotra"))    

    // Equilibrate the QP by diagonally scaling A
    auto Q = QPre;
    auto A = APre;
    auto b = bPre;
    auto c = cPre;
    const Int m = A.Height();
    const Int n = A.Width();
    Matrix<Real> dRow, dCol;
    if( ctrl.outerEquil )
    {
        GeomEquil( A, dRow, dCol, ctrl.print );

        DiagonalSolve( LEFT, NORMAL, dRow, b );
        DiagonalSolve( LEFT, NORMAL, dCol, c );
        // TODO: Replace with SymmetricDiagonalSolve
        {
            DiagonalSolve( LEFT, NORMAL, dCol, Q );
            DiagonalSolve( RIGHT, NORMAL, dCol, Q );
        }
        if( ctrl.primalInit )
            DiagonalScale( LEFT, NORMAL, dCol, x );
        if( ctrl.dualInit )
        {
            DiagonalScale( LEFT, NORMAL, dRow, y );
            DiagonalSolve( LEFT, NORMAL, dCol, z );
        }
    }
    else
    {
        Ones( dRow, m, 1 );
        Ones( dCol, n, 1 );
    }

    const Real bNrm2 = Nrm2( b );
    const Real cNrm2 = Nrm2( c );

    // TODO: Expose this as a parameter of MehrotraCtrl
    const bool standardShift = true;
    Initialize
    ( Q, A, b, c, x, y, z, ctrl.primalInit, ctrl.dualInit, standardShift ); 

    Matrix<Real> J, d, 
                 rb,    rc,    rmu,
                 dxAff, dyAff, dzAff,
                 dx,    dy,    dz;
    Matrix<Real> dSub;
    Matrix<Int> p;
#ifndef EL_RELEASE
    Matrix<Real> dxError, dyError, dzError, prod;
#endif
    for( Int numIts=0; ; ++numIts )
    {
        // Ensure that x and z are in the cone
        // ===================================
        const Int xNumNonPos = NumNonPositive( x );
        const Int zNumNonPos = NumNonPositive( z );
        if( xNumNonPos > 0 || zNumNonPos > 0 )
            LogicError
            (xNumNonPos," entries of x were nonpositive and ",
             zNumNonPos," entries of z were nonpositive");

        // Check for convergence
        // =====================
        // |primal - dual| / (1 + |primal|) <= tol ?
        // -----------------------------------------
        Zeros( d, n, 1 );
        Hemv( LOWER, Real(1), Q, x, Real(0), d );
        const Real xTQx = Dot(x,d);
        const Real primObj =  xTQx/2 + Dot(c,x);
        const Real dualObj = -xTQx/2 - Dot(b,y); 
        const Real objConv = Abs(primObj-dualObj) / (Real(1)+Abs(primObj));
        // || r_b ||_2 / (1 + || b ||_2) <= tol ?
        // --------------------------------------
        rb = b;
        Scale( Real(-1), rb );
        Gemv( NORMAL, Real(1), A, x, Real(1), rb );
        const Real rbNrm2 = Nrm2( rb );
        const Real rbConv = rbNrm2 / (Real(1)+bNrm2);
        // || r_c ||_2 / (1 + || c ||_2) <= tol ?
        // --------------------------------------
        rc = c;
        Hemv( LOWER,     Real(1), Q, x, Real(1), rc );
        Gemv( TRANSPOSE, Real(1), A, y, Real(1), rc );
        Axpy( Real(-1), z, rc );
        const Real rcNrm2 = Nrm2( rc );
        const Real rcConv = rcNrm2 / (Real(1)+cNrm2);
        // Now check the pieces
        // --------------------
        if( ctrl.print )
            cout << " iter " << numIts << ":\n"
                 << "  |primal - dual| / (1 + |primal|) = "
                 << objConv << "\n"
                 << "  || r_b ||_2 / (1 + || b ||_2)   = "
                 << rbConv << "\n"
                 << "  || r_c ||_2 / (1 + || c ||_2)   = "
                 << rcConv << endl;
        if( objConv <= ctrl.tol && rbConv <= ctrl.tol && rcConv <= ctrl.tol )
            break;

        // Raise an exception after an unacceptable number of iterations
        // =============================================================
        if( numIts == ctrl.maxIts )
            RuntimeError
            ("Maximum number of iterations (",ctrl.maxIts,") exceeded");

        // r_mu := x o z
        // =============
        rmu = z;
        DiagonalScale( LEFT, NORMAL, x, rmu );

        // Compute the affine search direction
        // ===================================
        if( ctrl.system == FULL_KKT )
        {
            // Construct the full KKT system
            // -----------------------------
            KKT( Q, A, x, z, J );
            KKTRHS( rc, rb, rmu, z, d );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            LDL( J, dSub, p, false );
            ldl::SolveAfter( J, dSub, p, d, false );
            ExpandSolution( m, n, d, dxAff, dyAff, dzAff );
        }
        else if( ctrl.system == AUGMENTED_KKT )
        {
            // Construct the "augmented" KKT system
            // ------------------------------------
            AugmentedKKT( Q, A, x, z, J );
            AugmentedKKTRHS( x, rc, rb, rmu, d );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            LDL( J, dSub, p, false );
            ldl::SolveAfter( J, dSub, p, d, false );
            ExpandAugmentedSolution( x, z, rmu, d, dxAff, dyAff, dzAff );
        }
        else
            LogicError("Invalid KKT system choice");
#ifndef EL_RELEASE
        // Sanity checks
        // =============
        dxError = rb;
        Gemv( NORMAL, Real(1), A, dxAff, Real(1), dxError );
        Real dxErrorNrm2 = Nrm2( dxError );

        dyError = rc;
        Hemv( LOWER,     Real(1), Q, dxAff, Real(1), dyError );
        Gemv( TRANSPOSE, Real(1), A, dyAff, Real(1), dyError );
        Axpy( Real(-1), dzAff, dyError );
        Real dyErrorNrm2 = Nrm2( dyError );

        Real rmuNrm2 = Nrm2( rmu );
        dzError = rmu;
        prod = dzAff;
        DiagonalScale( LEFT, NORMAL, x, prod );
        Axpy( Real(1), prod, dzError );
        prod = dxAff;
        DiagonalScale( LEFT, NORMAL, z, prod );
        Axpy( Real(1), prod, dzError );
        Real dzErrorNrm2 = Nrm2( dzError );

        if( ctrl.print )
            cout << "  || dxAffError ||_2 / (1 + || r_b ||_2) = " 
                 << dxErrorNrm2/(1+rbNrm2) << "\n"
                 << "  || dyAffError ||_2 / (1 + || r_c ||_2) = " 
                 << dyErrorNrm2/(1+rcNrm2) << "\n"
                 << "  || dzAffError ||_2 / (1 + || r_mu ||_2) = " 
                 << dzErrorNrm2/(1+rmuNrm2) << endl;
#endif

        // Compute the max affine [0,1]-step which keeps x and z in the cone
        // =================================================================
        const Real alphaAffPri = MaxStepInPositiveCone( x, dxAff, Real(1) );
        const Real alphaAffDual = MaxStepInPositiveCone( z, dzAff, Real(1) );
        if( ctrl.print )
            cout << "  alphaAffPri = " << alphaAffPri 
                 << ", alphaAffDual = " << alphaAffDual << endl;

        // Compute what the new duality measure would become
        // =================================================
        const Real mu = Dot(x,z) / n;
        // NOTE: dz and dx are used as temporaries
        dx = x;
        dz = z;
        Axpy( alphaAffPri,  dxAff, dx );
        Axpy( alphaAffDual, dzAff, dz );
        const Real muAff = Dot(dx,dz) / n;

        // Compute a centrality parameter using Mehrotra's formula
        // =======================================================
        // TODO: Allow the user to override this function
        const Real sigma = Pow(muAff/mu,Real(3)); 
        if( ctrl.print )
            cout << "  muAff = " << muAff 
                 << ", mu = " << mu 
                 << ", sigma = " << sigma << endl;

        // Solve for the centering-corrector 
        // =================================
        Zeros( rc, n, 1 );
        Zeros( rb, m, 1 );
        // r_mu := dxAff o dzAff - sigma*mu
        // --------------------------------
        rmu = dzAff;
        DiagonalScale( LEFT, NORMAL, dxAff, rmu );
        Shift( rmu, -sigma*mu );
        if( ctrl.system == FULL_KKT )
        {
            // Construct the new full KKT RHS
            // ------------------------------
            KKTRHS( rc, rb, rmu, z, d );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            ldl::SolveAfter( J, dSub, p, d, false );
            ExpandSolution( m, n, d, dx, dy, dz );
        }
        else if( ctrl.system == AUGMENTED_KKT )
        {
            // Construct the new "augmented" KKT RHS
            // -------------------------------------
            AugmentedKKTRHS( x, rc, rb, rmu, d );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            ldl::SolveAfter( J, dSub, p, d, false );
            ExpandAugmentedSolution( x, z, rmu, d, dx, dy, dz );
        }
        else
            LogicError("Invalid KKT system choice");
        // TODO: Residual checks for center-corrector

        // Add in the affine search direction
        // ==================================
        Axpy( Real(1), dxAff, dx );
        Axpy( Real(1), dyAff, dy );
        Axpy( Real(1), dzAff, dz );

        // Compute max [0,1/maxStepRatio] step which keeps x and z in the cone
        // ===================================================================
        Real alphaPri = MaxStepInPositiveCone( x, dx, 1/ctrl.maxStepRatio );
        Real alphaDual = MaxStepInPositiveCone( z, dz, 1/ctrl.maxStepRatio );
        alphaPri = Min(ctrl.maxStepRatio*alphaPri,Real(1));
        alphaDual = Min(ctrl.maxStepRatio*alphaDual,Real(1));
        if( ctrl.print )
            cout << "  alphaPri = " << alphaPri 
                 << ", alphaDual = " << alphaDual << endl;

        // Update the current estimates
        // ============================
        Axpy( alphaPri,  dx, x );
        Axpy( alphaDual, dy, y );
        Axpy( alphaDual, dz, z ); 
    }

    if( ctrl.outerEquil )
    {
        // Unequilibrate the QP
        DiagonalSolve( LEFT, NORMAL, dCol, x );
        DiagonalSolve( LEFT, NORMAL, dRow, y );
        DiagonalScale( LEFT, NORMAL, dCol, z );
    }
}

template<typename Real>
void Mehrotra
( const AbstractDistMatrix<Real>& QPre, const AbstractDistMatrix<Real>& APre, 
  const AbstractDistMatrix<Real>& bPre, const AbstractDistMatrix<Real>& cPre,
        AbstractDistMatrix<Real>& xPre,       AbstractDistMatrix<Real>& yPre,
        AbstractDistMatrix<Real>& zPre,
  const MehrotraCtrl<Real>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("qp::direct::Mehrotra"))    
    const Grid& grid = APre.Grid();
    const int commRank = grid.Rank();

    // Ensure that the inputs have the appropriate read/write properties
    DistMatrix<Real> Q(grid), A(grid), b(grid), c(grid);
    Q.Align(0,0);
    A.Align(0,0);
    b.Align(0,0);
    c.Align(0,0);
    Q = QPre;
    A = APre;
    b = bPre;
    c = cPre;
    ProxyCtrl control;
    control.colConstrain = true;
    control.rowConstrain = true;
    control.colAlign = 0;
    control.rowAlign = 0;
    // NOTE: x does not need to be a read proxy when !ctrl.primalInit
    auto xPtr = ReadWriteProxy<Real,MC,MR>(&xPre,control); auto& x = *xPtr;
    // NOTE: {y,z} do not need to be read proxies when !ctrl.dualInit
    auto yPtr = ReadWriteProxy<Real,MC,MR>(&yPre,control); auto& y = *yPtr;
    auto zPtr = ReadWriteProxy<Real,MC,MR>(&zPre,control); auto& z = *zPtr;

    // Equilibrate the QP by diagonally scaling A
    const Int m = A.Height();
    const Int n = A.Width();
    DistMatrix<Real,MC,STAR> dRow(grid);
    DistMatrix<Real,MR,STAR> dCol(grid);
    if( ctrl.outerEquil )
    {
        GeomEquil( A, dRow, dCol, ctrl.print );

        DiagonalSolve( LEFT, NORMAL, dRow, b );
        DiagonalSolve( LEFT, NORMAL, dCol, c );
        // TODO: Replace with SymmetricDiagonalSolve
        {
            DiagonalSolve( LEFT, NORMAL, dCol, Q );
            DiagonalSolve( RIGHT, NORMAL, dCol, Q );
        }
        if( ctrl.primalInit )
            DiagonalScale( LEFT, NORMAL, dCol, x );
        if( ctrl.dualInit )
        {
            DiagonalScale( LEFT, NORMAL, dRow, y );
            DiagonalSolve( LEFT, NORMAL, dCol, z );
        }
    }
    else
    {
        Ones( dRow, m, 1 );
        Ones( dCol, n, 1 );
    }

    const Real bNrm2 = Nrm2( b );
    const Real cNrm2 = Nrm2( c );

    // TODO: Expose this as a parameter of MehrotraCtrl
    const bool standardShift = true;
    Initialize
    ( Q, A, b, c, x, y, z, ctrl.primalInit, ctrl.dualInit, standardShift ); 

    DistMatrix<Real> 
        J(grid), d(grid), 
        rc(grid),    rb(grid),    rmu(grid), 
        dxAff(grid), dyAff(grid), dzAff(grid),
        dx(grid),    dy(grid),    dz(grid);
    dx.AlignWith( x );
    dz.AlignWith( x );
    dxAff.AlignWith( x );
    dzAff.AlignWith( x );
    rmu.AlignWith( x );
    DistMatrix<Real> dSub(grid);
    DistMatrix<Int> p(grid);
#ifndef EL_RELEASE
    DistMatrix<Real> dxError(grid), dyError(grid), dzError(grid), prod(grid);
    dzError.AlignWith( dz );
#endif
    for( Int numIts=0; ; ++numIts )
    {
        // Ensure that x and z are in the cone
        // ===================================
        const Int xNumNonPos = NumNonPositive( x );
        const Int zNumNonPos = NumNonPositive( z );
        if( xNumNonPos > 0 || zNumNonPos > 0 )
            LogicError
            (xNumNonPos," entries of x were nonpositive and ",
             zNumNonPos," entries of z were nonpositive");

        // Check for convergence
        // =====================
        // |primal - dual| / (1 + |primal|) <= tol ?
        // -----------------------------------------
        Zeros( d, n, 1 );
        Hemv( LOWER, Real(1), Q, x, Real(0), d );
        const Real xTQx = Dot(x,d);
        const Real primObj =  xTQx/2 + Dot(c,x);
        const Real dualObj = -xTQx/2 - Dot(b,y); 
        const Real objConv = Abs(primObj-dualObj) / (Real(1)+Abs(primObj));
        // || r_b ||_2 / (1 + || b ||_2) <= tol ?
        // --------------------------------------
        rb = b;
        Scale( Real(-1), rb );
        Gemv( NORMAL, Real(1), A, x, Real(1), rb );
        const Real rbNrm2 = Nrm2( rb );
        const Real rbConv = rbNrm2 / (Real(1)+bNrm2);
        // || r_c ||_2 / (1 + || c ||_2) <= tol ?
        // --------------------------------------
        rc = c;
        Hemv( LOWER,     Real(1), Q, x, Real(1), rc );
        Gemv( TRANSPOSE, Real(1), A, y, Real(1), rc );
        Axpy( Real(-1), z, rc );
        const Real rcNrm2 = Nrm2( rc );
        const Real rcConv = rcNrm2 / (Real(1)+cNrm2);
        // Now check the pieces
        // --------------------
        if( ctrl.print && commRank == 0 )
            cout << " iter " << numIts << ":\n"
                 << "  |primal - dual| / (1 + |primal|) = "
                 << objConv << "\n"
                 << "  || r_b ||_2 / (1 + || b ||_2)   = "
                 << rbConv << "\n"
                 << "  || r_c ||_2 / (1 + || c ||_2)   = "
                 << rcConv << endl;
        if( objConv <= ctrl.tol && rbConv <= ctrl.tol && rcConv <= ctrl.tol )
            break;

        // Raise an exception after an unacceptable number of iterations
        // =============================================================
        if( numIts == ctrl.maxIts )
            RuntimeError
            ("Maximum number of iterations (",ctrl.maxIts,") exceeded");

        // r_mu := x o z
        // =============
        rmu = z;
        DiagonalScale( LEFT, NORMAL, x, rmu );

        // Compute the affine search direction
        // ===================================
        if( ctrl.system == FULL_KKT )
        {
            // Construct the full KKT system
            // -----------------------------
            KKT( Q, A, x, z, J );
            KKTRHS( rc, rb, rmu, z, d );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            LDL( J, dSub, p, false );
            ldl::SolveAfter( J, dSub, p, d, false );
            ExpandSolution( m, n, d, dxAff, dyAff, dzAff );
        }
        else if( ctrl.system == AUGMENTED_KKT )
        {
            // Construct the "augmented" KKT system
            // ------------------------------------
            AugmentedKKT( Q, A, x, z, J );
            AugmentedKKTRHS( x, rc, rb, rmu, d );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            LDL( J, dSub, p, false );
            ldl::SolveAfter( J, dSub, p, d, false );
            ExpandAugmentedSolution( x, z, rmu, d, dxAff, dyAff, dzAff );
        }
        else
            LogicError("Invalid KKT system choice");
#ifndef EL_RELEASE
        // Sanity checks
        // =============
        dxError = rb;
        Gemv( NORMAL, Real(1), A, dxAff, Real(1), dxError );
        Real dxErrorNrm2 = Nrm2( dxError );

        dyError = rc;
        Hemv( LOWER,     Real(1), Q, dxAff, Real(1), dyError );
        Gemv( TRANSPOSE, Real(1), A, dyAff, Real(1), dyError );
        Axpy( Real(-1), dzAff, dyError );
        Real dyErrorNrm2 = Nrm2( dyError );

        Real rmuNrm2 = Nrm2( rmu );
        dzError = rmu;
        prod = dzAff;
        DiagonalScale( LEFT, NORMAL, x, prod );
        Axpy( Real(1), prod, dzError );
        prod = dxAff;
        DiagonalScale( LEFT, NORMAL, z, prod );
        Axpy( Real(1), prod, dzError );
        Real dzErrorNrm2 = Nrm2( dzError );

        if( ctrl.print && commRank == 0 )
            cout << "  || dxAffError ||_2 / (1 + || r_b ||_2) = " 
                 << dxErrorNrm2/(1+rbNrm2) << "\n"
                 << "  || dyAffError ||_2 / (1 + || r_c ||_2) = " 
                 << dyErrorNrm2/(1+rcNrm2) << "\n"
                 << "  || dzAffError ||_2 / (1 + || r_mu ||_2) = " 
                 << dzErrorNrm2/(1+rmuNrm2) << endl;
#endif

        // Compute the max affine [0,1]-step which keeps x and z in the cone
        // =================================================================
        const Real alphaAffPri = MaxStepInPositiveCone( x, dxAff, Real(1) );
        const Real alphaAffDual = MaxStepInPositiveCone( z, dzAff, Real(1) );
        if( ctrl.print && commRank == 0 )
            cout << "  alphaAffPri = " << alphaAffPri 
                 << ", alphaAffDual = " << alphaAffDual << endl;

        // Compute what the new duality measure would become
        // =================================================
        const Real mu = Dot(x,z) / n;
        // NOTE: dz and dx are used as temporaries
        dx = x;
        dz = z;
        Axpy( alphaAffPri,  dxAff, dx );
        Axpy( alphaAffDual, dzAff, dz );
        const Real muAff = Dot(dx,dz) / n;

        // Compute a centrality parameter using Mehrotra's formula
        // =======================================================
        // TODO: Allow the user to override this function
        const Real sigma = Pow(muAff/mu,Real(3)); 
        if( ctrl.print && commRank == 0 )
            cout << "  muAff = " << muAff 
                 << ", mu = " << mu 
                 << ", sigma = " << sigma << endl;

        // Solve for the centering-corrector 
        // =================================
        Zeros( rc, n, 1 );
        Zeros( rb, m, 1 );
        // r_mu := dxAff o dzAff - sigma*mu
        // --------------------------------
        rmu = dzAff;
        DiagonalScale( LEFT, NORMAL, dxAff, rmu );
        Shift( rmu, -sigma*mu );
        if( ctrl.system == FULL_KKT )
        {
            // Construct the new full KKT RHS
            // ------------------------------
            KKTRHS( rc, rb, rmu, z, d );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            ldl::SolveAfter( J, dSub, p, d, false );
            ExpandSolution( m, n, d, dx, dy, dz );
        }
        else if( ctrl.system == AUGMENTED_KKT )
        {
            // Construct the new "augmented" KKT RHS
            // -------------------------------------
            AugmentedKKTRHS( x, rc, rb, rmu, d );

            // Compute the proposed step from the KKT system
            // ---------------------------------------------
            ldl::SolveAfter( J, dSub, p, d, false );
            ExpandAugmentedSolution( x, z, rmu, d, dx, dy, dz );
        }
        else
            LogicError("Invalid KKT system choice");
        // TODO: Residual checks for center-corrector

        // Add in the affine search direction
        // ==================================
        Axpy( Real(1), dxAff, dx );
        Axpy( Real(1), dyAff, dy );
        Axpy( Real(1), dzAff, dz );

        // Compute max [0,1/maxStepRatio] step which keeps x and z in the cone
        // ===================================================================
        Real alphaPri = MaxStepInPositiveCone( x, dx, 1/ctrl.maxStepRatio );
        Real alphaDual = MaxStepInPositiveCone( z, dz, 1/ctrl.maxStepRatio );
        alphaPri = Min(ctrl.maxStepRatio*alphaPri,Real(1));
        alphaDual = Min(ctrl.maxStepRatio*alphaDual,Real(1));
        if( ctrl.print && commRank == 0 )
            cout << "  alphaPri = " << alphaPri 
                 << ", alphaDual = " << alphaDual << endl;

        // Update the current estimates
        // ============================
        Axpy( alphaPri,  dx, x );
        Axpy( alphaDual, dy, y );
        Axpy( alphaDual, dz, z ); 
    }

    if( ctrl.outerEquil )
    {
        // Unequilibrate the QP
        DiagonalSolve( LEFT, NORMAL, dCol, x );
        DiagonalSolve( LEFT, NORMAL, dRow, y );
        DiagonalScale( LEFT, NORMAL, dCol, z );
    }
}

template<typename Real>
void Mehrotra
( const SparseMatrix<Real>& QPre, const SparseMatrix<Real>& APre, 
  const Matrix<Real>& bPre,       const Matrix<Real>& cPre,
        Matrix<Real>& x,                Matrix<Real>& y, 
        Matrix<Real>& z,
  const MehrotraCtrl<Real>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("qp::direct::Mehrotra"))    

    // Equilibrate the QP by diagonally scaling A
    auto Q = QPre;
    auto A = APre;
    auto b = bPre;
    auto c = cPre;
    const Int m = A.Height();
    const Int n = A.Width();
    Matrix<Real> dRow, dCol;
    if( ctrl.outerEquil )
    {
        GeomEquil( A, dRow, dCol, ctrl.print );

        DiagonalSolve( LEFT, NORMAL, dRow, b );
        DiagonalSolve( LEFT, NORMAL, dCol, c );
        // TODO: Replace with SymmetricDiagonalSolve
        {
            DiagonalSolve( LEFT, NORMAL, dCol, Q );
            DiagonalSolve( RIGHT, NORMAL, dCol, Q );
        }
        if( ctrl.primalInit )
            DiagonalScale( LEFT, NORMAL, dCol, x );
        if( ctrl.dualInit )
        {
            DiagonalScale( LEFT, NORMAL, dRow, y );
            DiagonalSolve( LEFT, NORMAL, dCol, z );
        }
    }
    else
    {
        Ones( dRow, m, 1 );
        Ones( dCol, n, 1 );
    }

    const Real bNrm2 = Nrm2( b );
    const Real cNrm2 = Nrm2( c );

    vector<Int> map, invMap;
    SymmNodeInfo info;
    Separator rootSep;
    // The initialization involves an augmented KKT system, and so we can
    // only reuse the factorization metadata if the this IPM is using the
    // augmented formulation
    // TODO: Expose this as a parameter of MehrotraCtrl
    const bool standardShift = true;
    if( ctrl.system == AUGMENTED_KKT )
    {
        Initialize
        ( Q, A, b, c, x, y, z, map, invMap, rootSep, info,
          ctrl.primalInit, ctrl.dualInit, standardShift, ctrl.qsdCtrl );
    }  
    else
    {
        vector<Int> augMap, augInvMap;
        SymmNodeInfo augInfo;
        Separator augRootSep;
        Initialize
        ( Q, A, b, c, x, y, z, augMap, augInvMap, augRootSep, augInfo,
          ctrl.primalInit, ctrl.dualInit, standardShift, ctrl.qsdCtrl );
    }

    SparseMatrix<Real> J, JOrig;
    SymmFront<Real> JFront;
    Matrix<Real> d, 
                 rc,    rb,    rmu, 
                 dxAff, dyAff, dzAff,
                 dx,    dy,    dz;

    Matrix<Real> reg;
    if( ctrl.system == FULL_KKT )
    {
        reg.Resize( m+2*n, 1 );
        for( Int i=0; i<m+2*n; ++i )
        {
            if( i < n )
                reg.Set( i, 0, ctrl.qsdCtrl.regPrimal );
            else
                reg.Set( i, 0, -ctrl.qsdCtrl.regDual );
        }
    }
    else if( ctrl.system == AUGMENTED_KKT )
    {
        reg.Resize( n+m, 1 );
        for( Int i=0; i<n+m; ++i )
        {
            if( i < n )
                reg.Set( i, 0, ctrl.qsdCtrl.regPrimal );
            else
                reg.Set( i, 0, -ctrl.qsdCtrl.regDual );
        }
    }

    Matrix<Real> dInner;
#ifndef EL_RELEASE
    Matrix<Real> dxError, dyError, dzError, prod;
#endif
    for( Int numIts=0; ; ++numIts )
    {
        // Ensure that x and z are in the cone
        // ===================================
        const Int xNumNonPos = NumNonPositive( x );
        const Int zNumNonPos = NumNonPositive( z );
        if( xNumNonPos > 0 || zNumNonPos > 0 )
            LogicError
            (xNumNonPos," entries of x were nonpositive and ",
             zNumNonPos," entries of z were nonpositive");

        // Check for convergence
        // =====================
        // |primal - dual| / (1 + |primal|) <= tol ?
        // -----------------------------------------
        Zeros( d, n, 1 );
        // NOTE: The following assumes that Q is explicitly symmetric
        Multiply( NORMAL, Real(1), Q, x, Real(0), d );
        const Real xTQx = Dot(x,d);
        const Real primObj =  xTQx/2 + Dot(c,x);
        const Real dualObj = -xTQx/2 - Dot(b,y); 
        const Real objConv = Abs(primObj-dualObj) / (Real(1)+Abs(primObj));
        // || r_b ||_2 / (1 + || b ||_2) <= tol ?
        // --------------------------------------
        rb = b;
        Scale( Real(-1), rb );
        Multiply( NORMAL, Real(1), A, x, Real(1), rb );
        const Real rbNrm2 = Nrm2( rb );
        const Real rbConv = rbNrm2 / (Real(1)+bNrm2);
        // || r_c ||_2 / (1 + || c ||_2) <= tol ?
        // --------------------------------------
        rc = c;
        Multiply( NORMAL,    Real(1), Q, x, Real(1), rc );
        Multiply( TRANSPOSE, Real(1), A, y, Real(1), rc );
        Axpy( Real(-1), z, rc );
        const Real rcNrm2 = Nrm2( rc );
        const Real rcConv = rcNrm2 / (Real(1)+cNrm2);
        // Now check the pieces
        // --------------------
        if( ctrl.print )
            cout << " iter " << numIts << ":\n"
                 << "  |primal - dual| / (1 + |primal|) = "
                 << objConv << "\n"
                 << "  || r_b ||_2 / (1 + || b ||_2)   = "
                 << rbConv << "\n"
                 << "  || r_c ||_2 / (1 + || c ||_2)   = "
                 << rcConv << endl;
        if( objConv <= ctrl.tol && rbConv <= ctrl.tol && rcConv <= ctrl.tol )
            break;

        // Raise an exception after an unacceptable number of iterations
        // =============================================================
        if( numIts == ctrl.maxIts )
            RuntimeError
            ("Maximum number of iterations (",ctrl.maxIts,") exceeded");

        // r_mu := x o z
        // =============
        rmu = z; 
        DiagonalScale( LEFT, NORMAL, x, rmu );

        // Compute the affine search direction
        // ===================================
        if( ctrl.system == FULL_KKT || ctrl.system == AUGMENTED_KKT )
        {
            if( ctrl.system == FULL_KKT )
                KKT( Q, A, x, z, JOrig, false );
            else
                AugmentedKKT( Q, A, x, z, JOrig, false );
            J = JOrig;
            SymmetricEquil
            ( J, dInner,
              false, ctrl.innerEquil, 
              ctrl.scaleTwoNorm, ctrl.basisSize, ctrl.print );
            UpdateRealPartOfDiagonal( J, Real(1), reg );

            if( numIts == 0 )
            {
                NestedDissection( J.LockedGraph(), map, rootSep, info );
                InvertMap( map, invMap );
            }
            JFront.Pull( J, map, info );
            LDL( info, JFront, LDL_2D );

            if( ctrl.system == FULL_KKT )
                KKTRHS( rc, rb, rmu, z, d );
            else
                AugmentedKKTRHS( x, rc, rb, rmu, d );
            reg_qsd_ldl::SolveAfter
            ( JOrig, reg, dInner, invMap, info, JFront, d, ctrl.qsdCtrl );
            if( ctrl.system == FULL_KKT )
                ExpandSolution( m, n, d, dxAff, dyAff, dzAff );
            else
                ExpandAugmentedSolution( x, z, rmu, d, dxAff, dyAff, dzAff );
        }
        else
            LogicError("Invalid KKT system choice");
#ifndef EL_RELEASE
        // Sanity checks
        // =============
        dxError = rb;
        Multiply( NORMAL, Real(1), A, dxAff, Real(1), dxError );
        Real dxErrorNrm2 = Nrm2( dxError );

        dyError = rc;
        Multiply( NORMAL,    Real(1), Q, dxAff, Real(1), dyError );
        Multiply( TRANSPOSE, Real(1), A, dyAff, Real(1), dyError );
        Axpy( Real(-1), dzAff, dyError );
        Real dyErrorNrm2 = Nrm2( dyError );

        Real rmuNrm2 = Nrm2( rmu );
        dzError = rmu;
        prod = dzAff;
        DiagonalScale( LEFT, NORMAL, x, prod );
        Axpy( Real(1), prod, dzError );
        prod = dxAff;
        DiagonalScale( LEFT, NORMAL, z, prod );
        Axpy( Real(1), prod, dzError );
        Real dzErrorNrm2 = Nrm2( dzError );

        if( ctrl.print )
            cout << "  || dxAffError ||_2 / (1 + || r_b ||_2) = " 
                 << dxErrorNrm2/(1+rbNrm2) << "\n"
                 << "  || dyAffError ||_2 / (1 + || r_c ||_2) = " 
                 << dyErrorNrm2/(1+rcNrm2) << "\n"
                 << "  || dzAffError ||_2 / (1 + || r_mu ||_2) = " 
                 << dzErrorNrm2/(1+rmuNrm2) << endl;
#endif

        // Compute the max affine [0,1]-step which keeps x and z in the cone
        // =================================================================
        const Real alphaAffPri = MaxStepInPositiveCone( x, dxAff, Real(1) );
        const Real alphaAffDual = MaxStepInPositiveCone( z, dzAff, Real(1) );
        if( ctrl.print )
            cout << "  alphaAffPri = " << alphaAffPri 
                 << ", alphaAffDual = " << alphaAffDual << endl;

        // Compute what the new duality measure would become
        // =================================================
        const Real mu = Dot(x,z) / n;
        // NOTE: dz and dx are used as temporaries
        dx = x;
        dz = z;
        Axpy( alphaAffPri,  dxAff, dx );
        Axpy( alphaAffDual, dzAff, dz );
        const Real muAff = Dot(dx,dz) / n;

        // Compute a centrality parameter using Mehrotra's formula
        // =======================================================
        // TODO: Allow the user to override this function
        const Real sigma = Pow(muAff/mu,Real(3)); 
        if( ctrl.print )
            cout << "  muAff = " << muAff 
                 << ", mu = " << mu 
                 << ", sigma = " << sigma << endl;

        // Solve for the centering-corrector 
        // =================================
        Zeros( rc, n, 1 );
        Zeros( rb, m, 1 );
        // r_mu := dxAff o dzAff - sigma*mu
        // --------------------------------
        rmu = dzAff;
        DiagonalScale( LEFT, NORMAL, dxAff, rmu );
        Shift( rmu, -sigma*mu );
        if( ctrl.system == FULL_KKT )
        {
            KKTRHS( rc, rb, rmu, z, d );
            reg_qsd_ldl::SolveAfter
            ( JOrig, reg, dInner, invMap, info, JFront, d, ctrl.qsdCtrl );
            ExpandSolution( m, n, d, dx, dy, dz );
        }
        else if( ctrl.system == AUGMENTED_KKT )
        {
            AugmentedKKTRHS( x, rc, rb, rmu, d );
            reg_qsd_ldl::SolveAfter
            ( JOrig, reg, dInner, invMap, info, JFront, d, ctrl.qsdCtrl );
            ExpandAugmentedSolution( x, z, rmu, d, dx, dy, dz );
        }
        else
            LogicError("Invalid KKT system choice");
        // TODO: Residual checks for center-corrector

        // Add in the affine search direction
        // ==================================
        Axpy( Real(1), dxAff, dx );
        Axpy( Real(1), dyAff, dy );
        Axpy( Real(1), dzAff, dz );

        // Compute max [0,1/maxStepRatio] step which keeps x and z in the cone
        // ===================================================================
        Real alphaPri = MaxStepInPositiveCone( x, dx, 1/ctrl.maxStepRatio );
        Real alphaDual = MaxStepInPositiveCone( z, dz, 1/ctrl.maxStepRatio );
        alphaPri = Min(ctrl.maxStepRatio*alphaPri,Real(1));
        alphaDual = Min(ctrl.maxStepRatio*alphaDual,Real(1));
        if( ctrl.print )
            cout << "  alphaPri = " << alphaPri 
                 << ", alphaDual = " << alphaDual << endl;

        // Update the current estimates
        // ============================
        Axpy( alphaPri,  dx, x );
        Axpy( alphaDual, dy, y );
        Axpy( alphaDual, dz, z ); 
    }

    if( ctrl.outerEquil )
    {
        // Unequilibrate the QP
        DiagonalSolve( LEFT, NORMAL, dCol, x );
        DiagonalSolve( LEFT, NORMAL, dRow, y );
        DiagonalScale( LEFT, NORMAL, dCol, z );
    }
}

template<typename Real>
void Mehrotra
( const DistSparseMatrix<Real>& QPre, const DistSparseMatrix<Real>& APre, 
  const DistMultiVec<Real>& bPre,     const DistMultiVec<Real>& cPre,
        DistMultiVec<Real>& x,              DistMultiVec<Real>& y, 
        DistMultiVec<Real>& z,
  const MehrotraCtrl<Real>& ctrl )
{
    DEBUG_ONLY(CallStackEntry cse("qp::direct::Mehrotra"))    
    mpi::Comm comm = APre.Comm();
    const int commRank = mpi::Rank(comm);
    Timer timer;

    // Equilibrate the QP by diagonally scaling A
    auto Q = QPre;
    auto A = APre;
    auto b = bPre;
    auto c = cPre;
    const Int m = A.Height();
    const Int n = A.Width();
    DistMultiVec<Real> dRow(comm), dCol(comm);
    if( ctrl.outerEquil )
    {
        if( commRank == 0 && ctrl.time )
            timer.Start();
        GeomEquil( A, dRow, dCol, ctrl.print );
        if( commRank == 0 && ctrl.time )
            cout << "  GeomEquil: " << timer.Stop() << " secs" << endl;

        DiagonalSolve( LEFT, NORMAL, dRow, b );
        DiagonalSolve( LEFT, NORMAL, dCol, c );
        // TODO: Replace with SymmetricDiagonalSolve
        {
            DiagonalSolve( LEFT, NORMAL, dCol, Q );
            DiagonalSolve( RIGHT, NORMAL, dCol, Q );
        }
        if( ctrl.primalInit )
            DiagonalScale( LEFT, NORMAL, dCol, x );
        if( ctrl.dualInit )
        {
            DiagonalScale( LEFT, NORMAL, dRow, y );
            DiagonalSolve( LEFT, NORMAL, dCol, z );
        }
    }
    else
    {
        Ones( dRow, m, 1 );
        Ones( dCol, n, 1 );
    }

    const Real bNrm2 = Nrm2( b );
    const Real cNrm2 = Nrm2( c );

    DistMap map, invMap;
    DistSymmNodeInfo info;
    DistSeparator rootSep;
    // The initialization involves an augmented KKT system, and so we can
    // only reuse the factorization metadata if the this IPM is using the
    // augmented formulation
    // TODO: Expose this as a parameter of MehrotraCtrl
    const bool standardShift = true;
    if( commRank == 0 && ctrl.time )
        timer.Start();
    if( ctrl.system == AUGMENTED_KKT )
    {
        Initialize
        ( Q, A, b, c, x, y, z, map, invMap, rootSep, info,
          ctrl.primalInit, ctrl.dualInit, standardShift, ctrl.qsdCtrl ); 
    }  
    else
    {
        DistMap augMap, augInvMap;
        DistSymmNodeInfo augInfo;
        DistSeparator augRootSep;
        Initialize
        ( Q, A, b, c, x, y, z, augMap, augInvMap, augRootSep, augInfo,
          ctrl.primalInit, ctrl.dualInit, standardShift, ctrl.qsdCtrl );
    }
    if( commRank == 0 && ctrl.time )
        cout << "  Init: " << timer.Stop() << " secs" << endl;

    DistSparseMatrix<Real> J(comm), JOrig(comm);
    DistSymmFront<Real> JFront;
    DistMultiVec<Real> d(comm), 
                       rc(comm),    rb(comm),    rmu(comm), 
                       dxAff(comm), dyAff(comm), dzAff(comm),
                       dx(comm),    dy(comm),    dz(comm);

    DistMultiVec<Real> reg(comm);
    if( ctrl.system == FULL_KKT )
    {
        reg.Resize( m+2*n, 1 );
        for( Int iLoc=0; iLoc<reg.LocalHeight(); ++iLoc )
        {
            const Int i = reg.GlobalRow(iLoc);
            if( i < n )
                reg.SetLocal( iLoc, 0, ctrl.qsdCtrl.regPrimal );
            else
                reg.SetLocal( iLoc, 0, -ctrl.qsdCtrl.regDual );
        }
    }
    else if( ctrl.system == AUGMENTED_KKT )
    {
        reg.Resize( n+m, 1 );
        for( Int iLoc=0; iLoc<reg.LocalHeight(); ++iLoc )
        {
            const Int i = reg.GlobalRow(iLoc);
            if( i < n )
                reg.SetLocal( iLoc, 0, ctrl.qsdCtrl.regPrimal );
            else
                reg.SetLocal( iLoc, 0, -ctrl.qsdCtrl.regDual );
        }
    }

    DistMultiVec<Real> dInner(comm);
#ifndef EL_RELEASE
    DistMultiVec<Real> dxError(comm), dyError(comm), dzError(comm), prod(comm);
#endif
    for( Int numIts=0; ; ++numIts )
    {
        // Ensure that x and z are in the cone
        // ===================================
        const Int xNumNonPos = NumNonPositive( x );
        const Int zNumNonPos = NumNonPositive( z );
        if( xNumNonPos > 0 || zNumNonPos > 0 )
            LogicError
            (xNumNonPos," entries of x were nonpositive and ",
             zNumNonPos," entries of z were nonpositive");

        // Check for convergence
        // =====================
        // |primal - dual| / (1 + |primal|) <= tol ?
        // -----------------------------------------
        Zeros( d, n, 1 );
        // NOTE: The following assumes that Q is explicitly symmetric
        Multiply( NORMAL, Real(1), Q, x, Real(0), d );
        const Real xTQx = Dot(x,d);
        const Real primObj =  xTQx/2 + Dot(c,x);
        const Real dualObj = -xTQx/2 - Dot(b,y); 
        const Real objConv = Abs(primObj-dualObj) / (Real(1)+Abs(primObj));
        // || r_b ||_2 / (1 + || b ||_2) <= tol ?
        // --------------------------------------
        rb = b;
        Scale( Real(-1), rb );
        Multiply( NORMAL, Real(1), A, x, Real(1), rb );
        const Real rbNrm2 = Nrm2( rb );
        const Real rbConv = rbNrm2 / (Real(1)+bNrm2);
        // || r_c ||_2 / (1 + || c ||_2) <= tol ?
        // --------------------------------------
        rc = c;
        Multiply( NORMAL,    Real(1), Q, x, Real(1), rc );
        Multiply( TRANSPOSE, Real(1), A, y, Real(1), rc );
        Axpy( Real(-1), z, rc );
        const Real rcNrm2 = Nrm2( rc );
        const Real rcConv = rcNrm2 / (Real(1)+cNrm2);
        // Now check the pieces
        // --------------------
        if( ctrl.print && commRank == 0 )
            cout << " iter " << numIts << ":\n"
                 << "  |primal - dual| / (1 + |primal|) = "
                 << objConv << "\n"
                 << "  || r_b ||_2 / (1 + || b ||_2)   = "
                 << rbConv << "\n"
                 << "  || r_c ||_2 / (1 + || c ||_2)   = "
                 << rcConv << endl;
        if( objConv <= ctrl.tol && rbConv <= ctrl.tol && rcConv <= ctrl.tol )
            break;

        // Raise an exception after an unacceptable number of iterations
        // =============================================================
        if( numIts == ctrl.maxIts )
            RuntimeError
            ("Maximum number of iterations (",ctrl.maxIts,") exceeded");

        // r_mu := x o z
        // =============
        rmu = z; 
        DiagonalScale( LEFT, NORMAL, x, rmu );

        // Compute the affine search direction
        // ===================================
        if( ctrl.system == FULL_KKT || ctrl.system == AUGMENTED_KKT )
        {
            if( ctrl.system == FULL_KKT )
                KKT( Q, A, x, z, JOrig, false );
            else
                AugmentedKKT( Q, A, x, z, JOrig, false );
            J = JOrig;
            SymmetricEquil
            ( J, dInner,
              false, ctrl.innerEquil, 
              ctrl.scaleTwoNorm, ctrl.basisSize, ctrl.print );
            UpdateRealPartOfDiagonal( J, Real(1), reg );

            if( numIts == 0 )
            {
                if( commRank == 0 && ctrl.time )
                    timer.Start();
                NestedDissection( J.LockedDistGraph(), map, rootSep, info );
                if( commRank == 0 && ctrl.time )
                    cout << "  ND: " << timer.Stop() << " secs" << endl;
                InvertMap( map, invMap );
            }
            JFront.Pull( J, map, rootSep, info );
            if( commRank == 0 && ctrl.time )
                timer.Start();
            LDL( info, JFront, LDL_2D );
            if( commRank == 0 && ctrl.time )
                cout << "  LDL: " << timer.Stop() << " secs" << endl;

            if( ctrl.system == FULL_KKT )
                KKTRHS( rc, rb, rmu, z, d );
            else
                AugmentedKKTRHS( x, rc, rb, rmu, d );
            if( commRank == 0 && ctrl.time )
                timer.Start();
            reg_qsd_ldl::SolveAfter
            ( JOrig, reg, dInner, invMap, info, JFront, d, ctrl.qsdCtrl );
            if( commRank == 0 && ctrl.time )
                cout << "  Affine: " << timer.Stop() << " secs" << endl;
            if( ctrl.system == FULL_KKT )
                ExpandSolution( m, n, d, dxAff, dyAff, dzAff );
            else
                ExpandAugmentedSolution( x, z, rmu, d, dxAff, dyAff, dzAff );
        }
        else
            LogicError("Invalid KKT system choice");
#ifndef EL_RELEASE
        // Sanity checks
        // =============
        dxError = rb;
        Multiply( NORMAL, Real(1), A, dxAff, Real(1), dxError );
        Real dxErrorNrm2 = Nrm2( dxError );

        dyError = rc;
        Multiply( NORMAL,    Real(1), Q, dxAff, Real(1), dyError );
        Multiply( TRANSPOSE, Real(1), A, dyAff, Real(1), dyError );
        Axpy( Real(-1), dzAff, dyError );
        Real dyErrorNrm2 = Nrm2( dyError );

        Real rmuNrm2 = Nrm2( rmu );
        dzError = rmu;
        prod = dzAff;
        DiagonalScale( LEFT, NORMAL, x, prod );
        Axpy( Real(1), prod, dzError );
        prod = dxAff;
        DiagonalScale( LEFT, NORMAL, z, prod );
        Axpy( Real(1), prod, dzError );
        Real dzErrorNrm2 = Nrm2( dzError );

        if( ctrl.print && commRank == 0 )
            cout << "  || dxAffError ||_2 / (1 + || r_b ||_2) = " 
                 << dxErrorNrm2/(1+rbNrm2) << "\n"
                 << "  || dyAffError ||_2 / (1 + || r_c ||_2) = " 
                 << dyErrorNrm2/(1+rcNrm2) << "\n"
                 << "  || dzAffError ||_2 / (1 + || r_mu ||_2) = " 
                 << dzErrorNrm2/(1+rmuNrm2) << endl;
#endif

        // Compute the max affine [0,1]-step which keeps x and z in the cone
        // =================================================================
        const Real alphaAffPri = MaxStepInPositiveCone( x, dxAff, Real(1) );
        const Real alphaAffDual = MaxStepInPositiveCone( z, dzAff, Real(1) );
        if( ctrl.print && commRank == 0 )
            cout << "  alphaAffPri = " << alphaAffPri 
                 << ", alphaAffDual = " << alphaAffDual << endl;

        // Compute what the new duality measure would become
        // =================================================
        const Real mu = Dot(x,z) / n;
        // NOTE: dz and dx are used as temporaries
        dx = x;
        dz = z;
        Axpy( alphaAffPri,  dxAff, dx );
        Axpy( alphaAffDual, dzAff, dz );
        const Real muAff = Dot(dx,dz) / n;

        // Compute a centrality parameter using Mehrotra's formula
        // =======================================================
        // TODO: Allow the user to override this function
        const Real sigma = Pow(muAff/mu,Real(3)); 
        if( ctrl.print && commRank == 0 )
            cout << "  muAff = " << muAff 
                 << ", mu = " << mu 
                 << ", sigma = " << sigma << endl;

        // Solve for the centering-corrector 
        // =================================
        Zeros( rc, n, 1 );
        Zeros( rb, m, 1 );
        // r_mu := dxAff o dzAff - sigma*mu
        // --------------------------------
        rmu = dzAff;
        DiagonalScale( LEFT, NORMAL, dxAff, rmu );
        Shift( rmu, -sigma*mu );
        if( ctrl.system == FULL_KKT )
        {
            KKTRHS( rc, rb, rmu, z, d );
            if( commRank == 0 && ctrl.time )
                timer.Start();
            reg_qsd_ldl::SolveAfter
            ( JOrig, reg, dInner, invMap, info, JFront, d, ctrl.qsdCtrl );
            if( commRank == 0 && ctrl.time )
                cout << "  Corrector: " << timer.Stop() << " secs" << endl;
            ExpandSolution( m, n, d, dx, dy, dz );
        }
        else if( ctrl.system == AUGMENTED_KKT )
        {
            AugmentedKKTRHS( x, rc, rb, rmu, d );
            if( commRank == 0 && ctrl.time )
                timer.Start();
            reg_qsd_ldl::SolveAfter
            ( JOrig, reg, dInner, invMap, info, JFront, d, ctrl.qsdCtrl );
            if( commRank == 0 && ctrl.time )
                cout << "  Corrector: " << timer.Stop() << " secs" << endl;
            ExpandAugmentedSolution( x, z, rmu, d, dx, dy, dz );
        }
        else
            LogicError("Invalid KKT system choice");
        // TODO: Residual checks for center-corrector

        // Add in the affine search direction
        // ==================================
        Axpy( Real(1), dxAff, dx );
        Axpy( Real(1), dyAff, dy );
        Axpy( Real(1), dzAff, dz );

        // Compute max [0,1/maxStepRatio] step which keeps x and z in the cone
        // ===================================================================
        Real alphaPri = MaxStepInPositiveCone( x, dx, 1/ctrl.maxStepRatio );
        Real alphaDual = MaxStepInPositiveCone( z, dz, 1/ctrl.maxStepRatio );
        alphaPri = Min(ctrl.maxStepRatio*alphaPri,Real(1));
        alphaDual = Min(ctrl.maxStepRatio*alphaDual,Real(1));
        if( ctrl.print && commRank == 0 )
            cout << "  alphaPri = " << alphaPri 
                 << ", alphaDual = " << alphaDual << endl;

        // Update the current estimates
        // ============================
        Axpy( alphaPri,  dx, x );
        Axpy( alphaDual, dy, y );
        Axpy( alphaDual, dz, z ); 
    }

    if( ctrl.outerEquil )
    {
        // Unequilibrate the QP
        DiagonalSolve( LEFT, NORMAL, dCol, x );
        DiagonalSolve( LEFT, NORMAL, dRow, y );
        DiagonalScale( LEFT, NORMAL, dCol, z );
    }
}

#define PROTO(Real) \
  template void Mehrotra \
  ( const Matrix<Real>& Q, const Matrix<Real>& A, \
    const Matrix<Real>& b, const Matrix<Real>& c, \
          Matrix<Real>& x,       Matrix<Real>& y, \
          Matrix<Real>& z, \
    const MehrotraCtrl<Real>& ctrl ); \
  template void Mehrotra \
  ( const AbstractDistMatrix<Real>& Q, const AbstractDistMatrix<Real>& A, \
    const AbstractDistMatrix<Real>& b, const AbstractDistMatrix<Real>& c, \
          AbstractDistMatrix<Real>& x,       AbstractDistMatrix<Real>& y, \
          AbstractDistMatrix<Real>& z, \
    const MehrotraCtrl<Real>& ctrl ); \
  template void Mehrotra \
  ( const SparseMatrix<Real>& Q, const SparseMatrix<Real>& A, \
    const Matrix<Real>& b,       const Matrix<Real>& c, \
          Matrix<Real>& x,             Matrix<Real>& y, \
          Matrix<Real>& z, \
    const MehrotraCtrl<Real>& ctrl ); \
  template void Mehrotra \
  ( const DistSparseMatrix<Real>& Q, const DistSparseMatrix<Real>& A, \
    const DistMultiVec<Real>& b,     const DistMultiVec<Real>& c, \
          DistMultiVec<Real>& x,           DistMultiVec<Real>& y, \
          DistMultiVec<Real>& z, \
    const MehrotraCtrl<Real>& ctrl );

#define EL_NO_INT_PROTO
#define EL_NO_COMPLEX_PROTO
#include "El/macros/Instantiate.h"

} // namespace direct
} // namespace qp
} // namespace El
