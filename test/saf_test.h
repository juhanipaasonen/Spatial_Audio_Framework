/**
 * @file saf_test.h
 * @brief Unit testing program for the Spatial_Audio_Framework
 *
 * @author Leo McCormack
 * @date 27.04.2020
 */ 

#ifndef __SAF_TEST_H_INCLUDED__
#define __SAF_TEST_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/** Main testing program */
int main_test(void);

/* ========================================================================== */
/*                            Available unit tests                            */
/* ========================================================================== */

/**
 * Testing for perfect reconstruction of the saf_stft (when configured for 50%
 * window overlap) */
void test__saf_stft_50pc_overlap(void);
/** 
 * Testing for perfect reconstruction of the saf_stft (when configured for
 * linear time-invariant (LTI) filtering applications) */
void test__saf_stft_LTI(void);
/**
 * Testing the ims shoebox simulator, when applying the echograms in the time-
 * domain */
void test__ims_shoebox_TD(void);
/**
 * Testing the ims shoebox simulator, when generating room impulse respones
 * (RIRs) from the computed echograms */
void test__ims_shoebox_RIR(void);
/**
 * Testing the forward and backward real-(half)complex FFT (saf_rfft) */
void test__saf_rfft(void);
/**
 * Testing the saf_matrixConv */
void test__saf_matrixConv(void);
#ifdef AFSTFT_USE_FLOAT_COMPLEX
/**
 * Testing the alias-free STFT filterbank reconstruction */
void test__afSTFTMatrix(void);
#endif
/**
 * Testing the alias-free STFT filterbank reconstruction */
void test__afSTFT(void);
/**
 * Testing the smb_pitchShifter */
void test__smb_pitchShifter(void);
/**
 * Testing the sortf() function (sorting real floating point numbers) */
void test__sortf(void);
/**
 * Testing the sortz() function (sorting complex double-floating point numbers)
 */
void test__sortz(void);
/**
 * Testing the cmplxPairUp() function (grouping up conjugate symmetric values)
 */
void test__cmplxPairUp(void);
/**
 * Testing that the weights from the getVoronoiWeights() function sum to 4pi
 * and that for a uniform arrangement of points, the weights are all identical
 */
void test__getVoronoiWeights(void);
/**
 * Testing that the unique_i() function operates correctly */
void test__unique_i(void);
/**
 * Testing the realloc2d_r() function (reallocating 2-D array, while retaining
 * the previous data order; except truncated or extended) */
void test__realloc2d_r(void);
/**
 * Testing the performance of the latticeDecorrelator, verifying that the inter
 * channel cross-correlation coefficients are near 0 */
void test__latticeDecorrelator(void);
/**
 * Testing the formulate_M_and_Cr() function, and verifying that the output
 * mixing matrices yield signals which have the target covariance */
void test__formulate_M_and_Cr(void);
/**
 * Testing the formulate_M_and_Cr_cmplx() function, and verifying that the
 * output mixing matrices yield signals which have the target covariance */
void test__formulate_M_and_Cr_cmplx(void);
/**
 * Testing to assure that (given a uniform loudspeaker layout), the SAD, MMD and
 * EPAD decoders are all equivalent */
void test__getLoudspeakerDecoderMtx(void);
/**
 * Testing the orthogonality of the getSHreal() function */
void test__getSHreal(void);
/**
 * Testing that the getSHreal_recur() function is somewhat numerically similar
 * to the getSHreal function */
void test__getSHreal_recur(void);
/**
 * Testing the orthogonality of the getSHcomplex() function */
void test__getSHcomplex(void);
/**
 * Testing the spherical harmonic rotation matrix function getSHrotMtxReal() */
void test__getSHrotMtxReal(void);
/**
 * Testing the real to complex spherical harmonic conversion, using
 * getSHcomplex() as the reference */
void test__real2complexSHMtx(void);
/**
 * Testing the complex to real spherical harmonic conversion, using getSHreal()
 * as the reference */
void test__complex2realSHMtx(void);
/**
 * Testing the computeSectorCoeffsEP() and computeVelCoeffsMtx() functions and
 * comparing their output to that of a reference matlab function */
void test__computeSectorCoeffsEP(void);
/**
 * Testing that for T-designs, the condition numbers are all equal to 1 */
void test__checkCondNumberSHTReal(void);
/**
 * Testing the DoA estimation performance of sphMUSIC() */
void test__sphMUSIC(void);
/**
 * Testing the DoA estimation performance of sphESPRIT() */
void test__sphESPRIT(void);
/**
 * Testing that the coefficients computed with butterCoeffs() are numerically
 * similar to the "butter" function in Matlab */
void test__butterCoeffs(void);
/**
 * Testing that the faf_IIRFilterbank can re-construct the original signal power
 */
void test__faf_IIRFilterbank(void);
/**
 * Testing the SAF ambi_bin example (this may also serve as a tutorial on how
 * to use it) */
void test__saf_example_ambi_bin(void);
/**
 * Testing the SAF ambi_dec example (this may also serve as a tutorial on how
 * to use it) */
void test__saf_example_ambi_dec(void);
/**
 * Testing the SAF ambi_enc example (this may also serve as a tutorial on how
 * to use it) */
void test__saf_example_ambi_enc(void);
/**
 * Testing the SAF array2sh example (this may also serve as a tutorial on how
 * to use it) */
void test__saf_example_array2sh(void);
/**
 * Testing the SAF rotator example (this may also serve as a tutorial on how
 * to use it) */
void test__saf_example_rotator(void);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __SAF_TEST_H_INCLUDED__ */