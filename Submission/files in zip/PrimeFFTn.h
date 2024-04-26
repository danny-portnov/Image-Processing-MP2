#pragma once

typedef int    tInteger;  // 32 or 64 ???
typedef double tFloat;    // Beware of Multiple redefinitions - no warning

#define FORWARD_FFT_WITH_EXP_PLUS_1  +1
#define REVERSE_FFT_WITH_EXP_MINUS_1  -1

#define FORWARD_FFT  +1
#define REVERSE_FFT  -1

#define NORMALIZE_RESULT_BY_TOTAL_DIMENSION_OF_THE_TRANSFORM -1
#define NORMALIZE_RESULT_BY_SQRT_OF_TOTAL_DIMENSION_OF_THE_TRANSFORM -2

#define NO_NORMALIZATION 0
#define NORMALIZE_BY_NUMBER -1
#define NORMALIZE_BY_SQRT   -2

void fft_free (void);

void PrimeFFTn (tInteger   numberOfDimensions,
                tInteger * arrayOfDimensions,
                tFloat   * reData,
                tFloat   * imData,
	            tInteger exponentSign,
                tInteger normalizationScalingType);
