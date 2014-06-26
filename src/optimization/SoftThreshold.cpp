/*
   Copyright (c) 2009-2014, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El-lite.hpp"

namespace El {

template<typename F>
F SoftThreshold( F alpha, Base<F> tau )
{
    DEBUG_ONLY(
        CallStackEntry cse("SoftThreshold");
        if( tau < 0 )
            LogicError("Negative threshold does not make sense");
    )
    const Base<F> scale = Abs(alpha);
    return ( scale <= tau ? F(0) : alpha-(alpha/scale)*tau );
}

template<typename F>
void SoftThreshold( Matrix<F>& A, Base<F> tau, bool relative )
{
    DEBUG_ONLY(CallStackEntry cse("SoftThreshold"))
    if( relative )
        tau *= MaxNorm(A);
    EntrywiseMap( A, [=]( F alpha ) { return SoftThreshold(alpha,tau); } );
}

template<typename F>
void SoftThreshold( AbstractDistMatrix<F>& A, Base<F> tau, bool relative )
{
    DEBUG_ONLY(CallStackEntry cse("SoftThreshold"))
    if( relative )
        tau *= MaxNorm(A);
    EntrywiseMap( A, [=]( F alpha ) { return SoftThreshold(alpha,tau); } );
}

#define PROTO(F) \
  template F SoftThreshold( F alpha, Base<F> tau ); \
  template void SoftThreshold \
  ( Matrix<F>& A, Base<F> tau, bool relative ); \
  template void SoftThreshold \
  ( AbstractDistMatrix<F>& A, Base<F> tau, bool relative );

PROTO(float)
PROTO(double)
PROTO(Complex<float>)
PROTO(Complex<double>)

} // namespace El
