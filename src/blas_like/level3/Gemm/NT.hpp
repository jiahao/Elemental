/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/

namespace El {
namespace gemm {

// Normal Transpose Gemm that avoids communicating the matrix A
template<typename T>
inline void
SUMMA_NTA
( Orientation orientB,
  T alpha, const AbstractDistMatrix<T>& APre, const AbstractDistMatrix<T>& BPre,
  T beta,        AbstractDistMatrix<T>& CPre )
{
    DEBUG_ONLY(
        CallStackEntry cse("gemm::SUMMA_NTA");
        AssertSameGrids( APre, BPre, CPre );
        if( orientB == NORMAL )
            LogicError("B must be (Conjugate)Transposed");
        if( APre.Height() != CPre.Height() || BPre.Height() != CPre.Width() ||
            APre.Width() != BPre.Width() )
            LogicError
            ("Nonconformal matrices:\n",
             DimsString(APre,"A"),"\n",DimsString(BPre,"B"),"\n",
              DimsString(CPre,"C"));
    )
    const Int m = CPre.Height();
    const Int n = CPre.Width();
    const Int sumDim = APre.Width();
    const Int bsize = Blocksize();
    const Grid& g = APre.Grid();
    const bool conjugate = ( orientB == ADJOINT );

    auto APtr = ReadProxy<T,MC,MR>( &APre );      auto& A = *APtr;
    auto BPtr = ReadProxy<T,MC,MR>( &BPre );      auto& B = *BPtr;
    auto CPtr = ReadWriteProxy<T,MC,MR>( &CPre ); auto& C = *CPtr;

    // Temporary distributions
    DistMatrix<T,MR,STAR> B1Trans_MR_STAR(g);
    DistMatrix<T,MC,STAR> D1_MC_STAR(g);

    B1Trans_MR_STAR.AlignWith( A );
    D1_MC_STAR.AlignWith( A );

    Scale( beta, C );
    for( Int k=0; k<n; k+=bsize )
    {
        const Int nb = Min(bsize,n-k);
        auto B1 = B( IR(k,k+nb), IR(0,sumDim) );
        auto C1 = C( IR(0,m),    IR(k,k+nb)   );

        // C1[MC,*] := alpha A[MC,MR] (B1^[T/H])[MR,*]
        Transpose( B1, B1Trans_MR_STAR, conjugate );
        LocalGemm( NORMAL, NORMAL, alpha, A, B1Trans_MR_STAR, D1_MC_STAR );

        // C1[MC,MR] += scattered result of D1[MC,*] summed over grid rows
        AxpyContract( T(1), D1_MC_STAR, C1 );
    }
}

// Normal Transpose Gemm that avoids communicating the matrix B
template<typename T>
inline void
SUMMA_NTB
( Orientation orientB,
  T alpha, const AbstractDistMatrix<T>& APre, const AbstractDistMatrix<T>& BPre,
  T beta,        AbstractDistMatrix<T>& CPre )
{
    DEBUG_ONLY(
        CallStackEntry cse("gemm::SUMMA_NTB");
        AssertSameGrids( APre, BPre, CPre );
        if( orientB == NORMAL )
            LogicError("B must be (Conjugate)Transposed");
        if( APre.Height() != CPre.Height() ||
            BPre.Height() != CPre.Width() ||
            APre.Width() != BPre.Width() )
            LogicError
            ("Nonconformal matrices:\n",
             DimsString(APre,"A"),"\n",DimsString(BPre,"B"),"\n",
             DimsString(CPre,"C"));
    )
    const Int m = CPre.Height();
    const Int n = CPre.Width();
    const Int sumDim = APre.Width();
    const Int bsize = Blocksize();
    const Grid& g = APre.Grid();

    auto APtr = ReadProxy<T,MC,MR>( &APre );      auto& A = *APtr;
    auto BPtr = ReadProxy<T,MC,MR>( &BPre );      auto& B = *BPtr;
    auto CPtr = ReadWriteProxy<T,MC,MR>( &CPre ); auto& C = *CPtr;

    // Temporary distributions
    DistMatrix<T,MR,STAR> A1Trans_MR_STAR(g);
    DistMatrix<T,STAR,MC> D1_STAR_MC(g);
    DistMatrix<T,MR,MC> D1_MR_MC(g);

    A1Trans_MR_STAR.AlignWith( B );
    D1_STAR_MC.AlignWith( B );

    Scale( beta, C );
    for( Int k=0; k<m; k+=bsize )
    {
        const Int nb = Min(bsize,m-k);
        auto A1 = A( IR(k,k+nb), IR(0,sumDim) );
        auto C1 = C( IR(k,k+nb), IR(0,n)      );

        // D1[*,MC] := alpha A1[*,MR] (B[MC,MR])^T
        //           = alpha (A1^T)[MR,*] (B^T)[MR,MC]
        Transpose( A1, A1Trans_MR_STAR );
        LocalGemm( TRANSPOSE, orientB, alpha, A1Trans_MR_STAR, B, D1_STAR_MC );

        // C1[MC,MR] += scattered & transposed D1[*,MC] summed over grid rows
        Contract( D1_STAR_MC, D1_MR_MC );
        Axpy( T(1), D1_MR_MC, C1 );
    }
}

// Normal Transpose Gemm that avoids communicating the matrix C
template<typename T>
inline void
SUMMA_NTC
( Orientation orientB,
  T alpha, const AbstractDistMatrix<T>& APre, const AbstractDistMatrix<T>& BPre,
  T beta,        AbstractDistMatrix<T>& CPre )
{
    DEBUG_ONLY(
        CallStackEntry cse("gemm::SUMMA_NTC");
        AssertSameGrids( APre, BPre, CPre );
        if( orientB == NORMAL )
            LogicError("B must be (Conjugate)Transposed");
        if( APre.Height() != CPre.Height() ||
            BPre.Height() != CPre.Width() ||
            APre.Width() != BPre.Width() )
            LogicError
            ("Nonconformal matrices:\n",
             DimsString(APre,"A"),"\n",DimsString(BPre,"B"),"\n",
             DimsString(CPre,"C"));
    )
    const Int m = CPre.Height();
    const Int n = CPre.Width();
    const Int sumDim = APre.Width();
    const Int bsize = Blocksize();
    const Grid& g = APre.Grid();
    const bool conjugate = ( orientB == ADJOINT );

    auto APtr = ReadProxy<T,MC,MR>( &APre );      auto& A = *APtr;
    auto BPtr = ReadProxy<T,MC,MR>( &BPre );      auto& B = *BPtr;
    auto CPtr = ReadWriteProxy<T,MC,MR>( &CPre ); auto& C = *CPtr;

    // Temporary distributions
    DistMatrix<T,MC,STAR> A1_MC_STAR(g);
    DistMatrix<T,VR,STAR> B1_VR_STAR(g);
    DistMatrix<T,STAR,MR> B1Trans_STAR_MR(g);

    A1_MC_STAR.AlignWith( C );
    B1_VR_STAR.AlignWith( C );
    B1Trans_STAR_MR.AlignWith( C );

    Scale( beta, C );
    for( Int k=0; k<sumDim; k+=bsize )
    {
        const Int nb = Min(bsize,sumDim-k);
        auto A1 = A( IR(0,m), IR(k,k+nb) );
        auto B1 = B( IR(0,n), IR(k,k+nb) );

        A1_MC_STAR = A1;
        B1_VR_STAR = B1;
        Transpose( B1_VR_STAR, B1Trans_STAR_MR, conjugate );

        // C[MC,MR] += alpha A1[MC,*] (B1[MR,*])^T
        LocalGemm
        ( NORMAL, NORMAL, alpha, A1_MC_STAR, B1Trans_STAR_MR, T(1), C );
    }
}

template<typename T>
inline void
SUMMA_NT
( Orientation orientB,
  T alpha, const AbstractDistMatrix<T>& A, const AbstractDistMatrix<T>& B,
  T beta,        AbstractDistMatrix<T>& C, GemmAlgorithm alg=GEMM_DEFAULT )
{
    DEBUG_ONLY(CallStackEntry cse("gemm::SUMMA_NT"))
    const Int m = C.Height();
    const Int n = C.Width();
    const Int k = A.Width();
    const double weightTowardsC = 2.;

    switch( alg )
    {
    case GEMM_DEFAULT:
        if( m <= n && weightTowardsC*m <= k )
            SUMMA_NTB( orientB, alpha, A, B, beta, C );
        else if( n <= m && weightTowardsC*n <= k )
            SUMMA_NTA( orientB, alpha, A, B, beta, C );
        else
            SUMMA_NTC( orientB, alpha, A, B, beta, C );
        break;
    case GEMM_SUMMA_A: SUMMA_NTA( orientB, alpha, A, B, beta, C ); break;
    case GEMM_SUMMA_B: SUMMA_NTB( orientB, alpha, A, B, beta, C ); break;
    case GEMM_SUMMA_C: SUMMA_NTC( orientB, alpha, A, B, beta, C ); break;
    default: LogicError("Unsupported Gemm option");
    }
}

} // namespace gemm
} // namespace El
