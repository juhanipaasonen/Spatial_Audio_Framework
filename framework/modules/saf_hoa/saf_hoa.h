/*
 * Copyright 2018 Leo McCormack
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
 * @file saf_hoa.h
 * @brief Public part of the higher-order Ambisonics module (saf_hoa)
 *
 * A collection of Ambisonics related functions. Many of which are derived from
 * the Matlab library by Archontis Politis [1].
 *
 * @see [1] https://github.com/polarch/Higher-Order-Ambisonics
 *
 * @author Leo McCormack
 * @date 19.03.2018
 */

#ifndef __SAF_HOA_H_INCLUDED__
#define __SAF_HOA_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "../saf_utilities/saf_complex.h"
#include "../saf_utilities/saf_error.h"

/* ========================================================================== */
/*                                    Enums                                   */
/* ========================================================================== */

/**
 * Ambisonic decoding options for loudspeaker playback
 *
 * Note that all of these decoding options revert to "SAD" if the loudspeakers
 * are uniformly distributed on the sphere. The benefits afforded by MMD,
 * AllRAD, etc. relate to their improved performance when using irregular
 * loudspeaker arrangements.
 *
 * @see [1] Zotter F, Pomberger H, Noisternig M. Energy--preserving ambisonic
 *          decoding. Acta Acustica united with Acustica. 2012 Jan 1;
 *          98(1):37-47.
 * @see [2] Zotter F, Frank M. All-round ambisonic panning and decoding. Journal
 *          of the audio engineering society. 2012 Nov 26; 60(10):807-20.
 */
typedef enum _LOUDSPEAKER_AMBI_DECODER_METHODS {
    /**
     * The default decoder is "LOUDSPEAKER_DECODER_SAD"
     */
    LOUDSPEAKER_DECODER_DEFAULT,
    /**
     * Sampling Ambisonic Decoder (SAD): transpose of the loudspeaker spherical
     * harmonic matrix, scaled by the number of loudspeakers. This is the
     * simplest decoding approach, as it simply relies on generating hyper-
     * cardioid beamformers for each loudspeaker direction.
     */
    LOUDSPEAKER_DECODER_SAD,
    /**
     * Mode-Matching Decoder (MMD): pseudo-inverse of the loudspeaker spherical
     * harmonic matrix. Due to the pseudo-inverse, more signal energy is lent to
     * regions on the surface of the sphere that are more sparsely populated
     * with loudspeakers. Therefore, one must also be careful, as some
     * loudspeakers may be given a huge amount of signal energy and wake the
     * dead.
     */
    LOUDSPEAKER_DECODER_MMD,
    /**
     * Energy-Preserving Ambisonic Decoder (EPAD) [1].
     */
    LOUDSPEAKER_DECODER_EPAD,
    /**
     * All-Round Ambisonic Decoder (AllRAD): SAD decoding to t-design, panned
     * for the target loudspeaker directions using VBAP [2]. Perhaps the
     * Ambisonic decoder we would most recommend for irregular loudspeaker
     * layouts.
     */
    LOUDSPEAKER_DECODER_ALLRAD
    
} LOUDSPEAKER_AMBI_DECODER_METHODS;

/**
 * Ambisonic decoding options for binaural/headphone playback
 *
 * @note A more detailed description of each method may be found in
 *       saf_hoa_internal.h.
 *
 * @see [1] Z. Ben-Hur, F. Brinkmann, J. Sheaffer, S. Weinzierl, and B. Rafaely,
 *          "Spectral equalization in binaural signals represented by order-
 *          truncated spherical harmonics" The Journal of the Acoustical
 *          Society of America, vol. 141, no. 6, pp. 4087--4096, 2017.
 * @see [2] B. Bernschutz, A. V. Giner, C. Po"rschmann, and J. Arend, "Binaural
 *          reproduction of plane waves with reduced modal order" Acta Acustica
 *          united with Acustica, vol. 100, no. 5, pp. 972--983, 2014.
 * @see [3] Zaunschirm M, Scho"rkhuber C, Ho"ldrich R. Binaural rendering of
 *          Ambisonic signals by head-related impulse response time alignment
 *          and a diffuseness constraint. The Journal of the Acoustical Society
 *          of America. 2018 Jun 19;143(6):3616-27
 * @see [4] Scho"rkhuber C, Zaunschirm M, Ho"ldrich R. Binaural Rendering of
 *          Ambisonic Signals via Magnitude Least Squares. InProceedings of the
 *          DAGA 2018 (Vol. 44, pp. 339-342).
 */
typedef enum _BINAURAL_AMBI_DECODER_METHODS {
    /**
     * The default decoder is "BINAURAL_DECODER_LS"
     */
    BINAURAL_DECODER_DEFAULT,
    /**
     * Least-squares (LS) decoder. The simplest binaural decoder.
     */
    BINAURAL_DECODER_LS,
    /**
     * Least-squares (LS) decoder with diffuse-field spectral equalisation [1]
     */
    BINAURAL_DECODER_LSDIFFEQ,
    /**
     * Spatial resampling decoder (on the same lines as the virtual loudspeaker
     * approach) [2]
     */
    BINAURAL_DECODER_SPR,
    /**
     * Time-alignment decoder [3]. Relies on discarding the phase information of
     * the HRTFs, past the frequency at which humans are less sensitive to
     * inter-aural time differences. Therefore, the least-squares fitting
     * priorites matching the interaural level differences (ILDs), rather than
     * the interaural time differences (ITDs).
     */
    BINAURAL_DECODER_TA,
    /**
     * Magnitude least-squares decoder [4]. On similar lines to the time-
     * alignment decoder, but differing in its execution.
     */
    BINAURAL_DECODER_MAGLS
    
} BINAURAL_AMBI_DECODER_METHODS;


/* ========================================================================== */
/*                               Main Functions                               */
/* ========================================================================== */

/**
 * Computes REAL spherical harmonics [1] for multiple directions on the sphere
 *
 * The real spherical harmonics are computed WITHOUT the 1/sqrt(4*pi) term.
 * i.e. max(omni) = 1. Also, compared to getRSH_recur(), this function uses
 * unnorm_legendreP() and double precision, so is more suitable for determining
 * 'Y' in an initialisation stage. This version is indeed slower, but more
 * precise; especially for high orders.
 *
 * @note This function is mainly intended for Ambisonics, due to the omission of
 *       the 1/sqrt(4*pi) scaling, and the directions are given in
 *       [azimuth elevation] (degrees). In Ambisonics literature, the format
 *       convention of 'Y' is referred to as ACN/N3D
 *
 * @param[in]  order    Order of spherical harmonic expansion
 * @param[in]  dirs_deg Directions on the sphere [azi, ELEVATION] convention, in
 *                      DEGREES; FLAT: nDirs x 2
 * @param[in]  nDirs    Number of directions
 * @param[out] Y        The SH weights [WITHOUT the 1/sqrt(4*pi)];
 *                      FLAT: (order+1)^2 x nDirs
 *
 * @see [1] Rafaely, B. (2015). Fundamentals of spherical array processing
 *          (Vol. 8). Berlin: Springer.
 */
void getRSH(/* Input Arguments */
            int order,
            float* dirs_deg,
            int nDirs,
            /* Output Arguments */
            float* Y);

/**
 * Computes REAL spherical harmonics [1] for multiple directions on the sphere
 *
 * The real spherical harmonics are computed WITHOUT the 1/sqrt(4*pi) term.
 * i.e. max(omni) = 1. Also, Compared to getRSH(), this function uses
 * unnorm_legendreP_recur() and single precision, so is more suitable for
 * determining 'Y' in a real-time loop. It sacrifices some precision, as
 * numerical error propogates through the recursion, but it is faster.
 *
 * @note This function is mainly intended for Ambisonics, due to the omission of
 *       the 1/sqrt(4*pi) scaling, and the directions are given in
 *       [azimuth elevation] (degrees). In Ambisonics literature, the format
 *       convention of 'Y' is referred to as ACN/N3D
 *
 * @param[in]  order    Order of spherical harmonic expansion
 * @param[in]  dirs_deg Directions on the sphere [azi, ELEVATION] convention, in
 *                      DEGREES; FLAT: nDirs x 2
 * @param[in]  nDirs    Number of directions
 * @param[out] Y        The SH weights [WITHOUT the 1/sqrt(4*pi)];
 *                      FLAT: (order+1)^2 x nDirs
 *
 * @see [1] Rafaely, B. (2015). Fundamentals of spherical array processing
 *          (Vol. 8). Berlin: Springer.
 */
void getRSH_recur(/* Input Arguments */
                  int order,
                  float* dirs_deg,
                  int nDirs,
                  /* Output Arguments */
                  float* Y);

/**
 * Computes the weights required to manipulate a hyper-cardioid beam-pattern,
 * such that it has maximum energy in the given look-direction
 *
 * Traditionally, due to the back lobes of beamformers when panning a source
 * via Ambisonics encoding/decoding, there is unwanted energy given to
 * loudspeakers directly opposite the true source direction. This max_rE
 * weighting [1] essentially spatially "tapers" the spherical harmonic patterns
 * used to generate said beams, reducing the contribution of the higher
 * orders to the beam patterns. This results in worse spatial selectivity, as
 * the width of the beam pattern main lobe is widened, however, the back lobes
 * are also reduced; thus mitigating the aforementioned problem.
 *
 * @param[in]  order       Order of spherical harmonic expansion
 * @param[in]  diagMtxFlag Set to '0' if you want the weights to be returned as
 *                         a vector, or to '1' as a diagonal matrix instead.
 * @param[out] a_n         The max_rE weights, as a vector/diagonal matrix;
 *                         (order+1)^2 x 1 OR FLAT: (order+1)^2 x (order+1)^2
 *
 * @see [1] Zotter F, Frank M. All-round ambisonic panning and decoding. Journal
 *          of the audio engineering society. 2012 Nov 26; 60(10):807-20.
 */
void getMaxREweights(/* Input Arguments */
                     int order,
                     int diagMtxFlag,
                     /* Output Arguments */
                     float* a_n);

/**
 * Computes an ambisonic decoding matrix of a specific order, for a specific
 * loudspeaker layout
 *
 * @param[in]  ls_dirs_deg Loudspeaker directions in DEGREES [azi elev];
 *                         FLAT: nLS x 2
 * @param[in]  nLS         Number of loudspeakers
 * @param[in]  method      Decoding method
 *                         (see "LOUDSPEAKER_AMBI_DECODER_METHODS" enum)
 * @param[in]  order       Decoding order
 * @param[in]  enableMaxrE Set to '0' to disable, '1' to enable
 * @param[out] decMtx      Decoding matrix; FLAT: nLS x (order+1)^2
 */
void getLoudspeakerAmbiDecoderMtx(/* Input Arguments */
                                  float* ls_dirs_deg,
                                  int nLS,
                                  LOUDSPEAKER_AMBI_DECODER_METHODS method,
                                  int order,
                                  int enableMaxrE,
                                  /* Output Arguments */
                                  float* decMtx);

/**
 * Computes binaural ambisonic decoding matrices (one per frequency) at a
 * specific order, for a given HRTF set
 *
 * @param[in]  hrtfs         The HRTFs; FLAT: N_bands x NUM_EARS x N_dirs
 * @param[in]  hrtf_dirs_deg HRTF directions; FLAT: N_dirs x 2
 * @param[in]  N_dirs        Number of HRTF directions
 * @param[in]  N_bands       Number of frequency bands/bins
 * @param[in]  method        Decoding method (see BINAURAL_AMBI_DECODER_METHODS
 *                           enum)
 * @param[in]  order         Decoding order
 * @param[in]  freqVector    Only needed for BINAURAL_DECODER_TA or
 *                           BINAURAL_DECODER_MAGLS decoders (set to NULL if
 *                           using a different method); N_bands x 1
 * @param[in]  itd_s         Only needed for BINAURAL_DECODER_TA decoder (set
 *                           to NULL if using different method); N_dirs x 1
 * @param[in]  weights       Integration weights (set to NULL if not available);
 *                           N_dirs x 1
 * @param[in]  enableDiffCM  Set to '0' to disable diffuse correction, '1' to
 *                           enable
 * @param[in]  enableMaxrE   Set to '0' to disable maxRE weighting, '1' to
 *                           enable
 * @param[out] decMtx        Decoding matrices (one per frequency);
 *                           FLAT: N_bands x NUM_EARS x (order+1)^2
 */
void getBinauralAmbiDecoderMtx(/* Input Arguments */
                               float_complex* hrtfs,
                               float* hrtf_dirs_deg,
                               int N_dirs,
                               int N_bands,
                               BINAURAL_AMBI_DECODER_METHODS method,
                               int order,
                               float* freqVector,
                               float* itd_s,
                               float* weights,
                               int enableDiffCM,
                               int enableMaxrE,
                               /* Output Arguments */
                               float_complex* decMtx);

/**
 * Computes ambisonic decoding filters for a given HRTF set
 *
 * @param[in]  hrtfs         The HRTFs; FLAT: (fftSize/2+1) x NUM_EARS x N_dirs
 * @param[in]  hrtf_dirs_deg HRTF directions; FLAT: N_dirs x 2
 * @param[in]  N_dirs        Number of HRTF directions
 * @param[in]  fftSize       FFT size
 * @param[in]  fs            Sampling rate
 * @param[in]  method        Decoding method (see BINAURAL_AMBI_DECODER_METHODS
 *                           enum)
 * @param[in]  order         Decoding order
 * @param[in]  itd_s         Only needed for BINAURAL_DECODER_TA decoder (can
 *                           set to NULL if using different method); N_dirs x 1
 * @param[in]  weights       Integration weights (set to NULL if not available);
 *                           N_dirs x 1
 * @param[in]  enableDiffCM  Set to '0' to disable diffuse correction, '1' to
 *                           enable
 * @param[in]  enableMaxrE   Set to '0' to disable maxRE weighting, '1' to
 *                           enable
 * @param[out] decFilters    Decoding filters;
 *                           FLAT: NUM_EARS x (order+1)^2 x fftSize
 */
void getBinauralAmbiDecoderFilters(/* Input Arguments */
                                   float_complex* hrtfs,
                                   float* hrtf_dirs_deg,
                                   int N_dirs,
                                   int fftSize,
                                   float fs,
                                   BINAURAL_AMBI_DECODER_METHODS method,
                                   int order,
                                   float* itd_s,
                                   float* weights,
                                   int enableDiffCM,
                                   int enableMaxrE,
                                   /* Output Arguments */
                                   float* decFilters);

/**
 * Imposes a diffuse-field covariance constraint on a given binaural decoding
 * matrix [1]
 *
 * @note decMtx is altered in-place.
 *
 * @param[in]     hrtfs         The HRTFs; FLAT: N_bands x NUM_EARS x N_dirs
 * @param[in]     hrtf_dirs_deg HRTF directions; FLAT: N_dirs x 2
 * @param[in]     N_dirs        Number of HRTF directions
 * @param[in]     N_bands       Number of frequency bands/bins
 * @param[in]     order         Decoding order
 * @param[in]     weights       Integration weights (set to NULL if not
 *                              available); N_dirs x 1
 * @param[in,out] decMtx        Decoding matrix;
 *                              FLAT: N_bands x NUM_EARS x (order+1)^2
 *
 * @see [1] Zaunschirm M, Scho"rkhuber C, Ho"ldrich R. Binaural rendering of
 *          Ambisonic signals by head-related impulse response time alignment
 *          and a diffuseness constraint. The Journal of the Acoustical Society
 *          of America. 2018 Jun 19;143(6):3616-27
 */
void applyDiffCovMatching(/* Input Arguments */
                          float_complex* hrtfs,
                          float* hrtf_dirs_deg,
                          int N_dirs,
                          int N_bands,
                          int order,
                          float* weights,
                          /* Input/Output Arguments */
                          float_complex* decMtx);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __SAF_HOA_H_INCLUDED__ */
