/*
 * Copyright 2019 Leo McCormack
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
 * @file dirass.h
 * @brief A sound-field visualiser based on the directional re-assignment of
 *        beamformer energy, utilising the DoA estimates extracted from
 *        spatially-localised active-intensity (SLAI) vectors; which are centred
 *        around each of the corresponding scanning grid directions [1,2].
 *
 * @see [1] McCormack, L., Politis, A., and Pulkki, V. (2019). "Sharpening of
 *          angular spectra based on a directional re-assignment approach for
 *          ambisonic sound-field visualisation". IEEE International Conference
 *          on Acoustics, Speech and Signal Processing (ICASSP).
 * @see [2] McCormack, L., Delikaris-Manias, S., Politis, A., Pavlidi, D.,
 *          Farina, A., Pinardi, D. and Pulkki, V., 2019. Applications of
 *          Spatially Localized Active-Intensity Vectors for Sound-Field
 *          Visualization. Journal of the Audio Engineering Society, 67(11),
 *          pp.840-854.
 *
 * @author Leo McCormack
 * @date 21.02.2019
 */

#ifndef __DIRASS_H_INCLUDED__
#define __DIRASS_H_INCLUDED__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ========================================================================== */
/*                             Presets + Constants                            */
/* ========================================================================== */
 
#define DIRASS_MAX_NUM_INPUT_CHANNELS ( 64 )
    
/**
 * Available analysis orders.
 */
typedef enum _DIRASS_INPUT_ORDERS{
    INPUT_ORDER_FIRST = 1, /**< First-order analysis (4 channel input) */
    INPUT_ORDER_SECOND,    /**< Second-order analysis (9 channel input) */
    INPUT_ORDER_THIRD,     /**< Third-order analysis (16 channel input) */
    INPUT_ORDER_FOURTH,    /**< Fourth-order analysis (25 channel input) */
    INPUT_ORDER_FIFTH,     /**< Fifth-order analysis (36 channel input) */
    INPUT_ORDER_SIXTH,     /**< Sixth-order analysis (49 channel input) */
    INPUT_ORDER_SEVENTH    /**< Seventh-order analysis (64 channel input) */
    
}DIRASS_INPUT_ORDERS;
    
/**
 * Available upscaling orders.
 */
typedef enum _DIRASS_UPSCALE_ORDERS{
    UPSCALE_ORDER_FIRST = 1,   /**< First-order upscaling */
    UPSCALE_ORDER_SECOND,      /**< Second-order upscaling */
    UPSCALE_ORDER_THIRD,       /**< Third-order upscaling */
    UPSCALE_ORDER_FOURTH,      /**< Fourth-order upscaling */
    UPSCALE_ORDER_FIFTH,       /**< Fifth-order upscaling */
    UPSCALE_ORDER_SIXTH,       /**< Sixth-order upscaling */
    UPSCALE_ORDER_SEVENTH,     /**< Seventh-order upscaling */
    UPSCALE_ORDER_EIGHTH,      /**< Eighth-order upscaling */
    UPSCALE_ORDER_NINTH,       /**< Ninth-order upscaling */
    UPSCALE_ORDER_TENTH,       /**< Tenth-order upscaling */
    UPSCALE_ORDER_ELEVENTH,    /**< Eleventh-order upscaling */
    UPSCALE_ORDER_TWELFTH,     /**< Twelfth-order upscaling */
    UPSCALE_ORDER_THIRTEENTH,  /**< Thirteenth-order upscaling */
    UPSCALE_ORDER_FOURTEENTH,  /**< Fourteenth-order upscaling */
    UPSCALE_ORDER_FIFTEENTH,   /**< Fifteenth-order upscaling */
    UPSCALE_ORDER_SIXTHTEENTH, /**< Sixthteenth-order upscaling */
    UPSCALE_ORDER_SEVENTEENTH, /**< Seventeenth-order upscaling */
    UPSCALE_ORDER_EIGHTEENTH,  /**< Eighteenth-order upscaling */
    UPSCALE_ORDER_NINETEENTH,  /**< Ninteenth-order upscaling */
    UPSCALE_ORDER_TWENTIETH    /**< Twentieth-order upscaling */
    
}DIRASS_UPSCALE_ORDERS;
   
/**
 * Available scanning grid options
 */
typedef enum _DIRASS_GRID_OPTIONS {
    T_DESIGN_3 = 1,    /**< T_DESIGN_3        - 6 points */
    T_DESIGN_4,        /**< T_DESIGN_4        - 12 points */
    T_DESIGN_6,        /**< T_DESIGN_6        - 24 points */
    T_DESIGN_9,        /**< T_DESIGN_9        - 48 points */
    T_DESIGN_13,       /**< T_DESIGN_13       - 94 points */
    T_DESIGN_18,       /**< T_DESIGN_18       - 180 points */
    GRID_GEOSPHERE_6,  /**< GRID_GEOSPHERE_6  - 362 points */
    T_DESIGN_30,       /**< T_DESIGN_30       - 480 points */
    GRID_GEOSPHERE_8,  /**< GRID_GEOSPHERE_8  - 642 points */
    GRID_GEOSPHERE_9,  /**< GRID_GEOSPHERE_9  - 812 points */
    GRID_GEOSPHERE_10, /**< GRID_GEOSPHERE_10 - 1002 points */
    GRID_GEOSPHERE_12  /**< GRID_GEOSPHERE_12 - 1442 points */
    
}DIRASS_GRID_OPTIONS;

/**
 * Available Ambisonic channel ordering conventions
 *
 * @note CH_FUMA only supported for 1st order input.
 */
typedef enum _DIRASS_CH_ORDER {
    CH_ACN = 1, /**< Ambisonic Channel Numbering (ACN) */
    CH_FUMA     /**< (Obsolete) Furse-Malham/B-format (WXYZ) */
    
} DIRASS_CH_ORDER;
 
/**
 * Available Ambisonic normalisation conventions
 *
 * @note NORM_FUMA only supported for 1st order input and does NOT have the
 *       1/sqrt(2) scaling on the omni.
 */
typedef enum _DIRASS_NORM_TYPES {
    NORM_N3D = 1, /**< orthonormalised (N3D) */
    NORM_SN3D,    /**< Schmidt semi-normalisation (SN3D) */
    NORM_FUMA     /**< (Obsolete) Same as NORM_SN3D for 1st order */
    
} DIRASS_NORM_TYPES;
  
/**
 * Available sector beamforming patterns
 */
typedef enum _DIRASS_BEAM_TYPES {
    BEAM_TYPE_CARD = 1,  /**< Cardioid */
    BEAM_TYPE_HYPERCARD, /**< Hyper-cardioid */
    BEAM_TYPE_MAX_EV     /**< Hyper-cardioid with max_rE weighting */
    
} DIRASS_BEAM_TYPES;

/**
 * Available processing modes. More information can be found in [1]
 *
 * @see [1] McCormack, L., Politis, A., and Pulkki, V. (2019). "Sharpening of
 *          angular spectra based on a directional re-assignment approach for
 *          ambisonic sound-field visualisation". IEEE International Conference
 *          on Acoustics, Speech and Signal Processing (ICASSP).
 */
typedef enum _DIRASS_REASS_MODE {
    REASS_MODE_OFF = 1, /**< Re-assignment is disabled. i.e. dirass generates a
                         *   standard (beamformer)energy-based map */
    REASS_NEAREST,      /**< Each sector beamformer energy is re-assigned to the
                         *   nearest interpolation grid point, based on the
                         *   analysed DoA */
    REASS_UPSCALE       /**< Each sector beamformer is re-encoded into spherical
                         *   harmonics of a higher order. The map is then
                         *   derived from the upscaled SHs as normal. */
    
} DIRASS_REASS_MODES;
    
/**
 * Available horizontal feild-of-view (FOV) options
 */
typedef enum _DIRASS_HFOV_OPTIONS{
    HFOV_360 = 1, /**< 360 degrees */
    HFOV_180,     /**< 180 degrees */
    HFOV_90,      /**< 90 degrees */
    HFOV_60       /**< 60 degrees */
    
}DIRASS_HFOV_OPTIONS;
    
/**
 * Available aspect ratios
 */
typedef enum _DIRASS_ASPECT_RATIO_OPTIONS{
    ASPECT_RATIO_2_1 = 1, /**< ASPECT_RATIO_2_1  - 2:1 */
    ASPECT_RATIO_16_9,    /**< ASPECT_RATIO_16_9 - 16:9 */
    ASPECT_RATIO_4_3      /**< ASPECT_RATIO_4_3  - 4:3 */

}DIRASS_ASPECT_RATIO_OPTIONS;
    
/**
 * Current status of the codec.
 */
typedef enum _DIRASS_CODEC_STATUS{
    CODEC_STATUS_INITIALISED = 0, /**< Codec is initialised and ready to process
                                   *   input audio. */
    CODEC_STATUS_NOT_INITIALISED, /**< Codec has not yet been initialised, or
                                   *   the codec configuration has changed.
                                   *   Input audio should not be processed. */
    CODEC_STATUS_INITIALISING     /**< Codec is currently being initialised,
                                   *   input audio should not be processed. */
}DIRASS_CODEC_STATUS;

#define DIRASS_PROGRESSBARTEXT_CHAR_LENGTH 256
    

/* ========================================================================== */
/*                               Main Functions                               */
/* ========================================================================== */

/**
 * Creates an instance of the dirass
 *
 * @param[in] phDir (&) address of dirass handle
 */
void dirass_create(void** const phDir);

/**
 * Destroys an instance of the dirass
 *
 * @param[in] phDir (&) address of dirass handle
 */
void dirass_destroy(void** const phDir);

/**
 * Initialises an instance of dirass with default settings
 *
 * @param[in] hDir       dirass handle
 * @param[in] samplerate Host samplerate.
 */
void dirass_init(void* const hDir,
                 float  samplerate);
    
/**
 * Intialises the codec variables, based on current global/user parameters
 *
 * @param[in] hDir dirass handle
 */
void dirass_initCodec(void* const hDir);

/**
 * Analyses the input spherical harmonic signals to generate an activity-map as
 * in [1]
 *
 * @param[in] hDir      dirass handle
 * @param[in] inputs    Input channel buffers; 2-D array: nInputs x nSamples
 * @param[in] nInputs   Number of input channels
 * @param[in] nSamples  Number of samples in 'inputs'/'output' matrices
 * @param[in] isPlaying Flag to indicate if there is audio in the input buffers,
 *                      0: no audio, reduced processing, 1: audio, full
 *                      processing
 *
 * @see [1] McCormack, L., Politis, A., and Pulkki, V. (2019). "Sharpening of
 *          angular spectra based on a directional re-assignment approach for
 *          ambisonic sound-field visualisation". IEEE International Conference
 *          on Acoustics, Speech and Signal Processing (ICASSP).
 */
void dirass_analysis(void* const hDir,
                     float** const inputs,
                     int nInputs,
                     int nSamples,
                     int isPlaying);
    
   
/* ========================================================================== */
/*                                Set Functions                               */
/* ========================================================================== */

/**
 * Sets all intialisation flags to 1; re-initialising all settings/variables
 * as dirass is currently configured, at next available opportunity.
 */
void dirass_refreshSettings(void* const hDir);
    
/**
 * Sets the sector beamforming pattern to employ for the analysis (see
 * DIRASS_BEAM_TYPE enum).
 */
void dirass_setBeamType(void* const hDir, int newType);
    
/**
 * Sets the input/analysis order (see 'DIRASS_INPUT_ORDERS' enum)
 */
void dirass_setInputOrder(void* const hDir,  int newValue);
    
/**
 * Sets a new display grid option (see DIRASS_GRID_OPTIONS enum)
 *
 * @note Not safe to call while simultaneously calling dirass_analysis()!
 */
void dirass_setDisplayGridOption(void* const hDir,  int newOption);
   
/**
 * Sets the output display width in pixels
 *
 * @note Not safe to call while simultaneously calling dirass_analysis()!
 */
void dirass_setDispWidth(void* const hDir,  int newValue);
    
/**
 * Sets the upscale order, if DIRASS_REASS_MODE is set to REASS_UPSCALE,
 * (see UPSCALE_ORDER enum).
 */
void dirass_setUpscaleOrder(void* const hDir,  int newOrder);
    
/**
 * Sets the analysis directional re-assignment mode (see DIRASS_REASS_MODE enum)
 */
void dirass_setDiRAssMode(void* const hDir,  int newMode);
    
/**
 * Sets the minimum analysis frequency, in Hz
 */
void dirass_setMinFreq(void* const hDir,  float newValue);

/**
 * Sets the maximum analysis frequency, in Hz
 */
void dirass_setMaxFreq(void* const hDir,  float newValue);

/**
 * Sets the Ambisonic channel ordering convention to decode with, in order to
 * match the convention employed by the input signals (see DIRASS_CH_ORDER enum)
 */
void dirass_setChOrder(void* const hDir, int newOrder);

/**
 * Sets the Ambisonic normalisation convention to decode with, in order to match
 * with the convention employed by the input signals (see 'DIRASS_NORM_TYPE'
 * enum)
 */
void dirass_setNormType(void* const hDir, int newType);

/**
 * Sets the visualisation display window horizontal field-of-view (FOV)
 * (see 'DIRASS_HFOV_OPTIONS' enum)
 */
void dirass_setDispFOV(void* const hDir, int newOption);
    
/**
 * Sets the visualisation display window aspect-ratio (see
 * 'DIRASS_ASPECT_RATIO_OPTIONS' enum)
 */
void dirass_setAspectRatio(void* const hDir, int newOption);
    
/**
 * Sets the activity-map averaging coefficient, 0..1
 */
void dirass_setMapAvgCoeff(void* const hDir, float newValue);
    
/**
 * Informs dirass that it should compute a new activity-map
 */
void dirass_requestPmapUpdate(void* const hDir);
    

/* ========================================================================== */
/*                                Get Functions                               */
/* ========================================================================== */

/**
 * Returns current codec status (see 'DIRASS_CODEC_STATUS' enum)
 */
DIRASS_CODEC_STATUS dirass_getCodecStatus(void* const hDir);

/**
 * (Optional) Returns current intialisation/processing progress, between 0..1
 * - 0: intialisation/processing has started
 * - 1: intialisation/processing has ended
 */
float dirass_getProgressBar0_1(void* const hDir);

/**
 * (Optional) Returns current intialisation/processing progress text
 *
 * @note "text" string should be (at least) of length:
 *       DIRASS_PROGRESSBARTEXT_CHAR_LENGTH
 *
 * @param[in]  hDir dirass handle
 * @param[out] text Process bar text; DIRASS_PROGRESSBARTEXT_CHAR_LENGTH x 1
 */
void dirass_getProgressBarText(void* const hDir, char* text);

/**
 * Returns the current analysis/input order (see 'DIRASS_INPUT_ORDERS' enum)
 */
int dirass_getInputOrder(void* const hDir);
    
/**
 * Returns the sector beamforming pattern to employed for the analysis (see
 * DIRASS_BEAM_TYPE enum).
 */
int dirass_getBeamType(void* const hDir);

/**
 * Returns the current display grid option (see DIRASS_GRID_OPTIONS enum)
 */
int dirass_getDisplayGridOption(void* const hDir);

/**
 * Returns the current output display width in pixels
 */
int dirass_getDispWidth(void* const hDir);

/**
 * Returns the current upscale order (see DIRASS_UPSCALE_ORDER enum).
 */
int dirass_getUpscaleOrder(void* const hDir);

/**
 * Returns the current analysis directional re-assignment mode (see
 * DIRASS_REASS_MODE enum)
 */
int dirass_getDiRAssMode(void* const hDir); 

/**
 * Returns the current minimum analysis frequency, in Hz
 */
float dirass_getMinFreq(void* const hDir);

/**
 * Returns the current maximum analysis frequency, in Hz
 */
float dirass_getMaxFreq(void* const hDir);

/**
 * Returns the current sampling rate, in Hz
 */
int dirass_getSamplingRate(void* const hDir); 
    
/**
 * Returns the number of spherical harmonic signals required by the current
 * analysis order: (current_order + 1)^2
 */
int dirass_getNSHrequired(void* const hDir);

/**
 * Returns the Ambisonic channel ordering convention currently being used to
 * decode with, which should match the convention employed by the input signals
 * (see 'DIRASS_CH_ORDER' enum)
 */
int dirass_getChOrder(void* const hDir);

/**
 * Returns the Ambisonic normalisation convention currently being usedto decode
 * with, which should match the convention employed by the input signals (see
 * 'DIRASS_NORM_TYPE' enum)
 */
int dirass_getNormType(void* const hDir);

/**
 * Returns the current visualisation display window horizontal field-of-view
 * (FOV) (see 'DIRASS_HFOV_OPTIONS' enum)
 */
int dirass_getDispFOV(void* const hDir);

/**
 * Function: dirass_getAspectRatio
 * -------------------------------
 * Returns the current visualisation display window aspect-ratio (see
 * 'DIRASS_ASPECT_RATIO_OPTIONS' enum)
 */
int dirass_getAspectRatio(void* const hDir);

/**
 * Returns the current activity-map averaging coefficient, 0..1
 */
float dirass_getMapAvgCoeff(void* const hDir);
    
/**
 * Returns the latest computed activity-map if it is ready; otherwise it returns
 * 0, and you'll just have to wait a bit  
 *
 * @param[in]  hDir        (&) dirass handle
 * @param[out] grid_dirs   (&) scanning grid directions, in DEGREES; nDirs x 1
 * @param[out] pmap        (&) activity-map values; nDirs x 1
 * @param[out] nDirs       (&) number of directions
 * @param[out] pmapWidth   (&) activity-map width in pixels
 * @param[out] hfov        (&) horizontal FOV used to generate activity-map
 * @param[out] aspectRatio (&) aspect ratio used to generate activity-map
 * @returns                flag, if activity-map is ready, 1: it is, 0: it is
 *                         NOT
 */
int dirass_getPmap(void* const hDir,
                   float** grid_dirs,
                   float** pmap,
                   int* nDirs,
                   int* pmapWidth,
                   int* hfov,
                   float* aspectRatio);


#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* __DIRASS_H_INCLUDED__ */
