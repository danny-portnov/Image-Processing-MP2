#pragma once

#include "PrimeFFTn.h"

#include "c2dFloatGrayImage.h"

void DoFFT( c2dFloatGrayImage & realPart, 
            c2dFloatGrayImage & imagPart, 
            int exponentSign = FORWARD_FFT, 
            int normalizationScalingType = NORMALIZE_BY_SQRT ); 
 
void ShiftHalfSize( c2dFloatGrayImage & image); 
 
void Magnitude( c2dFloatGrayImage & result, 
                c2dFloatGrayImage & realPart, 
                c2dFloatGrayImage & imagPart ); 

void DoFiltrationInFD( c2dFloatGrayImage & realPartInFD,
                       c2dFloatGrayImage & imagPartInFD,
                       c2dFloatGrayImage & filterInFD );

bool SizesAreEqual( c2dFloatGrayImage & first,
				    c2dFloatGrayImage & second );
