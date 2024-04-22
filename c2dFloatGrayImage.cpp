#include "c2dFloatGrayImage.h"

// the following is needed for cin and cout instead of printf
#include <iostream>
using namespace std;  // explain someday

#include  <stdio.h>   // for sprintf_s instead of sprintf
#include  <memory.h>  // for memset

char ferrMessage[128]; // Very bad C idea: local-global variable
                       // but code is more compact:
                       //    no need for multiple declarations
                       // Option was to iclude ferrMessage in the class,
                       //    but I do not want to spoil it for now...


c2dFloatGrayImage::c2dFloatGrayImage(void)
{
	ZeroFields();
	Init( 1,1 ); // At least one pixel  
}

	
c2dFloatGrayImage::c2dFloatGrayImage( int numOfRows, int numOfColumns)
{
	ZeroFields();
	Init( numOfRows,numOfColumns );
}

c2dFloatGrayImage::c2dFloatGrayImage( c2dFloatGrayImage & other)
{
	ZeroFields();
	Init( other );
}

c2dFloatGrayImage::c2dFloatGrayImage( c2dByteGrayImage & other )
{
	ZeroFields();
	Init( other );
}

c2dFloatGrayImage::c2dFloatGrayImage( const char * bmpFileName)
{
	ZeroFields();
	LoadFromBmpFile( bmpFileName );
}

c2dFloatGrayImage::~c2dFloatGrayImage(void)
{
	Kill();
}


void c2dFloatGrayImage::Kill()
{
	if ( ptrToPixels != 0 )
	{
		delete [] ptrToPixels;
        ZeroFields();
	}
}

void c2dFloatGrayImage::ZeroFields()
{
	ptrToPixels = 0;
	numberOfRows = 0;
	numberOfColumns = 0;
}


void c2dFloatGrayImage::Init( int numOfRows, int numOfColumns)
{
	if (
		 ( numOfRows == numberOfRows )
		     &&
         ( numOfColumns == numberOfColumns) )
	 {
		 return;  // be smart, no init needed in this case
	 }
     
     if ( numOfRows <= 0 )
	 {
       sprintf_s( ferrMessage, "Negative or zero number of rows = %d requested",
		          numOfRows);
	   throw ferrMessage; 
	 }

     if ( numOfColumns <= 0 )
	 {
	   sprintf_s( ferrMessage, "Negative or zero number of columns = %d requested", 
		          numOfColumns);
	   throw ferrMessage; 
	 }

	 Kill();

	 numberOfRows    = numOfRows;
	 numberOfColumns = numOfColumns;

	 ptrToPixels = new tFloat [NumberOfPixels()];
	 
	 if ( ptrToPixels == 0 ) // allocation failed
	 {
       sprintf_s(ferrMessage, "Can not allocate %d bytes", NumberOfPixels() );
	   throw ferrMessage;
	 }
	 Clear(0); // takes time, but to be sure...
}

void c2dFloatGrayImage::Init( c2dFloatGrayImage & other)
{
	Init( other.NumberOfRows(), other.NumberOfColumns() );

	tFloat * ptrToThisPixels;
	tFloat * ptrToOtherPixels;

    ptrToThisPixels  = ScanLine(0);
	ptrToOtherPixels = other.ScanLine(0);

	for ( int i=0; i<NumberOfPixels(); i++)
	{
      *ptrToThisPixels++ = *ptrToOtherPixels++;
	}
}
void c2dFloatGrayImage::Init( c2dByteGrayImage  & other)
{
	Init( other.NumberOfRows(), other.NumberOfColumns() );

	tFloat * ptrToThisPixels;
	tByte  * ptrToOtherPixels;

    ptrToThisPixels  = ScanLine(0);
	ptrToOtherPixels = other.ScanLine(0);

	for ( int i=0; i<NumberOfPixels(); i++)
	{
      *ptrToThisPixels++ = (tFloat)((int) *ptrToOtherPixels++); // for sure conversion
	}
}

void c2dFloatGrayImage::LoadFromBmpFile( const char * bmpFileName )
{
	c2dByteGrayImage tempImage( bmpFileName );
	Init( tempImage );
}
    
void c2dFloatGrayImage::SaveAsOptimalGrayBmpFile( const char * bmpFileName)
{
	tFloat MinValue;
	tFloat MaxValue;

	tFloat * ptrToPixels = ScanLine(0);;
    MinValue = *ptrToPixels;
	MaxValue = MinValue;
    tFloat tempValue; 

	for ( int i=0; i< NumberOfPixels(); i++)
	{
      tempValue = *ptrToPixels++;
	  if ( MinValue > tempValue ) MinValue = tempValue; 
	  if ( MaxValue < tempValue ) MaxValue = tempValue; 
	}
    SaveToGrayBmpFile( bmpFileName, MinValue, MaxValue );
}

void c2dFloatGrayImage::SaveToGrayBmpFile( const char * bmpFileName,
		                                   tFloat MinValue,
							               tFloat MaxValue )
{
	c2dByteGrayImage tempImage( numberOfRows, numberOfColumns );
    // No LUT can be used here: explain
	tFloat c,b;
	if (MinValue == MaxValue)
	{
		c = 1; b = 0;
	}
	else
	{
		c = 255.0 / (MaxValue - MinValue);
		b = -c*MinValue;
	}
	tFloat tempValue;

	tByte * ptrToTempImagePixels = tempImage.ScanLine();
	tFloat * ptrToThisPixels     = ScanLine();

	for ( int i=0; i< tempImage.NumberOfPixels(); i++)
	{
		tempValue =  *ptrToThisPixels++;
        tempValue = c*tempValue+b;
		if ( tempValue < 0 )  tempValue = 0;
		if ( tempValue > 255) tempValue = 255;
        *ptrToTempImagePixels++ = (unsigned char)((int)tempValue);
	}
	tempImage.SaveToGrayBmpFile( bmpFileName ); 
}


void c2dFloatGrayImage::Clear( tFloat value )
{
	// hard work, memset can not be used here
	tFloat * ptrToThisPixels;
	ptrToThisPixels  = ScanLine(0);

	for ( int pixel = 0; pixel < NumberOfPixels(); pixel++)
	{
      *ptrToThisPixels++ = value;
	}
}



tFloat c2dFloatGrayImage::GetPixelValue( int row, int column )
{
    if ( IsInside( row, column) )
	{
		return *(fastAndDangerousPointerToPixel(row,column) ) ;
	}
	else
	{
		sprintf_s( ferrMessage,
			"Can not GetPixelValue for\nRow = %d {0..%d}\nColumn = %d {0..%d}",
                 row, (NumberOfRows()-1), column, (NumberOfColumns()-1));
		throw ferrMessage;
	}
}
 
void  c2dFloatGrayImage::SetPixelValue( int row, int column, tFloat value)
{
	if ( IsInside( row, column) )
	{
		*(fastAndDangerousPointerToPixel(row,column) ) = value ;
	}
	else
	{
		sprintf_s( ferrMessage,
			"Can not SetPixelValue for\nRow = %d {0..%d}\nColumn = %d {0..%d}",
                 row, (NumberOfRows()-1), column, (NumberOfColumns()-1));
		throw ferrMessage;
	}
}

bool c2dFloatGrayImage::IsInside( int row, int column)
{
    if ( row < 0 )                   return false;
	if ( row >= numberOfRows )       return false;
	if ( column < 0 )                return false;
	if ( column >= numberOfColumns ) return false;
	return true;
}

tFloat * c2dFloatGrayImage::ScanLine( int row )
{
	return PointerToPixel( row, 0);
}

tFloat * c2dFloatGrayImage::PointerToPixel( int row, int column )
{
  	if ( IsInside( row, column) )
	{
		return fastAndDangerousPointerToPixel(row,column);
	}
	else
	{
		sprintf_s( ferrMessage,
			"Can not provide Pointer to Pixel for\nRow = %d {0..%d}\nColumn = %d {0..%d}",
                 row, (NumberOfRows()-1), column, (NumberOfColumns()-1));
		throw ferrMessage;
	}
}
	
