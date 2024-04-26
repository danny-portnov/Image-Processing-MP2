
#define _USE_MATH_DEFINES 
#include <math.h>


#include "FFT.h"

void DoFFT( c2dFloatGrayImage & realPart, 
            c2dFloatGrayImage & imagPart, 
            int exponentSign, 
            int normalizationScalingType) 
{ 
  if (SizesAreEqual( realPart, imagPart) == false )
  {
	  throw
		"Error in DoFFT:\nSizes of Real and Imag parts are not equal";
  }

  tFloat * ptrToRe; 
  ptrToRe = (tFloat *) realPart.ScanLine(); 
 
  tFloat * ptrToIm; 
  ptrToIm = (tFloat *) imagPart.ScanLine(); 
 
  tInteger arrayOfDimensions[2]; 
  arrayOfDimensions[0] = realPart.NumberOfColumns(); 
  arrayOfDimensions[1] = realPart.NumberOfRows(); 
   
  try 
  { 
    PrimeFFTn( 2, 
                   arrayOfDimensions, 
                   ptrToRe, 
                   ptrToIm, 
                   exponentSign, 
                   normalizationScalingType ); 
   } 
   catch ( ... ) 
   { 
     // error in FFT 
     throw "Error in FFT";   
   } 
 
} 
 
void ShiftHalfSize( c2dFloatGrayImage & image) 
{ 
  tFloat * ptrToPixels; 
  ptrToPixels = image.ScanLine(); 
  
  int row, column;

  // 180 for col 
  for ( row = 0; 
        row < image.NumberOfRows(); 
        row++ ) 
  {    
    ptrToPixels = image.ScanLine(row);
    for ( column=0; 
          column < image.NumberOfColumns(); 
          column +=2) 
    { 
      *ptrToPixels  *= (-1); // change sign 
       ptrToPixels += 2;     // increment pointer 
    } 
  } 
 
  // 180 for row 
  for ( row = 0; 
        row < image.NumberOfRows(); 
        row +=2 ) 
  { 
	ptrToPixels = image.ScanLine( row );     
    for ( column=0; 
          column < image.NumberOfColumns(); 
          column++) 
    { 
      *ptrToPixels++  *= (-1); // change sign and increment pointer 
    } 
  } 
 
} 
 
void Magnitude( c2dFloatGrayImage & result, 
                c2dFloatGrayImage & realPart, 
                c2dFloatGrayImage & imagPart ) 
{ 
  if (SizesAreEqual( realPart, imagPart) == false )
  {
	  throw
		"Error in magnitude:\nSizes of Real and Imag parts are not equal";
  }

  tFloat * ptrToRe; 
  ptrToRe = realPart.ScanLine(); 
 
  tFloat * ptrToIm; 
  ptrToIm = imagPart.ScanLine(); 
   
  result.Init ( realPart.NumberOfRows(), realPart.NumberOfColumns() ); 
 
  tFloat * ptrToResult; 
  ptrToResult = result.ScanLine(); 
 
 
  tFloat a,b; 
  for ( int pixel=0; pixel< result.NumberOfPixels(); pixel++ ) 
  { 
    a = *ptrToRe++; 
    b = *ptrToIm++; 
    *ptrToResult++  = sqrt( a*a + b*b); 
  } 
 }

void DoFiltrationInFD( c2dFloatGrayImage & realPartInFD,
                       c2dFloatGrayImage & imagPartInFD,
                       c2dFloatGrayImage & filterInFD )
{
  if (SizesAreEqual( realPartInFD, imagPartInFD) == false )
  {
	  throw
		"Error in DoFiltrationInFD:\nSizes of Real and Imag parts are not equal";
  }

  if (SizesAreEqual( realPartInFD, filterInFD) == false )
  {
	  throw
		"Error in DoFiltrationInFD:\nSizes of Image and Filter are not equal";
  }
  tFloat * ptrToReal;
  tFloat * ptrToImag;
  tFloat * ptrToFilter;

  ptrToReal = realPartInFD.ScanLine();
  ptrToImag = imagPartInFD.ScanLine();
  ptrToFilter = filterInFD.ScanLine();

  for ( int pixel=0; pixel < filterInFD.NumberOfPixels(); pixel++)
  {
	 *ptrToReal++ = (*ptrToReal) * (*ptrToFilter);
     *ptrToImag++ = (*ptrToImag) * (*ptrToFilter++);
  }
}

bool SizesAreEqual( c2dFloatGrayImage & first,
				    c2dFloatGrayImage & second )
{
	if ( first.NumberOfColumns() != second.NumberOfColumns() )
	{
		return false;
	}

	if ( first.NumberOfRows() != second.NumberOfRows() )
	{
		return false;
	}
	return true;
}
 

 

