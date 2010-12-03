/*
   Copyright (c) 2009-2010, Jack Poulson
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
#ifndef WITHOUT_PMRRR
#include "elemental/lapack.hpp"
using namespace elemental;

// Grab the full set of eigenpairs.
template<typename R>
void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape, 
  DistMatrix<R,MC,  MR>& A,
  DistMatrix<R,MC,  MR>& B,
  DistMatrix<R,Star,VR>& w,
  DistMatrix<R,MC,  MR>& X,
  bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::GeneralizedHermitianEig");
    // TODO: Checks for input consistency
#endif
    const Side side = ( genEigType==AXBX ? Right : Left );

    lapack::Chol( shape, B );
    lapack::Hegst( side, shape, A, B );
    lapack::HermitianEig( shape, A, w, X, tryForHighAccuracy );
    if( genEigType == AXBX || genEigType == ABX )
    {
        if( shape == Lower )
            blas::Trsm( Left, Lower, ConjugateTranspose, NonUnit, (R)1, B, X );
        else
            blas::Trsm( Left, Upper, Normal, NonUnit, (R)1, B, X );
    }
    else /* genEigType == BAX */
    {
        if( shape == Lower )
            blas::Trmm( Left, Lower, Normal, NonUnit, (R)1, B, X );
        else
            blas::Trmm( Left, Upper, ConjugateTranspose, NonUnit, (R)1, B, X );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

// Grab a partial set of eigenpairs. 
// The partial set is determined by the inclusive zero-indexed range 
//   a,a+1,...,b    ; a >= 0, b < n  
// of the n eigenpairs sorted from smallest to largest eigenvalues.  
template<typename R>
void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape, 
  DistMatrix<R,MC,  MR>& A,
  DistMatrix<R,MC,  MR>& B,
  DistMatrix<R,Star,VR>& w,
  DistMatrix<R,MC,  MR>& X,
  int a, int b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::GeneralizedHermitianEig");
    // TODO: Checks for input consistency
#endif
    const Side side = ( genEigType==AXBX ? Right : Left );

    lapack::Chol( shape, B );
    lapack::Hegst( side, shape, A, B );
    lapack::HermitianEig( shape, A, w, X, a, b, tryForHighAccuracy );
    if( genEigType == AXBX || genEigType == ABX )
    {
        if( shape == Lower )
            blas::Trsm( Left, Lower, ConjugateTranspose, NonUnit, (R)1, B, X );
        else
            blas::Trsm( Left, Upper, Normal, NonUnit, (R)1, B, X );
    }
    else /* genEigType == BAX */
    {
        if( shape == Lower )
            blas::Trmm( Left, Lower, Normal, NonUnit, (R)1, B, X );
        else
            blas::Trmm( Left, Upper, ConjugateTranspose, NonUnit, (R)1, B, X );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

// Grab a partial set of eigenpairs.
// The partial set is determined by the half-open interval (a,b]
template<typename R>
void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape, 
  DistMatrix<R,MC,  MR>& A,
  DistMatrix<R,MC,  MR>& B,
  DistMatrix<R,Star,VR>& w,
  DistMatrix<R,MC,  MR>& X,
  R a, R b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::GeneralizedHermitianEig");
    // TODO: Checks for input consistency
#endif
    const Side side = ( genEigType==AXBX ? Right : Left );

    lapack::Chol( shape, B );
    lapack::Hegst( side, shape, A, B );
    lapack::HermitianEig( shape, A, w, X, a, b, tryForHighAccuracy );
    if( genEigType == AXBX || genEigType == ABX )
    {
        if( shape == Lower )
            blas::Trsm( Left, Lower, ConjugateTranspose, NonUnit, (R)1, B, X );
        else
            blas::Trsm( Left, Upper, Normal, NonUnit, (R)1, B, X );
    }
    else /* genEigType == BAX */
    {
        if( shape == Lower )
            blas::Trmm( Left, Lower, Normal, NonUnit, (R)1, B, X );
        else
            blas::Trmm( Left, Upper, ConjugateTranspose, NonUnit, (R)1, B, X );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

// Grab the full set of eigenvalues
template<typename R>
void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape, 
  DistMatrix<R,MC,  MR>& A,
  DistMatrix<R,MC,  MR>& B,
  DistMatrix<R,Star,VR>& w,
  bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::GeneralizedHermitianEig");
    // TODO: Checks for input consistency
#endif
    const Side side = ( genEigType==AXBX ? Right : Left );

    lapack::Chol( shape, B );
    lapack::Hegst( side, shape, A, B );
    lapack::HermitianEig( shape, A, w, tryForHighAccuracy );
#ifndef RELEASE
    PopCallStack();
#endif
}

// Grab a partial set of eigenvalues. 
// The partial set is determined by the inclusive zero-indexed range 
//   a,a+1,...,b    ; a >= 0, b < n  
// of the n eigenpairs sorted from smallest to largest eigenvalues.  
template<typename R>
void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape, 
  DistMatrix<R,MC,  MR>& A,
  DistMatrix<R,MC,  MR>& B,
  DistMatrix<R,Star,VR>& w,
  int a, int b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::GeneralizedHermitianEig");
    // TODO: Checks for input consistency
#endif
    const Side side = ( genEigType==AXBX ? Right : Left );

    lapack::Chol( shape, B );
    lapack::Hegst( side, shape, A, B );
    lapack::HermitianEig( shape, A, w, a, b, tryForHighAccuracy );
#ifndef RELEASE
    PopCallStack();
#endif
}

// Grab a partial set of eigenvalues.
// The partial set is determined by the half-open interval (a,b]
template<typename R>
void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape, 
  DistMatrix<R,MC,  MR>& A,
  DistMatrix<R,MC,  MR>& B,
  DistMatrix<R,Star,VR>& w,
  R a, R b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::GeneralizedHermitianEig");
    // TODO: Checks for input consistency
#endif
    const Side side = ( genEigType==AXBX ? Right : Left );

    lapack::Chol( shape, B );
    lapack::Hegst( side, shape, A, B );
    lapack::HermitianEig( shape, A, w, a, b, tryForHighAccuracy );
#ifndef RELEASE
    PopCallStack();
#endif
}

#ifndef WITHOUT_COMPLEX
// Grab the full set of eigenpairs
template<typename R>
void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape, 
  DistMatrix<std::complex<R>,MC,  MR>& A,
  DistMatrix<std::complex<R>,MC,  MR>& B,
  DistMatrix<             R, Star,VR>& w,
  DistMatrix<std::complex<R>,MC,  MR>& X,
  bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::GeneralizedHermitianEig");
    // TODO: Checks for input consistency
#endif
    const Side side = ( genEigType==AXBX ? Right : Left );

    lapack::Chol( shape, B );
    lapack::Hegst( side, shape, A, B );
    lapack::HermitianEig( shape, A, w, X, tryForHighAccuracy );
    if( genEigType == AXBX || genEigType == ABX )
    {
        if( shape == Lower )
        {
            blas::Trsm
            ( Left, Lower, ConjugateTranspose, NonUnit, 
              std::complex<R>(1), B, X );
        }
        else
        {
            blas::Trsm
            ( Left, Upper, Normal, NonUnit, 
              std::complex<R>(1), B, X );
        }
    }
    else /* genEigType == BAX */
    {
        if( shape == Lower )
        {
            blas::Trmm
            ( Left, Lower, Normal, NonUnit,
              std::complex<R>(1), B, X );
        }
        else
        {
            blas::Trmm
            ( Left, Upper, ConjugateTranspose, NonUnit,
              std::complex<R>(1), B, X );
        }
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

// Grab a partial set of eigenpairs. 
// The partial set is determined by the inclusive zero-indexed range 
//   a,a+1,...,b    ; a >= 0, b < n  
// of the n eigenpairs sorted from smallest to largest eigenvalues.  
template<typename R>
void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape, 
  DistMatrix<std::complex<R>,MC,  MR>& A,
  DistMatrix<std::complex<R>,MC,  MR>& B,
  DistMatrix<             R, Star,VR>& w,
  DistMatrix<std::complex<R>,MC,  MR>& X,
  int a, int b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::GeneralizedHermitianEig");
    // TODO: Checks for input consistency
#endif
    const Side side = ( genEigType==AXBX ? Right : Left );

    lapack::Chol( shape, B );
    lapack::Hegst( side, shape, A, B );
    lapack::HermitianEig( shape, A, w, X, a, b, tryForHighAccuracy );
    if( genEigType == AXBX || genEigType == ABX )
    {
        if( shape == Lower )
        {
            blas::Trsm
            ( Left, Lower, ConjugateTranspose, NonUnit, 
              std::complex<R>(1), B, X );
        }
        else
        {
            blas::Trsm
            ( Left, Upper, Normal, NonUnit, 
              std::complex<R>(1), B, X );
        }
    }
    else /* genEigType == BAX */
    {
        if( shape == Lower )
        {
            blas::Trmm
            ( Left, Lower, Normal, NonUnit,
              std::complex<R>(1), B, X );
        }
        else
        {
            blas::Trmm
            ( Left, Upper, ConjugateTranspose, NonUnit,
              std::complex<R>(1), B, X );
        }
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

// Grab a partial set of eigenpairs.
// The partial set is determined by the half-open interval (a,b]
template<typename R>
void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape, 
  DistMatrix<std::complex<R>,MC,  MR>& A,
  DistMatrix<std::complex<R>,MC,  MR>& B,
  DistMatrix<             R, Star,VR>& w,
  DistMatrix<std::complex<R>,MC,  MR>& X,
  R a, R b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::GeneralizedHermitianEig");
    // TODO: Checks for input consistency
#endif
    const Side side = ( genEigType==AXBX ? Right : Left );

    lapack::Chol( shape, B );
    lapack::Hegst( side, shape, A, B );
    lapack::HermitianEig( shape, A, w, X, a, b, tryForHighAccuracy );
    if( genEigType == AXBX || genEigType == ABX )
    {
        if( shape == Lower )
        {
            blas::Trsm
            ( Left, Lower, ConjugateTranspose, NonUnit, 
              std::complex<R>(1), B, X );
        }
        else
        {
            blas::Trsm
            ( Left, Upper, Normal, NonUnit, 
              std::complex<R>(1), B, X );
        }
    }
    else /* genEigType == BAX */
    {
        if( shape == Lower )
        {
            blas::Trmm
            ( Left, Lower, Normal, NonUnit,
              std::complex<R>(1), B, X );
        }
        else
        {
            blas::Trmm
            ( Left, Upper, ConjugateTranspose, NonUnit,
              std::complex<R>(1), B, X );
        }
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

// Grab a full set of eigenvalues
template<typename R>
void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape, 
  DistMatrix<std::complex<R>,MC,  MR>& A,
  DistMatrix<std::complex<R>,MC,  MR>& B,
  DistMatrix<             R, Star,VR>& w,
  bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::GeneralizedHermitianEig");
    // TODO: Checks for input consistency
#endif
    const Side side = ( genEigType==AXBX ? Right : Left );

    lapack::Chol( shape, B );
    lapack::Hegst( side, shape, A, B );
    lapack::HermitianEig( shape, A, w, tryForHighAccuracy );
#ifndef RELEASE
    PopCallStack();
#endif
}

// Grab a partial set of eigenvalues. 
// The partial set is determined by the inclusive zero-indexed range 
//   a,a+1,...,b    ; a >= 0, b < n  
// of the n eigenpairs sorted from smallest to largest eigenvalues.  
template<typename R>
void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape, 
  DistMatrix<std::complex<R>,MC,  MR>& A,
  DistMatrix<std::complex<R>,MC,  MR>& B,
  DistMatrix<             R, Star,VR>& w,
  int a, int b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::GeneralizedHermitianEig");
    // TODO: Checks for input consistency
#endif
    const Side side = ( genEigType==AXBX ? Right : Left );

    lapack::Chol( shape, B );
    lapack::Hegst( side, shape, A, B );
    lapack::HermitianEig( shape, A, w, a, b, tryForHighAccuracy );
#ifndef RELEASE
    PopCallStack();
#endif
}

// Grab a partial set of eigenvalues.
// The partial set is determined by the half-open interval (a,b]
template<typename R>
void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape, 
  DistMatrix<std::complex<R>,MC,  MR>& A,
  DistMatrix<std::complex<R>,MC,  MR>& B,
  DistMatrix<             R, Star,VR>& w,
  R a, R b, bool tryForHighAccuracy )
{
#ifndef RELEASE
    PushCallStack("lapack::GeneralizedHermitianEig");
    // TODO: Checks for input consistency
#endif
    const Side side = ( genEigType==AXBX ? Right : Left );

    lapack::Chol( shape, B );
    lapack::Hegst( side, shape, A, B );
    lapack::HermitianEig( shape, A, w, a, b, tryForHighAccuracy );
#ifndef RELEASE
    PopCallStack();
#endif
}
#endif // WITHOUT_COMPLEX

template void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape,
  DistMatrix<double,MC,  MR>& A,
  DistMatrix<double,MC,  MR>& B,
  DistMatrix<double,Star,VR>& w,
  DistMatrix<double,MC,  MR>& X,
  bool tryForHighAccuracy );

template void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape,
  DistMatrix<double,MC,  MR>& A,
  DistMatrix<double,MC,  MR>& B,
  DistMatrix<double,Star,VR>& w,
  DistMatrix<double,MC,  MR>& X,
  int a, int b, bool tryForHighAccuracy );

template void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape,
  DistMatrix<double,MC,  MR>& A,
  DistMatrix<double,MC,  MR>& B,
  DistMatrix<double,Star,VR>& w,
  DistMatrix<double,MC,  MR>& X,
  double a, double b, bool tryForHighAccuracy );

template void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape,
  DistMatrix<double,MC,  MR>& A,
  DistMatrix<double,MC,  MR>& B,
  DistMatrix<double,Star,VR>& w,
  bool tryForHighAccuracy );

template void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape,
  DistMatrix<double,MC,  MR>& A,
  DistMatrix<double,MC,  MR>& B,
  DistMatrix<double,Star,VR>& w,
  int a, int b, bool tryForHighAccuracy );

template void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape,
  DistMatrix<double,MC,  MR>& A,
  DistMatrix<double,MC,  MR>& B,
  DistMatrix<double,Star,VR>& w,
  double a, double b, bool tryForHighAccuracy );

#ifndef WITHOUT_COMPLEX
template void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape,
  DistMatrix<std::complex<double>,MC,  MR>& A,
  DistMatrix<std::complex<double>,MC,  MR>& B,
  DistMatrix<             double, Star,VR>& w,
  DistMatrix<std::complex<double>,MC,  MR>& X,
  bool tryForHighAccuracy );

template void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape,
  DistMatrix<std::complex<double>,MC,  MR>& A,
  DistMatrix<std::complex<double>,MC,  MR>& B,
  DistMatrix<             double, Star,VR>& w,
  DistMatrix<std::complex<double>,MC,  MR>& X,
  int a, int b, bool tryForHighAccuracy );

template void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape,
  DistMatrix<std::complex<double>,MC,  MR>& A,
  DistMatrix<std::complex<double>,MC,  MR>& B,
  DistMatrix<             double, Star,VR>& w,
  DistMatrix<std::complex<double>,MC,  MR>& X,
  double a, double b, bool tryForHighAccuracy );

template void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape,
  DistMatrix<std::complex<double>,MC,  MR>& A,
  DistMatrix<std::complex<double>,MC,  MR>& B,
  DistMatrix<             double, Star,VR>& w,
  bool tryForHighAccuracy );

template void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape,
  DistMatrix<std::complex<double>,MC,  MR>& A,
  DistMatrix<std::complex<double>,MC,  MR>& B,
  DistMatrix<             double, Star,VR>& w,
  int a, int b, bool tryForHighAccuracy );

template void
elemental::lapack::GeneralizedHermitianEig
( GenEigType genEigType, Shape shape,
  DistMatrix<std::complex<double>,MC,  MR>& A,
  DistMatrix<std::complex<double>,MC,  MR>& B,
  DistMatrix<             double, Star,VR>& w,
  double a, double b, bool tryForHighAccuracy );
#endif // WITHOUT_COMPLEX
#endif // WITHOUT_PMRRR