/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#pragma once
#ifndef EL_IMPORTS_FLAME_HPP
#define EL_IMPORTS_FLAME_HPP

#ifdef EL_HAVE_FLA_BSVD
namespace El {

void FlaBidiagSVD
( BlasInt k, BlasInt mU, BlasInt mV, double* d, double* e, 
  double* U, BlasInt ldu, double* V, BlasInt ldv, 
  BlasInt numAccum=32, BlasInt maxNumIts=30, BlasInt bAlg=512 );

void FlaBidiagSVD
( BlasInt k, BlasInt mU, BlasInt mV, double* d, double* e, 
  Complex<double>* U, BlasInt ldu, Complex<double>* V, BlasInt ldv, 
  BlasInt numAccum=32, BlasInt maxNumIts=30, BlasInt bAlg=512 );

} // namespace El
#endif // ifdef EL_HAVE_FLA_BSVD

#endif // ifndef EL_IMPORTS_FLAME_HPP
