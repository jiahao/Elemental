/*
   Copyright (c) 2009-2012, Jack Poulson
   All rights reserved.

   This file is part of Elemental.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

    - Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

    - Neither the name of the owner nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/
#include <ctime>
#include "elemental.hpp"
using namespace std;
using namespace elem;

template<typename F> 
void TestCorrectness
( bool print, UpperOrLower uplo, UnitOrNonUnit diag,
  const DistMatrix<F>& A, const DistMatrix<F>& B, const DistMatrix<F>& AOrig )
{
    typedef typename Base<F>::type R;
    const Grid& g = A.Grid();
    const int m = AOrig.Height();

    const int k=100;
    DistMatrix<F> X(g), Y(g), Z(g);
    Uniform( m, k, X );
    Y = X;
    Zeros( m, k, Z );

    if( uplo == LOWER )
    {
        // Test correctness by comparing the application of A against a 
        // random set of k vectors to the application of 
        // tril(B)^H AOrig tril(B)
        Trmm( LEFT, LOWER, NORMAL, diag, F(1), B, Y );
        Hemm( LEFT, LOWER, F(1), AOrig, Y, F(0), Z );
        Trmm( LEFT, LOWER, ADJOINT, diag, F(1), B, Z );
        Hemm( LEFT, LOWER, F(-1), A, X, F(1), Z );
        R infNormOfAOrig = HermitianNorm( uplo, AOrig, INFINITY_NORM );
        R frobNormOfAOrig = HermitianNorm( uplo, AOrig, FROBENIUS_NORM );
        R infNormOfA = HermitianNorm( uplo, A, INFINITY_NORM );
        R frobNormOfA = HermitianNorm( uplo, A, FROBENIUS_NORM );
        R oneNormOfError = Norm( Z, ONE_NORM );
        R infNormOfError = Norm( Z, INFINITY_NORM );
        R frobNormOfError = Norm( Z, FROBENIUS_NORM );
        if( g.Rank() == 0 )
        {
            cout << "||AOrig||_1 = ||AOrig||_oo     = "
                 << infNormOfAOrig << "\n"
                 << "||AOrig||_F                    = "
                 << frobNormOfAOrig << "\n"
                 << "||A||_1 = ||A||_oo             = "
                 << infNormOfA << "\n"
                 << "||A||_F                        = "
                 << frobNormOfA << "\n"
                 << "||A X - L^H AOrig L X||_1  = "
                 << oneNormOfError << "\n"
                 << "||A X - L^H AOrig L X||_oo = " 
                 << infNormOfError << "\n"
                 << "||A X - L^H AOrig L X||_F  = "
                 << frobNormOfError << endl;
        }
    }
    else
    {
        // Test correctness by comparing the application of A against a 
        // random set of k vectors to the application of 
        // triu(B) AOrig triu(B)^H
        Trmm( LEFT, UPPER, ADJOINT, diag, F(1), B, Y );
        Hemm( LEFT, UPPER, F(1), AOrig, Y, F(0), Z );
        Trmm( LEFT, UPPER, NORMAL, diag, F(1), B, Z );
        Hemm( LEFT, UPPER, F(-1), A, X, F(1), Z );
        R infNormOfAOrig = HermitianNorm( uplo, AOrig, INFINITY_NORM );
        R frobNormOfAOrig = HermitianNorm( uplo, AOrig, FROBENIUS_NORM );
        R infNormOfA = HermitianNorm( uplo, A, INFINITY_NORM );
        R frobNormOfA = HermitianNorm( uplo, A, FROBENIUS_NORM );
        R oneNormOfError = Norm( Z, ONE_NORM );
        R infNormOfError = Norm( Z, INFINITY_NORM );
        R frobNormOfError = Norm( Z, FROBENIUS_NORM );
        if( g.Rank() == 0 )
        {
            cout << "||AOrig||_1 = ||AOrig||_oo     = "
                 << infNormOfAOrig << "\n"
                 << "||AOrig||_F                    = "
                 << frobNormOfAOrig << "\n"
                 << "||A||_1 = ||A||_oo             = "
                 << infNormOfA << "\n"
                 << "||A||_F                        = "
                 << frobNormOfA << "\n"
                 << "||A X - U AOrig U^H X||_1  = "
                 << oneNormOfError << "\n"
                 << "||A X - U AOrig U^H X||_oo = " 
                 << infNormOfError << "\n"
                 << "||A X - U AOrig U^H X||_F  = "
                 << frobNormOfError << endl;
        }
    }
}

template<typename F> 
void TestTwoSidedTrmm
( bool testCorrectness, bool print, UpperOrLower uplo, UnitOrNonUnit diag, 
  int m, const Grid& g )
{
    DistMatrix<F> A(g), B(g), AOrig(g);

    Zeros( m, m, A );
    Zeros( m, m, B );
    MakeHermitianUniformSpectrum( A, 1, 10 );
    MakeHermitianUniformSpectrum( B, 1, 10 );
    MakeTrapezoidal( LEFT, uplo, 0, B );
    if( testCorrectness )
    {
        if( g.Rank() == 0 )
        {
            cout << "  Making copy of original matrix...";
            cout.flush();
        }
        AOrig = A;
        if( g.Rank() == 0 )
            cout << "DONE" << endl;
    }
    if( print )
    {
        A.Print("A");
        B.Print("B");
    }

    if( g.Rank() == 0 )
    {
        cout << "  Starting reduction to Hermitian standard EVP...";
        cout.flush();
    }
    mpi::Barrier( g.Comm() );
    const double startTime = mpi::Time();
    TwoSidedTrmm( uplo, diag, A, B );
    mpi::Barrier( g.Comm() );
    const double runTime = mpi::Time() - startTime;
    double gFlops = Pow(double(m),3.)/(runTime*1.e9);
    if( IsComplex<F>::val )
        gFlops *= 4.;
    if( g.Rank() == 0 )
    {
        cout << "DONE. " << endl
             << "  Time = " << runTime << " seconds. GFlops = "
             << gFlops << endl;
    }
    if( print )
        A.Print("A after reduction");
    if( testCorrectness )
        TestCorrectness( print, uplo, diag, A, B, AOrig );
}

int 
main( int argc, char* argv[] )
{
    Initialize( argc, argv );
    mpi::Comm comm = mpi::COMM_WORLD;
    const int commRank = mpi::CommRank( comm );
    const int commSize = mpi::CommSize( comm );

    try
    {
        MpiArgs args( argc, argv, comm );
        int r = args.Optional("--r",0,"height of process grid");
        const char uploChar = args.Optional
            ("--uplo",'L',"lower or upper triangular storage: L/U");
        const char diagChar = args.Optional
            ("--unit",'N',"(non-)unit diagonal: N/U");
        const int m = args.Optional("--m",100,"height of matrix");
        const int nb = args.Optional("--nb",96,"algorithmic blocksize");
        const bool testCorrectness = args.Optional
            ("--correctness",false,"test correctness?");
        const bool print = args.Optional("--print",false,"print matrices?");
        args.Process();

        if( r == 0 )
            r = Grid::FindFactor( commSize );
        if( commSize % r != 0 )
            throw std::logic_error("Invalid process grid height");
        const int c = commSize / r;
        const Grid g( comm, r, c );
        const UpperOrLower uplo = CharToUpperOrLower( uploChar );
        const UnitOrNonUnit diag = CharToUnitOrNonUnit( diagChar );
        SetBlocksize( nb );

#ifndef RELEASE
        if( commRank == 0 )
        {
            cout << "==========================================\n"
                 << " In debug mode! Performance will be poor! \n"
                 << "==========================================" << endl;
        }
#endif
        if( commRank == 0 )
            cout << "Will test TwoSidedTrmm" << uploChar << diagChar << endl;

        if( commRank == 0 )
        {
            cout << "---------------------\n"
                 << "Testing with doubles:\n"
                 << "---------------------" << endl;
        }
        TestTwoSidedTrmm<double>( testCorrectness, print, uplo, diag, m, g );

        if( commRank == 0 )
        {
            cout << "--------------------------------------\n"
                 << "Testing with double-precision complex:\n"
                 << "--------------------------------------" << endl;
        }
        TestTwoSidedTrmm<Complex<double> >
        ( testCorrectness, print, uplo, diag, m, g );
    }
    catch( ArgException& e ) { }
    catch( exception& e )
    {
        ostringstream os;
        os << "Process " << commRank << " caught error message:\n" << e.what()
           << endl; 
        cerr << os.str();
#ifndef RELEASE
        DumpCallStack();
#endif
    }
    Finalize();
    return 0;
}
