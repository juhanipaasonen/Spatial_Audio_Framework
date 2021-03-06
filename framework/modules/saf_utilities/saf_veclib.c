/*
 * Copyright 2016-2018 Leo McCormack
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @file saf_veclib.c
 * @brief Contains wrappers for optimised linear algebra routines, utilising
 *        CBLAS and LAPACK
 *
 * ## Dependencies
 *   A performance library comprising CBLAS and LAPACK routines is required by
 *   the module and, thus, also by the SAF framework as a whole. Add one of the
 *   following FLAGS to your project's preprocessor definitions list, in order
 *   to enable one of these suitable performance libraries, which must also be
 *   linked correctly to your project.
 *   - SAF_USE_INTEL_MKL:
 *       to enable Intel's Math Kernal Library with Fortran LAPACK interface
 *   - SAF_USE_ATLAS:
 *       to enable ATLAS BLAS routines and ATLAS's CLAPACK interface
 *   - SAF_USE_OPENBLAS_WITH_LAPACKE:
 *       to enable OpenBLAS with LAPACKE interface
 *
 * @see More information can be found here:
 *      https://github.com/leomccormack/Spatial_Audio_Framework
 * @note MacOSX users only: saf_utilities will employ Apple's Accelerate library
 *       by default, if none of the above FLAGS are defined.
 *
 * @author Leo McCormack
 * @date 11.07.2016
 */

#include "saf_utilities.h"
#include <float.h>

/* just to remove compiler warnings: */
#if defined(__APPLE__) && !defined(SAF_USE_INTEL_MKL)
  typedef __CLPK_integer       veclib_int;
  typedef __CLPK_real          veclib_float;
  typedef __CLPK_complex       veclib_float_complex;
  typedef __CLPK_doublecomplex veclib_double_complex;
#elif defined(SAF_USE_INTEL_MKL)
  typedef int                  veclib_int;
  typedef float                veclib_float;
  typedef MKL_Complex8         veclib_float_complex;
  typedef MKL_Complex16        veclib_double_complex;
#else
  typedef int                  veclib_int;
  typedef float                veclib_float;
  typedef float_complex        veclib_float_complex;
  typedef double_complex       veclib_double_complex;
#endif


/* ========================================================================== */
/*                     Find Index of Min-Abs-Value (?iminv)                   */
/* ========================================================================== */

void utility_siminv
(
    const float* a,
    const int len,
    int* index
)
{
#if defined(__ACCELERATE__)
    float minVal;
    vDSP_Length ind_tmp;
    vDSP_minvi(a, 1, &minVal, &ind_tmp, len);
    *index = (int)ind_tmp; 
#elif defined(INTEL_MKL_VERSION)
    *index = (int)cblas_isamin(len, a, 1);
#else
    int i;
    *index = 0;
    float minVal=FLT_MAX;
    for(i=0; i<len; i++){
        if(fabsf(a[i])<minVal){
            minVal = fabsf(a[i]);
            *index = i;
        } 
    }
#endif
}

void utility_ciminv
(
    const float_complex* a,
    const int len,
    int* index
)
{
#if defined(__ACCELERATE__)
    int i;
    float* abs_a;
    float minVal;
    abs_a = malloc1d(len*sizeof(float));
    for(i=0; i<len; i++)
        abs_a[i] = cabsf(a[i]);
    vDSP_Length ind_tmp;
    vDSP_minvi(abs_a, 1, &minVal, &ind_tmp, len);
    *index = (int)ind_tmp;
    free(abs_a);
#elif defined(INTEL_MKL_VERSION)
    *index = (int)cblas_icamin(len, a, 1);
#else
    int i;
    *index = 0;
    float minVal=FLT_MAX;
    for(i=0; i<len; i++){
        if(cabsf(a[i])<minVal){
            minVal = cabsf(a[i]);
            *index = i;
        } 
    }
#endif
}


/* ========================================================================== */
/*                     Find Index of Max-Abs-Value (?imaxv)                   */
/* ========================================================================== */

void utility_simaxv
(
    const float* a,
    const int len,
    int* index
)
{
    *index = (int)cblas_isamax(len, a, 1);
}

void utility_cimaxv
(
    const float_complex* a,
    const int len,
    int* index
)
{
    *index = (int)cblas_icamax(len, a, 1);
}


/* ========================================================================== */
/*                              Vector-Abs (?vabs)                            */
/* ========================================================================== */

void utility_svabs
(
    const float* a,
    const int len,
    float* c
)
{
#ifdef INTEL_MKL_VERSION
    vsAbs(len, a, c);
#else
    int i;
    for(i=0; i<len; i++)
        c[i] = fabsf(a[i]);
#endif
}

void utility_cvabs
(
    const float_complex* a,
    const int len,
    float* c
)
{
#ifdef INTEL_MKL_VERSION
    vcAbs(len, (MKL_Complex8*)a, c);
#else
    int i;
    for(i=0; i<len; i++)
        c[i] = cabsf(a[i]);
#endif
}


/* ========================================================================== */
/*                        Vector-Vector Copy (?vvcopy)                        */
/* ========================================================================== */

void utility_svvcopy
(
    const float* a,
    const int len,
    float* c
)
{
    cblas_scopy(len, a, 1, c, 1);
}

void utility_cvvcopy
(
    const float_complex* a,
    const int len,
    float_complex* c
)
{
    cblas_ccopy(len, a, 1, c, 1);
}


/* ========================================================================== */
/*                       Vector-Vector Addition (?vvadd)                      */
/* ========================================================================== */

void utility_svvadd
(
    float* a,
    const float* b,
    const int len,
    float* c
)
{
#ifdef __ACCELERATE__
	if (c == NULL) {
		float* tmp;
		tmp = malloc1d(len * sizeof(float));
		vDSP_vadd(a, 1, b, 1, tmp, 1, len);
		utility_svvcopy(tmp, len, a);
		free(tmp);
	}
	else
		vDSP_vadd(a, 1, b, 1, c, 1, len);
#elif defined(INTEL_MKL_VERSION)
	if (c == NULL) {
		float* tmp;
		tmp = malloc1d(len * sizeof(float));
		vsAdd(len, a, b, tmp);
		utility_svvcopy(tmp, len, a);
		free(tmp);
	}
	else
		vsAdd(len, a, b, c);
#else
	int i;
	for (i = 0; i < len; i++)
		c[i] = a[i] + b[i];
#endif
}

void utility_cvvadd
(
    float_complex* a,
    const float_complex* b,
    const int len,
    float_complex* c
)
{
#ifdef INTEL_MKL_VERSION
	if (c == NULL) {
		float_complex* tmp;
		tmp = malloc1d(len * sizeof(float_complex));
		vcAdd(len, (MKL_Complex8*)a, (MKL_Complex8*)b, (MKL_Complex8*)tmp);
		utility_cvvcopy(tmp, len, a);
		free(tmp);
	}
	else
		vcAdd(len, (MKL_Complex8*)a, (MKL_Complex8*)b, (MKL_Complex8*)c);
#else
	int i;
	for (i = 0; i < len; i++)
		c[i] = ccaddf(a[i], b[i]);
#endif
}


/* ========================================================================== */
/*                     Vector-Vector Subtraction (?vvsub)                     */
/* ========================================================================== */

void utility_svvsub
(
    float* a,
    const float* b,
    const int len,
    float* c
)
{
#ifdef __ACCELERATE__
	if (c == NULL) {
		float* tmp;
		tmp = malloc1d(len * sizeof(float));
		vDSP_vsub(a, 1, b, 1, tmp, 1, len);
		utility_svvcopy(tmp, len, a);
		free(tmp);
	}
	else
		vDSP_vsub(a, 1, b, 1, c, 1, len);
#elif defined(INTEL_MKL_VERSION)
	if (c == NULL) {
		float* tmp;
		tmp = malloc1d(len * sizeof(float));
		vsSub(len, a, b, tmp);
		utility_svvcopy(tmp, len, a);
		free(tmp);
	}
	else
		vsSub(len, a, b, c);
#else
	int i;
	for (i = 0; i < len; i++)
		c[i] = a[i] + b[i];
#endif
}

void utility_cvvsub
(
    float_complex* a,
    const float_complex* b,
    const int len,
    float_complex* c
)
{
#ifdef INTEL_MKL_VERSION
	if (c == NULL) {
		float_complex* tmp;
		tmp = malloc1d(len * sizeof(float_complex));
		vcSub(len, (MKL_Complex8*)a, (MKL_Complex8*)b, (MKL_Complex8*)tmp);
		utility_cvvcopy(tmp, len, a);
		free(tmp);
	}
	else
		vcSub(len, (MKL_Complex8*)a, (MKL_Complex8*)b, (MKL_Complex8*)c);
#else
	int i;
    if (c == NULL) {
        for (i = 0; i < len; i++)
            a[i] = ccaddf(a[i], b[i]);
    }
    else{
        for (i = 0; i < len; i++)
            c[i] = ccaddf(a[i], b[i]);
    }
#endif
}


/* ========================================================================== */
/*                    Vector-Vector Multiplication (?vvmul)                   */
/* ========================================================================== */

void utility_svvmul
(
    float* a,
    const float* b,
    const int len,
    float* c
)
{
#ifdef __ACCELERATE__
    if(c==NULL){
        float* tmp;
        tmp=malloc1d(len*sizeof(float));
        vDSP_vmul(a, 1, b, 1, tmp, 1, len);
        utility_svvcopy(tmp, len, a);
        free(tmp);
    }
    else
        vDSP_vmul(a, 1, b, 1, c, 1, len);
#elif defined(INTEL_MKL_VERSION)
    if (c == NULL) {
        float* tmp;
        tmp = malloc1d(len * sizeof(float));
        vsMul(len, a, b, tmp);
        utility_svvcopy(tmp, len, a);
        free(tmp);
    }
    else
        vsMul(len, a, b, c);
#else
    int i;
    for (i = 0; i < len; i++)
        c[i] = a[i] * b[i];
#endif
}

void utility_cvvmul
(
    float_complex* a,
    const float_complex* b,
    const int len,
    float_complex* c
)
{
#ifdef INTEL_MKL_VERSION
    if (c == NULL) {
        float_complex* tmp;
        tmp = malloc1d(len * sizeof(float_complex));
        vcMul(len, (MKL_Complex8*)a, (MKL_Complex8*)b, (MKL_Complex8*)tmp);
        utility_cvvcopy(tmp, len, a);
        free(tmp);
    }
    else
        vcMul(len, (MKL_Complex8*)a, (MKL_Complex8*)b, (MKL_Complex8*)c);
#else
	int i;
    if (c == NULL) {
        for (i = 0; i < len; i++)
            a[i] = ccmulf(a[i], b[i]);
    }
    else{
        for (i = 0; i < len; i++)
            c[i] = ccmulf(a[i], b[i]);
    }
#endif
}


/* ========================================================================== */
/*                     Vector-Vector Dot Product (?vvdot)                     */
/* ========================================================================== */

void utility_svvdot
(
    const float* a,
    const float* b,
    const int len,
    float* c
)
{
    c[0] = cblas_sdot (len, a, 1, b, 1);
}

void utility_cvvdot
(
    const float_complex* a,
    const float_complex* b,
    const int len,
    CONJ_FLAG flag,
    float_complex* c
)
{
    switch(flag){
        default:
        case NO_CONJ:
            cblas_cdotu_sub(len, a, 1, b, 1, c);
            break;
        case CONJ:
            cblas_cdotc_sub(len, a, 1, b, 1, c);
            break;
    }
}


/* ========================================================================== */
/*                       Vector-Scalar Product (?vsmul)                       */
/* ========================================================================== */

void utility_svsmul
(
    float* a,
    const float* s,
    const int len,
    float* c
)
{
#ifdef __ACCELERATE__
    if(c==NULL)
        cblas_sscal(len, s[0], a, 1);
    else
        vDSP_vsmul(a, 1, s, c, 1, len);
#else
    if (c == NULL)
        cblas_sscal(len, s[0], a, 1);
    else {
        utility_svvcopy(a, len, c); 
        cblas_sscal(len, s[0], c, 1);
    }
#endif
}

void utility_cvsmul
(
    float_complex* a,
    const float_complex* s,
    const int len,
    float_complex* c
)
{
    if (c == NULL)
        cblas_cscal(len, s, a, 1);
    else {
        int i;
        for (i = 0; i<len; i++)
            c[i] = ccmulf(a[i], s[0]);
    }
}


/* ========================================================================== */
/*                       Vector-Scalar Division (?vsdiv)                      */
/* ========================================================================== */

void utility_svsdiv
(
    float* a,
    const float* s,
    const int len,
    float* c
)
{
    if(s[0] == 0.0f){
        memset(c, 0, len*sizeof(float));
        return;
    }
#ifdef __ACCELERATE__
    vDSP_vsdiv(a, 1, s, c, 1, len);
#else
    int i;
    for(i=0; i<len; i++)
        c[i] = a[i] / s[0];
#endif
}


/* ========================================================================== */
/*                       Vector-Scalar Addition (?vsadd)                      */
/* ========================================================================== */

void utility_svsadd
(
    float* a,
    const float* s,
    const int len,
    float* c
)
{
#ifdef __ACCELERATE__
    vDSP_vsadd(a, 1, s, c, 1, len);
#else
    int i;
    for(i=0; i<len; i++)
        c[i] = a[i] + s[0];
#endif
}


/* ========================================================================== */
/*                     Vector-Scalar Subtraction (?vssub)                     */
/* ========================================================================== */

void utility_svssub
(
    float* a,
    const float* s,
    const int len,
    float* c
)
{
    int i;
    for(i=0; i<len; i++)
        c[i] = a[i] - s[0];
}


/* ========================================================================== */
/*                     Singular-Value Decomposition (?svd)                    */
/* ========================================================================== */

void utility_ssvd
(
    const float* A,
    const int dim1,
    const int dim2,
    float* U,
    float* S,
    float* V,
    float* sing
)
{
    int i, j, m, n, lda, ldu, ldvt, info;
    m = dim1; n = dim2; lda = dim1; ldu = dim1; ldvt = dim2;
    float* a, *s, *u, *vt;
#ifdef VECLIB_USE_LAPACK_FORTRAN_INTERFACE
    int lwork;
    float wkopt;
    float *work;
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    float* superb;
#endif
    
    a = malloc1d(lda*n*sizeof(float));
    s = malloc1d(MIN(n,m)*sizeof(float));
    u = malloc1d(ldu*m*sizeof(float));
    vt = malloc1d(ldvt*n*sizeof(float));
    
    /* store in column major order */
    for(i=0; i<dim1; i++)
        for(j=0; j<dim2; j++)
            a[j*dim1+i] = A[i*dim2 +j];
    
    /* perform the singular value decomposition */
#ifdef VECLIB_USE_CLAPACK_INTERFACE
    /* no such implementation in altas-clapack */
    assert(0);
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    superb = malloc((MIN(m,n)-1)*sizeof(float));
    info = LAPACKE_sgesvd(CblasColMajor, 'A', 'A', m, n, a, lda, s, u, ldu, vt, ldvt, superb);
    free(superb);
#elif defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    lwork = -1;
    sgesvd_( "A", "A", &m, &n, a, &lda, s, u, &ldu, vt, &ldvt, &wkopt, &lwork, &info );
    lwork = (int)wkopt;
    work = malloc1d( lwork*sizeof(float) );
    sgesvd_( "A", "A", &m, &n, a, &lda, s, u, &ldu, vt, &ldvt, work, &lwork, &info );
    free(work);
#endif
    
    /* svd failed to converge */
    if( info != 0 ) {
        if (S != NULL)
            memset(S, 0, dim1*dim2*sizeof(float));
        if (U != NULL)
            memset(U, 0, dim1*dim1*sizeof(float));
        if (V != NULL)
            memset(U, 0, dim2*dim2*sizeof(float));
        if (sing != NULL)
            memset(sing, 0, MIN(dim1, dim2)*sizeof(float));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_SVD);
#endif
    }
    /* svd successful */
    else {
        if (S != NULL){
            memset(S, 0, dim1*dim2*sizeof(float));
            /* singular values on the diagonal MIN(dim1, dim2). The remaining elements are 0.  */
            for(i=0; i<MIN(dim1, dim2); i++)
                S[i*dim2+i] = s[i];
        }
        
        /*return as row-major*/
        if (U != NULL)
            for(i=0; i<dim1; i++)
                for(j=0; j<dim1; j++)
                    U[i*dim1+j] = u[j*dim1+i];
        
        /* lapack returns VT, i.e. row-major V already */
        if (V != NULL)
            for(i=0; i<dim2; i++)
                for(j=0; j<dim2; j++)
                    V[i*dim2+j] = vt[i*dim2+j];
        
        if (sing != NULL)
            for(i=0; i<MIN(dim1, dim2); i++)
                sing[i] = s[i];
    }
    
    free(a);
    free(s);
    free(u);
    free(vt);
}

void utility_csvd
(
    const float_complex* A,
    const int dim1,
    const int dim2,
    float_complex* U,
    float_complex* S,
    float_complex* V,
    float* sing
)
{
    int i, j, m, n, lda, ldu, ldvt, info;
    m = dim1; n = dim2; lda = dim1; ldu = dim1; ldvt = dim2;
    float_complex* a, *u, *vt, *work;
    float* s;
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    int lwork;
    float_complex wkopt;
    float* rwork;
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    float* superb;
#endif
    
    a = malloc1d(lda*n*sizeof(float_complex));
    s = malloc1d(MIN(n,m)*sizeof(float));
    u = malloc1d(ldu*m*sizeof(float_complex));
    vt = malloc1d(ldvt*n*sizeof(float_complex));
    
    /* store in column major order */
    for(i=0; i<dim1; i++)
        for(j=0; j<dim2; j++)
            a[j*dim1+i] = A[i*dim2 +j];
    
    /* perform the singular value decomposition */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    rwork = malloc1d(m*MAX(1, 5*MIN(n,m))*sizeof(float));
    lwork = -1;
    cgesvd_( "A", "A", (veclib_int*)&m, (veclib_int*)&n, (veclib_float_complex*)a, (veclib_int*)&lda, s, (veclib_float_complex*)u, (veclib_int*)&ldu,
            (veclib_float_complex*)vt, &ldvt, (veclib_float_complex*)&wkopt, &lwork, rwork, (veclib_int*)&info );
    lwork = (int)(crealf(wkopt)+0.01f);
    work = malloc1d( lwork*sizeof(float_complex) );
    cgesvd_( "A", "A", &m, &n, (veclib_float_complex*)a, &lda, s, (veclib_float_complex*)u, &ldu, (veclib_float_complex*)vt, &ldvt,
            (veclib_float_complex*)work, &lwork, rwork, &info);
    free(work);
    free(rwork);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    assert(0); /* no such implementation in clapack */
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    superb = malloc((MIN(m,n)-1)*sizeof(float));
    info = LAPACKE_cgesvd(CblasColMajor, 'A', 'A', m, n, (veclib_float_complex*)a, lda, s, (veclib_float_complex*)u, ldu, vt, ldvt, superb);
    free(superb);
#endif

    /* svd failed to converge */
    if( info != 0 ) {
        if (S != NULL)
            memset(S, 0, dim1*dim2*sizeof(float_complex));
        if (U != NULL)
            memset(U, 0, dim1*dim1*sizeof(float_complex));
        if (V != NULL)
            memset(U, 0, dim2*dim2*sizeof(float_complex));
        if (sing != NULL)
            memset(sing, 0, MIN(dim1, dim2)*sizeof(float_complex));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_SVD);
#endif
    }
    /* svd successful */
    else {
        if (S != NULL){
            memset(S, 0, dim1*dim2*sizeof(float_complex));
            /* singular values on the diagonal MIN(dim1, dim2). The remaining elements are 0.  */
            for(i=0; i<MIN(dim1, dim2); i++)
                S[i*dim2+i] = cmplxf(s[i], 0.0f);
        }
        /*return as row-major*/
        if (U != NULL)
            for(i=0; i<dim1; i++)
                for(j=0; j<dim1; j++)
                    U[i*dim1+j] = u[j*dim1+i];
        
        /* lapack returns VT, i.e. row-major V already */
        if (V != NULL)
            for(i=0; i<dim2; i++)
                for(j=0; j<dim2; j++)
                    V[i*dim2+j] = conjf(vt[i*dim2+j]); /* v^H */
        
        if (sing != NULL)
            for(i=0; i<MIN(dim1, dim2); i++)
                sing[i] = s[i];
    }
    
    free(a);
    free(s);
    free(u);
    free(vt);
}


/* ========================================================================== */
/*                 Symmetric Eigenvalue Decomposition (?seig)                 */
/* ========================================================================== */

void utility_sseig
(
    const float* A,
    const int dim,
    int sortDecFLAG,
    float* V,
    float* D,
    float* eig
)
{
    int i, j, n, lda, info, lwork;
    float wkopt;
    float* work, *w, *a;

    n = dim;
    lda = dim;
    w = malloc1d(dim*sizeof(float));
    a = malloc1d(dim*dim*sizeof(float));
    
    /* store in column major order (i.e. transpose) */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[i*dim+j] = A[j*dim+i];
    
    /* solve the eigenproblem */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    lwork = -1;
    ssyev_( "Vectors", "Upper", &n, a, &lda, w, &wkopt, &lwork, &info );
    lwork = (int)wkopt;
    work = (float*)malloc1d( lwork*sizeof(float) );
    ssyev_( "Vectors", "Upper", &n, a, &lda, w, work, &lwork, &info );
    free(work);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    assert(0); /* no such implementation in clapack */
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_ssyev(CblasColMajor, 'V', 'U', n, a, lda, w );
#endif
    
    /* output */
    if(D!=NULL)
        memset(D, 0, dim*dim*sizeof(float));
    if( info != 0 ) {
        /* failed to converge and find the eigenvalues */
        if(V!=NULL)
            memset(V, 0, dim*dim*sizeof(float));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_EVG);
#endif
    }
    else{
        if(sortDecFLAG){
            for(i=0; i<dim; i++){
                if(V!=NULL)
                    for(j=0; j<dim; j++)
                        V[i*dim+j] = a[(dim-j-1)*dim+i]; /* transpose, back to row-major and reverse order */
                if(D!=NULL)
                    D[i*dim+i] = w[dim-i-1]; /* store along the diagonal, reversing the order */
                if(eig!=NULL)
                    eig[i] = w[dim-i-1];
            }
        }
        else{
            for(i=0; i<dim; i++){
                if(V!=NULL)
                    for(j=0; j<dim; j++)
                        V[i*dim+j] = a[j*dim+i]; /* transpose, back to row-major */
                if(D!=NULL)
                    D[i*dim+i] = w[i]; /* store along the diagonal */
                if(eig!=NULL)
                    eig[i] = w[i];
            }
        }
    }
    
    free(w);
    free(a);
}

void utility_cseig
(
    const float_complex* A,
    const int dim,
    int sortDecFLAG,
    float_complex* V,
    float_complex* D,
    float* eig
)
{
    int i, j, n, lda, info;
    float_complex wkopt;
    float *w;
    float_complex* a;
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    int lwork;
    float *rwork;
    float_complex *work;
#endif

    n = dim;
    lda = dim;
    w = malloc1d(dim*sizeof(float));
    a = malloc1d(dim*dim*sizeof(float_complex));
    
    /* store in column major order (i.e. transpose) */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[i*dim+j] = A[j*dim+i];
    
    /* solve the eigenproblem */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    rwork = malloc1d((3*n-2)*sizeof(float));
    lwork = -1;
    cheev_( "Vectors", "Upper", &n, (veclib_float_complex*)a, &lda, w, (veclib_float_complex*)&wkopt, &lwork, rwork, &info );
    lwork = (int)crealf(wkopt);
    work = (float_complex*)malloc1d( lwork*sizeof(float_complex) );
    cheev_( "Vectors", "Upper", &n, (veclib_float_complex*)a, &lda, w, (veclib_float_complex*)work, &lwork, rwork, &info );
    free(work);
    free(rwork);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    assert(0); /* no such implementation in clapack */
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_cheev(CblasColMajor, 'V', 'U', n, (veclib_float_complex*)a, lda, w);
#endif
    
    /* output */
    if(D!=NULL)
        memset(D, 0, dim*dim*sizeof(float_complex));
    if( info != 0 ) {
        /* failed to converge and find the eigenvalues */
        if(V!=NULL)
            memset(V, 0, dim*dim*sizeof(float_complex));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_EVG);
#endif
    }
    
    /* transpose, back to row-major and reverse order */
    else{
        if(sortDecFLAG){
            for(i=0; i<dim; i++) {
                if(V!=NULL)
                    for(j=0; j<dim; j++)
                        V[i*dim+j] = a[(dim-j-1)*dim+i];
                if(D!=NULL)
                    D[i*dim+i] = cmplxf(w[dim-i-1], 0.0f); /* store along the diagonal, reversing the order */
                if(eig!=NULL)
                    eig[i] = w[dim-i-1];
            }
        }
        else {
            for(i=0; i<dim; i++){
                if(V!=NULL)
                    for(j=0; j<dim; j++)
                        V[i*dim+j] = a[j*dim+i];
                if(D!=NULL)
                    D[i*dim+i] = cmplxf(w[i], 0.0f); /* store along the diagonal */
                if(eig!=NULL)
                    eig[i] = w[i];
            }
        }
    }
    
    free(w);
    free(a);
}


/* ========================================================================== */
/*                     Eigenvalues of Matrix Pair (?eigmp)                    */
/* ========================================================================== */

void utility_ceigmp
(
    const float_complex* A,
    const float_complex* B,
    const int dim,
    float_complex* VL,
    float_complex* VR,
    float_complex* D
)
{
    int i, j;
    int n, lda, ldb, ldvl, ldvr, info;
    float_complex* a, *b, *vl, *vr, *alpha, *beta;
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    int lwork;
    float* rwork;
    float_complex* work;
#endif
    
    n = lda = ldb = ldvl = ldvr = dim;
    a = malloc1d(dim*dim*sizeof(float_complex));
    b = malloc1d(dim*dim*sizeof(float_complex));
    vl = malloc1d(dim*dim*sizeof(float_complex));
    vr = malloc1d(dim*dim*sizeof(float_complex));
    alpha = malloc1d(dim*sizeof(float_complex));
    beta = malloc1d(dim*sizeof(float_complex));
    
    /* store in column major order */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[j*dim+i] = A[i*dim+j];
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            b[j*dim+i] = B[i*dim+j];
    
    /* solve eigen problem */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    lwork = 4*n; /* 2x more than required, but is faster */
    work = malloc1d(lwork*sizeof(float_complex));
    rwork = malloc1d(4*lwork*sizeof(float)); /* 2x more than required, but is faster */
    cggev_("V", "V", &n, (veclib_float_complex*)a, &lda, (veclib_float_complex*)b, &ldb, (veclib_float_complex*)alpha, (veclib_float_complex*)beta,
           (veclib_float_complex*)vl, &ldvl, (veclib_float_complex*)vr, &ldvr, (veclib_float_complex*)work, &lwork, rwork, &info);
    free(work);
    free(rwork);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    assert(0); /* no such implementation in clapack */
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_cggev(CblasColMajor, 'V', 'V', n, (veclib_float_complex*)a, lda, (veclib_float_complex*)b, ldb, (veclib_float_complex*)alpha,
                         (veclib_float_complex*)beta, (veclib_float_complex*)vl, ldvl, (veclib_float_complex*)vr, ldvr);
#endif
    
    if(D!=NULL)
        memset(D, 0, dim*dim*sizeof(float_complex));
    
    /* failed to converge and find the eigenvalues */
    if( info != 0 ) {
        if(VL!=NULL)
            memset(VL, 0, dim*dim*sizeof(float_complex));
        if(VR!=NULL)
            memset(VR, 0, dim*dim*sizeof(float_complex));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_EVG);
#endif
    }
    /* transpose, back to row-major */
    else{
        if(D!=NULL)
            for(i=0; i<dim; i++)
                D[i*dim+i] = ccdivf(alpha[i],beta[i]);
        
        if(VL!=NULL)
            for(i=0; i<dim; i++)
                for(j=0; j<dim; j++)
                    VL[i*dim+j] = vl[j*dim+i];
        if(VR!=NULL)
            for(i=0; i<dim; i++)
                for(j=0; j<dim; j++)
                    VR[i*dim+j] = vr[j*dim+i];
    }
    
    free(a);
    free(b);
    free(vl);
    free(vr);
    free(alpha);
    free(beta);
}

void utility_zeigmp
(
    const double_complex* A,
    const double_complex* B,
    const int dim,
    double_complex* VL,
    double_complex* VR,
    double_complex* D
)
{
    int i, j;
    int n, lda, ldb, ldvl, ldvr, info;
    double_complex* a, *b, *vl, *vr, *alpha, *beta;
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    int lwork;
    double* rwork;
    double_complex* work;
#endif
    
    n = lda = ldb = ldvl = ldvr = dim;
    a = malloc1d(dim*dim*sizeof(double_complex));
    b = malloc1d(dim*dim*sizeof(double_complex));
    vl = malloc1d(dim*dim*sizeof(double_complex));
    vr = malloc1d(dim*dim*sizeof(double_complex));
    alpha = malloc1d(dim*sizeof(double_complex));
    beta = malloc1d(dim*sizeof(double_complex));
    
    /* store in column major order */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[j*dim+i] = A[i*dim+j];
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            b[j*dim+i] = B[i*dim+j]; /* store in column major order */
    
    /* solve eigen problem */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    lwork = 4*n; /* 2x more than required, but is faster */
    work = malloc1d(lwork*sizeof(double_complex));
    rwork = malloc1d(4*lwork*sizeof(double)); /* 2x more than required, but is faster */
    zggev_("V", "V", &n, (veclib_double_complex*)a, &lda, (veclib_double_complex*)b, &ldb, (veclib_double_complex*)alpha, (veclib_double_complex*)beta,
           (veclib_double_complex*)vl, &ldvl, (veclib_double_complex*)vr, &ldvr, (veclib_double_complex*)work, &lwork, rwork, &info);
    free(work);
    free(rwork);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    assert(0); /* no such implementation in clapack */
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_zggev(CblasColMajor, 'V', 'V', n, (veclib_double_complex*)a, lda, (veclib_double_complex*)b, ldb, (veclib_double_complex*)alpha,
                         (veclib_double_complex*)beta, (veclib_double_complex*)vl, ldvl, (veclib_double_complex*)vr, ldvr);
#endif
    
    if(D!=NULL)
        memset(D, 0, dim*dim*sizeof(double_complex));
    
    /* failed to converge and find the eigenvalues */
    if( info != 0 ) {
        if(VL!=NULL)
            memset(VL, 0, dim*dim*sizeof(double_complex));
        if(VR!=NULL)
            memset(VR, 0, dim*dim*sizeof(double_complex));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_EVG);
#endif
    }
    /* transpose, back to row-major */
    else{
        if(D!=NULL)
            for(i=0; i<dim; i++)
                D[i*dim+i] = ccdiv(alpha[i],beta[i]);
        
        if(VL!=NULL)
            for(i=0; i<dim; i++)
                for(j=0; j<dim; j++)
                    VL[i*dim+j] = vl[j*dim+i];
        if(VR!=NULL)
            for(i=0; i<dim; i++)
                for(j=0; j<dim; j++)
                    VR[i*dim+j] = vr[j*dim+i];
    }
    
    free(a);
    free(b);
    free(vl);
    free(vr);
    free(alpha);
    free(beta);
}


/* ========================================================================== */
/*                       Eigenvalue Decomposition (?eig)                      */
/* ========================================================================== */

void utility_ceig
(
    const float_complex* A,
    const int dim,
    int sortDecFLAG,
    float_complex* VL,
    float_complex* VR,
    float_complex* D,
    float* eig
)
{
    int i, j, n, lda, ldvl, ldvr, info;
    float_complex *w, *vl, *vr, *a;
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    int lwork;
    float* rwork;
    float_complex wkopt;
    float_complex* work;
#endif

    n = lda = ldvl = ldvr = dim;
    w = malloc1d(dim*sizeof(float_complex));
    vl = malloc1d(dim*dim*sizeof(float_complex));
    vr = malloc1d(dim*dim*sizeof(float_complex));
    a = malloc1d(dim*dim*sizeof(float_complex));
    
    /* store in column major order (i.e. transpose) */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[i*dim+j] = A[j*dim+i];

	assert(0); /* below code was crashing. Consider using utility_cseig instead, if A is hermitian */
    
    /* solve the eigenproblem */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    lwork = -1;
    rwork = malloc1d(2*dim*sizeof(float));
    cgeev_( "Vectors", "Vectors", &n, (veclib_float_complex*)a, &lda, (veclib_float_complex*)w, (veclib_float_complex*)vl, &ldvl,
           (veclib_float_complex*)vr, &ldvr, (veclib_float_complex*)&wkopt, &lwork, rwork, &info );
    lwork = (int)crealf(wkopt);
    work = malloc1d( lwork*sizeof(float_complex) );
    cgeev_( "Vectors", "Vectors", &n, (veclib_float_complex*)a, &lda, (veclib_float_complex*)w, (veclib_float_complex*)vl, &ldvl,
           (veclib_float_complex*)vr, &ldvr, (veclib_float_complex*)work, &lwork, rwork, &info );
    free(rwork);
    free(work);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    assert(0); /* no such implementation in clapack */
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_cgeev(CblasColMajor, 'V', 'V', n, (veclib_float_complex*)a, lda, (veclib_float_complex*)w, (veclib_float_complex*)vl, ldvl,
                         (veclib_float_complex*)vr, ldvr );
#endif
    
    /* sort the eigenvalues */
    float* wr;
    int* sort_idx;
    wr=malloc1d(dim*sizeof(float));
    sort_idx=malloc1d(dim*sizeof(int));
    for(i=0; i<dim; i++)
        wr[i] = crealf(w[i]);
    sortf(wr, NULL, sort_idx, dim, sortDecFLAG);
    
    /* output */
    if(D!=NULL)
        memset(D, 0, dim*dim*sizeof(float_complex));
    
    /* failed to converge and find the eigenvalues */
    if( info != 0 ) {
        if(VL!=NULL)
            memset(VL, 0, dim*dim*sizeof(float_complex));
        if(VR!=NULL)
            memset(VR, 0, dim*dim*sizeof(float_complex));
        if(eig!=NULL)
            memset(eig, 0, dim*sizeof(float));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_EVG);
#endif
    }
    /* transpose, back to row-major */
    else{
        for(i=0; i<dim; i++){
            if(VL!=NULL)
                for(j=0; j<dim; j++)
                    VL[i*dim+j] = vl[sort_idx[j]*dim+i];
            if(VR!=NULL)
                for(j=0; j<dim; j++)
                    VR[i*dim+j] = vr[sort_idx[j]*dim+i];
            if(D!=NULL)
                D[i*dim+i] = cmplxf(wr[i], 0.0f); /* store along the diagonal */
            if(eig!=NULL)
                eig[i] = wr[i];
        }
    }
    
    free(w);
    free(vl);
    free(vr);
    free(a);
    free(wr);
    free(sort_idx);
}


/* ========================================================================== */
/*                       General Linear Solver (?glslv)                       */
/* ========================================================================== */

void utility_sglslv
(
    const float* A,
    const int dim,
    float* B,
    int nCol,
    float* X
)
{
    int i, j, n = dim, nrhs = nCol, lda = dim, ldb = dim, info;
    int* IPIV;
    float* a, *b;

    IPIV = malloc1d(dim*sizeof(int));
    a = malloc1d(dim*dim*sizeof(float));
    b = malloc1d(dim*nrhs*sizeof(float));
    
    /* store in column major order */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[j*dim+i] = A[i*dim+j];
    for(i=0; i<dim; i++)
        for(j=0; j<nCol; j++)
            b[j*dim+i] = B[i*nCol+j];
    
    /* solve Ax = b for each column in b (b is replaced by the solution: x) */
#ifdef VECLIB_USE_CLAPACK_INTERFACE
    info = clapack_sgesv(CblasColMajor, n, nrhs, a, lda, IPIV, b, ldb);
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_sgesv(CblasColMajor, n, nrhs, a, lda, IPIV, b, ldb );
#elif defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    sgesv_( &n, &nrhs, a, &lda, IPIV, b, &ldb, &info );
#endif
    
    if(info!=0){
        /* A is singular, solution not possible */
        memset(X, 0, dim*nCol*sizeof(float));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_SOLVE_LINEAR_EQUATION);
#endif
    }
    else{
        /* store solution in row-major order */
        for(i=0; i<dim; i++)
            for(j=0; j<nCol; j++)
                X[i*nCol+j] = b[j*dim+i];
    }
    
    free(IPIV);
    free(a);
    free(b);
}

void utility_cglslv
(
    const float_complex* A,
    const int dim,
    float_complex* B,
    int nCol,
    float_complex* X
)
{
    int i, j, n = dim, nrhs = nCol, lda = dim, ldb = dim, info;
    int* IPIV;
    IPIV = malloc1d(dim*sizeof(int));
    float_complex* a, *b;

    a = malloc1d(dim*dim*sizeof(float_complex));
    b = malloc1d(dim*nrhs*sizeof(float_complex));
    
    /* store in column major order */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[j*dim+i] = A[i*dim+j];
    for(i=0; i<dim; i++)
        for(j=0; j<nCol; j++)
            b[j*dim+i] = B[i*nCol+j];
    
    /* solve Ax = b for each column in b (b is replaced by the solution: x) */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    cgesv_( &n, &nrhs, (veclib_float_complex*)a, &lda, IPIV, (veclib_float_complex*)b, &ldb, &info );
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    info = clapack_cgesv(CblasColMajor, n, nrhs, (veclib_float_complex*)a, lda, IPIV, (veclib_float_complex*)b, ldb);
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_cgesv(CblasColMajor, n, nrhs, (veclib_float_complex*)a, lda, IPIV, (veclib_float_complex*)b, ldb);
#endif
    
    /* A is singular, solution not possible */
    if(info!=0){
        memset(X, 0, dim*nCol*sizeof(float_complex));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_SOLVE_LINEAR_EQUATION);
#endif
    }
    /* store solution in row-major order */
    else{
        for(i=0; i<dim; i++)
            for(j=0; j<nCol; j++)
                X[i*nCol+j] = b[j*dim+i];
    }
    
    free(IPIV);
    free(a);
    free(b);
}

void utility_dglslv
(
    const double* A,
    const int dim,
    double* B,
    int nCol,
    double* X
)
{
    int i, j, n = dim, nrhs = nCol, lda = dim, ldb = dim, info;
    int* IPIV;
    double* a, *b;
 
    IPIV = malloc1d(dim*sizeof(int));
    a = malloc1d(dim*dim*sizeof(double));
    b = malloc1d(dim*nrhs*sizeof(double));
    
    /* store in column major order */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[j*dim+i] = A[i*dim+j];
    for(i=0; i<dim; i++)
        for(j=0; j<nCol; j++)
            b[j*dim+i] = B[i*nCol+j];
    
    /* solve Ax = b for each column in b (b is replaced by the solution: x) */
#ifdef VECLIB_USE_CLAPACK_INTERFACE
    info = clapack_dgesv(CblasColMajor, n, nrhs, a, lda, IPIV, b, ldb);
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_dgesv(CblasColMajor, n, nrhs, a, lda, IPIV, b, ldb);
#elif defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    dgesv_( &n, &nrhs, a, &lda, IPIV, b, &ldb, &info );
#endif
    
    /* A is singular, solution not possible */
    if(info!=0){
        memset(X, 0, dim*nCol*sizeof(double));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_SOLVE_LINEAR_EQUATION);
#endif
    }
    /* store solution in row-major order */
    else{
        for(i=0; i<dim; i++)
            for(j=0; j<nCol; j++)
                X[i*nCol+j] = b[j*dim+i];
    }
    
    free(IPIV);
    free(a);
    free(b);
}

void utility_zglslv
(
    const double_complex* A,
    const int dim,
    double_complex* B,
    int nCol,
    double_complex* X
)
{
    int i, j, n = dim, nrhs = nCol, lda = dim, ldb = dim, info;
    int* IPIV;
    double_complex* a, *b;
 
    IPIV = malloc1d(dim*sizeof(int));
    a = malloc1d(dim*dim*sizeof(double_complex));
    b = malloc1d(dim*nrhs*sizeof(double_complex));
    
    /* store in column major order */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[j*dim+i] = A[i*dim+j];
    for(i=0; i<dim; i++)
        for(j=0; j<nCol; j++)
            b[j*dim+i] = B[i*nCol+j];
    
    /* solve Ax = b for each column in b (b is replaced by the solution: x) */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    zgesv_( &n, &nrhs, (veclib_double_complex*)a, &lda, IPIV, (veclib_double_complex*)b, &ldb, &info );
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    info = clapack_zgesv(CblasColMajor, n, nrhs, (veclib_double_complex*)a, lda, IPIV, (veclib_double_complex*)b, ldb);
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_zgesv(CblasColMajor, n, nrhs, (veclib_double_complex*)a, lda, IPIV, (veclib_double_complex*)b, ldb);
#endif
    
    /* A is singular, solution not possible */
    if(info!=0){
        memset(X, 0, dim*nCol*sizeof(double_complex));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_SOLVE_LINEAR_EQUATION);
#endif
    }
    /* store solution in row-major order */
    else{
        for(i=0; i<dim; i++)
            for(j=0; j<nCol; j++)
                X[i*nCol+j] = b[j*dim+i];
    }
    
    free(IPIV);
    free(a);
    free(b);
}


/* ========================================================================== */
/*                      Symmetric Linear Solver (?slslv)                      */
/* ========================================================================== */

void utility_sslslv
(
    const float* A,
    const int dim,
    float* B,
    int nCol,
    float* X
)
{
    int i, j, n = dim, nrhs = nCol, lda = dim, ldb = dim, info;
    float* a, *b;
 
    a = malloc1d(dim*dim*sizeof(float));
    b = malloc1d(dim*nrhs*sizeof(float));
    
    /* store in column major order */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[j*dim+i] = A[i*dim+j];
    for(i=0; i<dim; i++)
        for(j=0; j<nCol; j++)
            b[j*dim+i] = B[i*nCol+j];
    
    /* solve Ax = b for each column in b (b is replaced by the solution: x) */
#ifdef VECLIB_USE_CLAPACK_INTERFACE
    info = clapack_sposv(CblasColMajor, CblasUpper, n, nrhs, a, lda, b, ldb);
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_sposv(CblasColMajor, CblasUpper, n, nrhs, a, lda, b, ldb);
#elif defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    sposv_( "U", &n, &nrhs, a, &lda, b, &ldb, &info );
#endif
    
    /* A is not symmetric positive definate, solution not possible */
    if(info!=0){
        memset(X, 0, dim*nCol*sizeof(float));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_SOLVE_LINEAR_EQUATION);
#endif
    }
    /* store solution in row-major order */
    else{
        for(i=0; i<dim; i++)
            for(j=0; j<nCol; j++)
                X[i*nCol+j] = b[j*dim+i];
    }
    
    free(a);
    free(b);
}

void utility_cslslv
(
    const float_complex* A,
    const int dim,
    float_complex* B,
    int nCol,
    float_complex* X
)
{
    int i, j, n = dim, nrhs = nCol, lda = dim, ldb = dim, info;
    float_complex* a, *b;
 
    a = malloc1d(dim*dim*sizeof(float_complex));
    b = malloc1d(dim*nrhs*sizeof(float_complex));
    
    /* store in column major order */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[j*dim+i] = A[i*dim+j];
    for(i=0; i<dim; i++)
        for(j=0; j<nCol; j++)
            b[j*dim+i] = B[i*nCol+j];
    
    /* solve Ax = b for each column in b (b is replaced by the solution: x) */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    cposv_( "U", &n, &nrhs, (veclib_float_complex*)a, &lda, (veclib_float_complex*)b, &ldb, &info );
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    info = clapack_cposv(CblasColMajor, CblasUpper, n, nrhs, (veclib_float_complex*)a, lda, (veclib_float_complex*)b, ldb);
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_cposv(CblasColMajor, CblasUpper, n, nrhs, (veclib_float_complex*)a, lda, (veclib_float_complex*)b, ldb);
#endif
    
    /* A is not symmetric positive definate, solution not possible */
    if(info!=0){
        memset(X, 0, dim*nCol*sizeof(float_complex));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_SOLVE_LINEAR_EQUATION);
#endif
    }
    /* store solution in row-major order */
    else{
        for(i=0; i<dim; i++)
            for(j=0; j<nCol; j++)
                X[i*nCol+j] = b[j*dim+i];
    }
    
    free(a);
    free(b);
}


/* ========================================================================== */
/*                        Matrix Pseudo-Inverse (?pinv)                       */
/* ========================================================================== */

void utility_spinv
(
    const float* inM,
    const int dim1,
    const int dim2,
    float* outM
)
{
    
    int i, j, m, n, k, lda, ldu, ldvt, info;
    float* a, *s, *u, *vt, *inva;
    float ss;
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    int lwork;
    float wkopt;
    float* work;
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    float* superb;
#endif
    
    m = lda = ldu = dim1;
    n = dim2;
    k = ldvt = m < n ? m : n;
    a = malloc1d(m*n*sizeof(float) );
    s = (float*)malloc1d(k*sizeof(float));
    u = (float*)malloc1d(ldu*k*sizeof(float));
    vt = (float*)malloc1d(ldvt*n*sizeof(float));
    
    /* store in column major order */
    for(i=0; i<m; i++)
        for(j=0; j<n; j++)
            a[j*m+i] = inM[i*n+j];
    
    /* singular value decomposition */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    lwork = -1;
    sgesvd_("S", "S", &m, &n, a, &lda, s, u, &ldu, vt, &ldvt, &wkopt, &lwork, &info);
    lwork = (int)wkopt;
    work = (float*)malloc1d(lwork*sizeof(float));
    sgesvd_("S", "S", &m, &n, a, &lda, s, u, &ldu, vt, &ldvt, work, &lwork, &info );
    free(work);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    assert(0); /* no such implementation in clapack */
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    superb = malloc((MIN(m,n)-1)*sizeof(float));
    info = LAPACKE_sgesvd(CblasColMajor, 'S', 'S', m, n, a, lda, s, u, ldu, vt, ldvt, superb);
    free(superb);
#endif
    
    if( info != 0 ) {
        memset(outM, 0, dim1*dim2*sizeof(float));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_SVD);
#endif
    }
    int incx=1;
    for(i=0; i<k; i++){
        if(s[i] > 1.0e-5f)
            ss=1.0f/s[i];
        else
            ss=s[i];
        cblas_sscal(m, ss, &u[i*m], incx);
    }
    inva = (float*)malloc1d(n*m*sizeof(float));
    int ld_inva=n;
    cblas_sgemm(CblasColMajor, CblasTrans, CblasTrans, n, m, k, 1.0f,
                vt, ldvt,
                u, ldu, 0.0f,
                inva, ld_inva);
    
    /* return in row-major order */
    for(i=0; i<m; i++)
        for(j=0; j<n; j++)
            outM[j*m+i] = inva[i*n+j];
    
    /* clean-up */
    free(a);
    free(s);
    free(u);
    free(vt);
    free(inva);
}

void utility_cpinv
(
    const float_complex* inM,
    const int dim1,
    const int dim2,
    float_complex* outM
)
{
    int i, j, m, n, k, lda, ldu, ldvt, info;
    float_complex* a,  *u, *vt, *inva;
    float_complex  ss_cmplx;
    const float_complex calpha = cmplxf(1.0f, 0.0f); const float_complex cbeta = cmplxf(0.0f, 0.0f); /* blas */
    float *s;
    float ss;
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    int lwork;
    float* rwork;
    float_complex wkopt;
    float_complex* work;
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    float* superb;
#endif
    
    m = lda = ldu = dim1;
    n = dim2;
    k = ldvt = m < n ? m : n;
    a = malloc1d(lda*n*sizeof(float_complex));
    s = malloc1d(MIN(n,m)*sizeof(float));
    u = malloc1d(ldu*m*sizeof(float_complex));
    vt = malloc1d(ldvt*n*sizeof(float_complex));
    
    /* store in column major order */
    for(i=0; i<dim1; i++)
        for(j=0; j<dim2; j++)
            a[j*dim1+i] = inM[i*dim2 +j];
    
    /* singular value decomposition */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    rwork = malloc1d(m*MAX(1, 5*MIN(n,m))*sizeof(float));
    lwork = -1;
    cgesvd_( "A", "A", &m, &n, (veclib_float_complex*)a, &lda, s, (veclib_float_complex*)u, &ldu, (veclib_float_complex*)vt, &ldvt,
            (veclib_float_complex*)&wkopt, &lwork, rwork, &info );
    lwork = (int)(crealf(wkopt)+0.01f);
    work = malloc1d( lwork*sizeof(float_complex) );
    cgesvd_( "A", "A", &m, &n, (veclib_float_complex*)a, &lda, s, (veclib_float_complex*)u, &ldu, (veclib_float_complex*)vt, &ldvt,
            (veclib_float_complex*)work, &lwork, rwork, &info);
    free(work);
    free(rwork);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    assert(0); /* no such implementation in clapack */
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    superb = malloc((MIN(m,n)-1)*sizeof(float));
    info = LAPACKE_cgesvd(CblasColMajor, 'S', 'S', m, n, (veclib_float_complex*)a, lda, s, (veclib_float_complex*)u, ldu,
                          (veclib_float_complex*)vt, ldvt, superb);
    free(superb);
#endif
    
    if( info != 0 ) {
        memset(outM, 0, dim1*dim2*sizeof(float_complex));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_SVD);
#endif
    }
    int incx=1;
    for(i=0; i<k; i++){
        if(s[i] > 1.0e-5f)
            ss=1.0f/s[i];
        else
            ss=s[i];
        ss_cmplx = cmplxf(ss, 0.0f);
        cblas_cscal(m, &ss_cmplx, &u[i*m], incx);
    }
    inva = malloc1d(n*m*sizeof(float_complex));
    int ld_inva=n;
    cblas_cgemm(CblasColMajor, CblasConjTrans, CblasConjTrans, n, m, k, &calpha,
                vt, ldvt,
                u, ldu, &cbeta,
                inva, ld_inva);
    
    /* return in row-major order */
    for(i=0; i<m; i++)
        for(j=0; j<n; j++)
            outM[j*m+i] = inva[i*n+j];

    /* clean-up */
    free(a);
    free(s);
    free(u);
    free(vt);
    free(inva);
}

void utility_dpinv
(
    const double* inM,
    const int dim1,
    const int dim2,
    double* outM
)
{
    int i, j, m, n, k, lda, ldu, ldvt, info;
    double* a, *s, *u, *vt, *inva;
    double ss;
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    int lwork;
    double wkopt;
    double* work;
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    double* superb;
#endif
    
    m = lda = ldu = dim1;
    n = dim2;
    k = ldvt = m < n ? m : n;
    a = malloc1d(m*n*sizeof(double) );
    s = (double*)malloc1d(k*sizeof(double));
    u = (double*)malloc1d(ldu*k*sizeof(double));
    vt = (double*)malloc1d(ldvt*n*sizeof(double));
    
    /* store in column major order */
    for(i=0; i<m; i++)
        for(j=0; j<n; j++)
            a[j*m+i] = inM[i*n+j];
    
    /* singular value decomposition */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    lwork = -1;
    dgesvd_("S", "S", &m, &n, a, &lda, s, u, &ldu, vt, &ldvt, &wkopt, &lwork, &info);
    lwork = (int)wkopt;
    work = (double*)malloc1d(lwork*sizeof(double));
    dgesvd_("S", "S", &m, &n, a, &lda, s, u, &ldu, vt, &ldvt, work, &lwork, &info );
    free(work);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    assert(0); /* no such implementation in clapack */
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    superb = malloc((MIN(m,n)-1)*sizeof(double));
    info = LAPACKE_dgesvd(CblasColMajor, 'S', 'S', m, n, a, lda, s, u, ldu, vt, ldvt, superb);
    free(superb);
#endif
    
    if( info != 0 ) {
        memset(outM, 0, dim1*dim2*sizeof(float));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_SVD);
#endif
    }
    int incx=1;
    for(i=0; i<k; i++){
        if(s[i] > 1.0e-9)
            ss=1.0/s[i];
        else
            ss=s[i];
        cblas_dscal(m, ss, &u[i*m], incx);
    }
    inva = (double*)malloc1d(n*m*sizeof(double));
    int ld_inva=n;
    cblas_dgemm( CblasColMajor, CblasTrans, CblasTrans, n, m, k, 1.0,
                vt, ldvt,
                u, ldu, 0.0,
                inva, ld_inva);
    
    /* return in row-major order */
    for(i=0; i<m; i++)
        for(j=0; j<n; j++)
            outM[j*m+i] = inva[i*n+j];
    
    /* clean-up */
    free(a);
    free(s);
    free(u);
    free(vt);
    free(inva);
}

void utility_zpinv
(
    const double_complex* inM,
    const int dim1,
    const int dim2,
    double_complex* outM
)
{
    int i, j, m, n, k, lda, ldu, ldvt, info;
    double_complex* a, *u, *vt, *inva;
    double_complex ss_cmplx;
    const double_complex calpha = cmplx(1.0, 0.0); const double_complex cbeta = cmplx(0.0, 0.0); /* blas */
    double* s;
    double ss;
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    int lwork;
    double* rwork;
    double_complex wkopt;
    double_complex* work;
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    double* superb;
#endif
    
    m = lda = ldu = dim1;
    n = dim2;
    k = ldvt = m < n ? m : n;
    a = malloc1d(lda*n*sizeof(double_complex));
    s = malloc1d(MIN(n,m)*sizeof(double));
    u = malloc1d(ldu*m*sizeof(double_complex));
    vt = malloc1d(ldvt*n*sizeof(double_complex));
    
    /* store in column major order */
    for(i=0; i<dim1; i++)
        for(j=0; j<dim2; j++)
            a[j*dim1+i] = inM[i*dim2 +j];
    
    /* singular value decomposition */
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    rwork = malloc1d(m*MAX(1, 5*MIN(n,m))*sizeof(double));
    lwork = -1;
    zgesvd_( "A", "A", &m, &n, (veclib_double_complex*)a, &lda, s, (veclib_double_complex*)u, &ldu, (veclib_double_complex*)vt, &ldvt,
            (veclib_double_complex*)&wkopt, &lwork, rwork, &info );
    lwork = (int)(creal(wkopt)+0.01);
    work = malloc1d( lwork*sizeof(double_complex) );
    zgesvd_( "A", "A", &m, &n, (veclib_double_complex*)a, &lda, s, (veclib_double_complex*)u, &ldu, (veclib_double_complex*)vt, &ldvt,
            (veclib_double_complex*)work, &lwork, rwork, &info);
    free(work);
    free(rwork);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    assert(0); /* no such implementation in clapack */
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    superb = malloc((MIN(m,n)-1)*sizeof(double));
    info = LAPACKE_zgesvd(CblasColMajor, 'A', 'A', m, n, (veclib_double_complex*)a, lda, s, (veclib_double_complex*)u, ldu,
                          (veclib_double_complex*)vt, ldvt, superb);
    free(superb);
#endif

    if( info != 0 ) {
        memset(outM, 0, dim1*dim2*sizeof(double_complex));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_SVD);
#endif
    }
    int incx=1;
    for(i=0; i<k; i++){
        if(s[i] > 1.0e-5)
            ss=1.0/s[i];
        else
            ss=s[i];
        ss_cmplx = cmplx(ss, 0.0);
        cblas_zscal(m, &ss_cmplx, &u[i*m], incx);
    }
    inva = malloc1d(n*m*sizeof(double_complex));
    int ld_inva=n;
    cblas_zgemm(CblasColMajor, CblasConjTrans, CblasConjTrans, n, m, k, &calpha,
                vt, ldvt,
                u, ldu, &cbeta,
                inva, ld_inva);
    
    /* return in row-major order */
    for(i=0; i<m; i++)
        for(j=0; j<n; j++)
            outM[j*m+i] = inva[i*n+j];
    
    /* clean-up */
    free(a);
    free(s);
    free(u);
    free(vt);
    free(inva);
}


/* ========================================================================== */
/*                       Cholesky Factorisation (?chol)                       */
/* ========================================================================== */

void utility_schol
(
    const float* A,
    const int dim,
    float* X
)
{
    int i, j, info, n, lda;
    float* a;
 
    n = lda = dim;
    a = malloc1d(dim*dim*sizeof(float));
    
    /* store in column major order */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[j*dim+i] = A[i*dim+j];
    
    /* a is replaced by solution */
#ifdef VECLIB_USE_CLAPACK_INTERFACE
    info = clapack_spotrf(CblasColMajor, CblasUpper, n, a, lda);
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_spotrf(CblasColMajor, CblasUpper, n, a, lda);
#elif defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    spotrf_( "U", &n, a, &lda, &info );
#endif
    
    /* A is not positive definate, solution not possible */
    if(info>0){
        memset(X, 0, dim*dim*sizeof(float));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_CHOL);
#endif
    }
    
    /* store solution in row-major order */
    else{
        for(i=0; i<dim; i++)
            for(j=0; j<dim; j++)
                X[i*dim+j] = j>=i ? a[j*dim+i] : 0.0f;
    }
    
    free(a);
}

void utility_cchol
(
    const float_complex* A,
    const int dim,
    float_complex* X
)
{
    int i, j, info, n, lda;
    float_complex* a;
    
    n = lda = dim;
    a = malloc1d(dim*dim*sizeof(float_complex));
    
    /* store in column major order */
    for(i=0; i<dim; i++)
        for(j=0; j<dim; j++)
            a[j*dim+i] = A[i*dim+j];
    
    /* a is replaced by solution */
#if defined(VECLIB_USE_CLAPACK_INTERFACE)
    info = clapack_cpotrf(CblasColMajor, CblasUpper, n, (veclib_float_complex*)a, lda);
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    info = LAPACKE_cpotrf(CblasColMajor, CblasUpper, n, (veclib_float_complex*)a, lda);
#elif defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    cpotrf_( "U", &n, (veclib_float_complex*)a, &lda, &info );
#endif
    
    /* A is not positive definate, solution not possible */
    if(info>0){
        memset(X, 0, dim*dim*sizeof(float_complex));
#ifndef NDEBUG
        saf_error_print(SAF_WARNING__FAILED_TO_COMPUTE_CHOL);
#endif
    }
    /* store solution in row-major order */
    else{
        for(i=0; i<dim; i++)
            for(j=0; j<dim; j++)
                X[i*dim+j] = j>=i ? a[j*dim+i] : cmplxf(0.0f, 0.0f);
    }
    
    free(a);
}


/* ========================================================================== */
/*                           Matrix Inversion (?inv)                          */
/* ========================================================================== */

// TODO: rewrite for row-major
void utility_sinv(float * A, const int N)
{
    int *IPIV;
    IPIV = malloc1d(N * sizeof(int));
    int LWORK = N*N;
    float *WORK;
    WORK = (float*)malloc1d(LWORK * sizeof(float));
    int INFO;
    
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    sgetrf_((veclib_int*)&N, (veclib_int*)&N, A, (veclib_int*)&N, IPIV, &INFO);
    sgetri_((veclib_int*)&N, A, (veclib_int*)&N, IPIV, WORK, &LWORK, &INFO);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    INFO = clapack_sgetrf(CblasColMajor, N, N, A, N, IPIV);
    INFO = clapack_sgetri(CblasColMajor, N, A, N, IPIV);
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    INFO = LAPACKE_sgetrf(CblasColMajor, N, N, A, N, IPIV);
    INFO = LAPACKE_sgetri(CblasColMajor, N, A, N, IPIV);
#endif
    
    free(IPIV);
    free(WORK);
}

void utility_dinv(double* A, const int N)
{
    int *IPIV;
    IPIV = malloc1d( (N+1)*sizeof(int));
    int LWORK = N*N;
    double *WORK;
    WORK = malloc1d( LWORK*sizeof(double));
    int INFO;
    
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    dgetrf_((veclib_int*)&N, (veclib_int*)&N, A, (veclib_int*)&N, IPIV, &INFO);
    dgetri_((veclib_int*)&N, A, (veclib_int*)&N, IPIV, WORK, &LWORK, &INFO);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    INFO = clapack_dgetrf(CblasColMajor, N, N, A, N, IPIV);
    INFO = clapack_dgetri(CblasColMajor, N, A, N, IPIV);
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    INFO = LAPACKE_dgetrf(CblasColMajor, N, N, A, N, IPIV);
    INFO = LAPACKE_dgetri(CblasColMajor, N, A, N, IPIV);
#endif
    
    free(IPIV);
    free(WORK);
}

void utility_cinv(float_complex * A, const int N)
{
    int *IPIV;
    IPIV = malloc1d(N * sizeof(int));
    int LWORK = N*N;
    float_complex *WORK;
    WORK = (float_complex*)malloc1d(LWORK * sizeof(float_complex));
    int INFO;
    
#if defined(VECLIB_USE_LAPACK_FORTRAN_INTERFACE)
    cgetrf_((veclib_int*)&N, (veclib_int*)&N, (veclib_float_complex*)A, (veclib_int*)&N, IPIV, &INFO);
    cgetri_((veclib_int*)&N, (veclib_float_complex*)A, (veclib_int*)&N, IPIV, (veclib_float_complex*)WORK, &LWORK, &INFO);
#elif defined(VECLIB_USE_CLAPACK_INTERFACE)
    INFO = clapack_cgetrf(CblasColMajor, N, N, (veclib_float_complex*)A, N, IPIV);
    INFO = clapack_cgetri(CblasColMajor, N, (veclib_float_complex*)A, N, IPIV);
#elif defined(VECLIB_USE_LAPACKE_INTERFACE)
    INFO = LAPACKE_cgetrf(CblasColMajor, N, N, (veclib_float_complex*)A, N, IPIV);
    INFO = LAPACKE_cgetri(CblasColMajor, N, (veclib_float_complex*)A, N, IPIV);
#endif
    
    free(IPIV);
    free(WORK);
}
