#pragma once

#include "c2dByteGrayImage.h"

typedef double tFloat; 

class c2dFloatGrayImage
{
public:
	c2dFloatGrayImage(void);
	c2dFloatGrayImage( int numOfRows, int numOfColumns);
	c2dFloatGrayImage( c2dFloatGrayImage & other);
    c2dFloatGrayImage( c2dByteGrayImage  & other);

    c2dFloatGrayImage( const char * bmpFileName);
	
	~c2dFloatGrayImage(void);
	
	void Init( int numOfRows, int numOfColumns);
	void Init( c2dFloatGrayImage & other);
	void Init( c2dByteGrayImage  & other);

	void LoadFromBmpFile( const char * bmpFileName );
    
	void SaveAsOptimalGrayBmpFile( const char * bmpFileName);
	void SaveToGrayBmpFile( const char * bmpFileName,
		                    tFloat MinValue,
							tFloat MaxValue  );

	void Clear( tFloat value = 0 );

	int NumberOfRows()    { return numberOfRows; };
	int NumberOfColumns() { return numberOfColumns; };
	int NumberOfPixels()  { return numberOfRows*numberOfColumns; };

    tFloat GetPixelValue( int row, int column );
    void   SetPixelValue( int row, int column, tFloat value);

    bool   IsInside( int row, int column);

    tFloat * ScanLine( int row=0 );
	tFloat * PointerToPixel( int row=0, int column = 0 );	
	tFloat * fastAndDangerousPointerToPixel( int row = 0, int column = 0 )
	                  { return ptrToPixels + row*numberOfColumns + column; };

protected:
    void Kill();
	void ZeroFields();

	int numberOfRows;
	int numberOfColumns;

	tFloat *ptrToPixels;

	char errMessage[128]; 
	  // room for creating user-friendly error messages
	  // some day use smart (dynamic) class cString
};
