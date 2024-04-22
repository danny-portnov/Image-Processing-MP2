#pragma once

typedef unsigned char tByte; 

class c2dByteGrayImage
{
 public:
    c2dByteGrayImage(void);
    c2dByteGrayImage(int numOfRows, int numOfColumns);
    c2dByteGrayImage( c2dByteGrayImage & other );
	c2dByteGrayImage( const char * bmpFileName );
   ~c2dByteGrayImage(void);
 
   void Init( int numOfRows, int numOfColumns );
   void Init( c2dByteGrayImage & other );
   void Init( const char * bmpFileName );
 
   void LoadFromBmpFile( const char * bmpFileName );
   void SaveToGrayBmpFile( const char * bmpFileName );

   void Clear( tByte value = 0 );

   int NumberOfRows()    { return numberOfRows; };
   int NumberOfColumns() { return numberOfColumns; };
   int NumberOfPixels()  { return numberOfRows*numberOfColumns; };
  
   tByte GetPixelValue( int row, int column );
   void  SetPixelValue( int row, int column, tByte value);
   bool IsInside( int row, int column);

   tByte * ScanLine( int row=0 );
   tByte * PointerToPixel( int row=0, int column = 0 );
   tByte * fastAndDangerousPointerToPixel( int row = 0, int column = 0 )
	                  { return ptrToPixels + row*numberOfColumns + column; };
protected:
   void Kill();
   int numberOfRows;
   int numberOfColumns;
   tByte *ptrToPixels;  
};



