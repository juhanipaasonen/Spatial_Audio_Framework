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
 * @file saf_sh_internal.c
 * @ingroup SH
 * @brief Internal source for the Spherical Harmonic Transform and Spherical
 *        Array Processing module (#SAF_SH_MODULE)
 *
 * A collection of spherical harmonic related functions. Many of which have been
 * derived from Matlab libraries by Archontis Politis [1-3].
 *
 * @see [1] https://github.com/polarch/Spherical-Harmonic-Transform
 * @see [2] https://github.com/polarch/Array-Response-Simulator
 * @see [3] https://github.com/polarch/Spherical-Array-Processing
 *
 * @author Leo McCormack
 * @date 22.05.2016
 */

#include "saf_sh.h"
#include "saf_sh_internal.h"

/* ========================================================================== */
/*                          Misc. Internal Functions                          */
/* ========================================================================== */

float wigner_3j
(
    int j1,
    int j2,
    int j3,
    int m1,
    int m2,
    int m3
)
{
    int t, N_t;
    float w, coeff1, coeff2, tri_coeff, sum_s, x_t;
    
    /* Check selection rules (http://mathworld.wolfram.com/Wigner3j-Symbol.html) */
    if (abs(m1)>abs(j1) || abs(m2)>abs(j2) || abs(m3)>abs(j3))
        w = 0.0f;
    else if (m1+m2+m3 !=0)
        w = 0.0f;
    else if ( j3<abs(j1-j2) || j3>(j1+j2) ) /* triangle inequality */
        w = 0.0f;
    
    else {
        /* evaluate the Wigner-3J symbol using the Racah formula (http://mathworld.wolfram.com/Wigner3j-Symbol.html)
         number of terms for the summation */
        N_t = -1000000000;
        N_t = j1+m1 > N_t ? j1+m1 : N_t;
        N_t = j1-m1 > N_t ? j1-m1 : N_t;
        N_t = j2+m2 > N_t ? j2+m2 : N_t;
        N_t = j2-m2 > N_t ? j2-m2 : N_t;
        N_t = j3+m3 > N_t ? j3+m3 : N_t;
        N_t = j3-m3 > N_t ? j3-m3 : N_t;
        N_t = j1+j2-j3 > N_t ? j1+j2-j3 : N_t;
        N_t = j2+j3-j1 > N_t ? j2+j3-j1 : N_t;
        N_t = j3+j1-j2 > N_t ? j3+j1-j2 : N_t;
        
        /* coefficients before the summation */
        coeff1 = powf(-1.0f,(float)(j1-j2-m3));
        coeff2 = (float)(factorial(j1+m1)*(float)factorial(j1-m1)*(float)factorial(j2+m2)*(float)factorial(j2-m2)* (float)factorial(j3+m3)*(float)factorial(j3-m3));
        tri_coeff = (float)(factorial(j1 + j2 - j3)*(float)factorial(j1 - j2 + j3)*(float)factorial(-j1 + j2 + j3)/(float)factorial(j1 + j2 + j3 + 1));
        
        /* summation over integers that do not result in negative factorials */
        sum_s = 0.0f;
        for (t = 0; t<=N_t; t++){
            
            /* check factorial for negative values, include in sum if not */
            if (j3-j2+t+m1 >= 0 && j3-j1+t-m2 >=0 && j1+j2-j3-t >= 0 && j1-t-m1 >=0 && j2-t+m2 >= 0){
                x_t = (float)factorial(t)*(float)factorial(j1+j2-j3-t)*(float)factorial(j3-j2+t+m1)*(float)factorial(j3-j1+t-m2)*(float)factorial(j1-t-m1)*(float)factorial(j2-t+m2);
                sum_s += powf(-1.0f, (float)t)/x_t;
            }
        }
        w = coeff1*sqrtf(coeff2*tri_coeff)*sum_s;
    }
    return w;
}

void gaunt_mtx
(
    int N1,
    int N2,
    int N,
    float* A
)
{
    int n, m, q, n1, m1, q1, n2, m2, q2, D1, D2, D3;
    float wigner3jm, wigner3j0;
     
    D1 = (N1+1)*(N1+1);
    D2 = (N2+1)*(N2+1);
    D3 = (N+1)*(N+1);
    memset(A, 0, D1*D2*D3*sizeof(float));
    for (n = 0; n<=N; n++){
        for (m = -n; m<=n; m++){
            q = n*(n+1)+m;
            
            for (n1 = 0; n1<=N1; n1++){
                for (m1 = -n1; m1<=n1; m1++){
                    q1 = n1*(n1+1)+m1;
                    
                    for (n2 = 0; n2<=N2; n2++){
                        for (m2 = -n2; m2<=n2; m2++){
                            q2 = n2*(n2+1)+m2;
                            
                            if (n<abs(n1-n2) || n>n1+n2)
                                A[q1*D2*D3 + q2*D3 + q] = 0.0f;
                            else{
                                wigner3jm = wigner_3j(n1, n2, n, m1, m2, -m);
                                wigner3j0 = wigner_3j(n1, n2, n, 0, 0, 0);
                                A[q1*D2*D3 + q2*D3 + q] = powf(-1.0f,(float)m) *
                                                          sqrtf((2.0f*(float)n1+1.0f)*(2.0f*(float)n2+1.0f)*(2.0f*(float)n+1.0f)/(4.0f*M_PI)) *
                                                          wigner3jm * wigner3j0;
                            }
                        }
                    }
                }
            }
        }
    }
}


/* ========================================================================== */
/*             Internal functions for spherical harmonic rotations            */
/* ========================================================================== */

/* Ivanic, J., Ruedenberg, K. (1998). Rotation Matrices for Real Spherical Harmonics. Direct Determination
 * by Recursion Page: Additions and Corrections. Journal of Physical Chemistry A, 102(45), 9099?9100. */
float getP
(
    int i,
    int l,
    int a,
    int b,
    float** R_1,
    float** R_lm1
)
{
    float ret, ri1, rim1, ri0;
    
    ri1 = R_1[i + 1][1 + 1];
    rim1 = R_1[i + 1][-1 + 1];
    ri0 = R_1[i + 1][0 + 1];
    
    if (b == -l)
        ret = ri1 * R_lm1[a + l - 1][0] + rim1 * R_lm1[a + l - 1][2 * l - 2];
    else {
        if (b == l)
            ret = ri1*R_lm1[a + l - 1][2 * l - 2] - rim1 * R_lm1[a + l - 1][0];
        else
            ret = ri0 * R_lm1[a + l - 1][b + l - 1];
    }
    
    return ret;
}

/* Ivanic, J., Ruedenberg, K. (1998). Rotation Matrices for Real Spherical Harmonics. Direct Determination
 * by Recursion Page: Additions and Corrections. Journal of Physical Chemistry A, 102(45), 9099?9100. */
float getU
(
    int l,
    int m,
    int n,
    float** R_1,
    float** R_lm1
)
{
    return getP(0, l, m, n, R_1, R_lm1);
}

/* Ivanic, J., Ruedenberg, K. (1998). Rotation Matrices for Real Spherical Harmonics. Direct Determination
 * by Recursion Page: Additions and Corrections. Journal of Physical Chemistry A, 102(45), 9099?9100. */
float getV
(
    int l,
    int m,
    int n,
    float** R_1,
    float** R_lm1
)
{
    int d;
    float ret, p0, p1;
    
    if (m == 0) {
        p0 = getP(1, l, 1, n, R_1, R_lm1);
        p1 = getP(-1, l, -1, n, R_1, R_lm1);
        ret = p0 + p1;
    }
    else {
        if (m>0) {
            d = m == 1 ? 1 : 0;
            p0 = getP(1, l, m - 1, n, R_1, R_lm1);
            p1 = getP(-1, l, -m + 1, n, R_1, R_lm1);
            ret = p0*sqrtf(1.0f + d) - p1*(1.0f - d);
        }
        else {
            d = m == -1 ? 1 : 0;
            p0 = getP(1, l, m + 1, n, R_1, R_lm1);
            p1 = getP(-1, l, -m - 1, n, R_1, R_lm1);
            ret = p0*(1.0f - (float)d) + p1*sqrtf(1.0f + (float)d);
        }
    }
    
    return ret;
}

/* Ivanic, J., Ruedenberg, K. (1998). Rotation Matrices for Real Spherical Harmonics. Direct Determination
 * by Recursion Page: Additions and Corrections. Journal of Physical Chemistry A, 102(45), 9099?9100. */
float getW
(
    int l,
    int m,
    int n,
    float** R_1,
    float** R_lm1
)
{
    float ret, p0, p1;
    ret = 0.0f;
    
    if (m != 0) {
        if (m>0) {
            p0 = getP(1, l, m + 1, n, R_1, R_lm1);
            p1 = getP(-1, l, -m - 1, n, R_1, R_lm1);
            ret = p0 + p1;
        }
        else {
            p0 = getP(1, l, m - 1, n, R_1, R_lm1);
            p1 = getP(-1, l, -m + 1, n, R_1, R_lm1);
            ret = p0 - p1;
        }
    }
    return ret;
}
