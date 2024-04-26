#include "c2dByteGrayImage.h"

// the following is needed for cin and cout instead of printf
#include <iostream>
using namespace std;  // explain someday

#include  <stdio.h>   // for sprintf_s instead of sprintf
#include  <memory.h>  // for memset

char errMessage[128]; // Very bad C idea: local-global variable
                      // but code is more compact:
                      //    no need for multiple declarations
                      // Option was to iclude errMessage in the class,
                      //    but I do not want to spoil it for now...

typedef unsigned short int tWord; 
typedef unsigned int       tDWord;
typedef int                tLong;  


#define myBMP_TAG 0x4d42        // "BM"  

// BMP file starts with BITMAPINFOHEADER
// see MSHelp for last version of BITMAPINFOHEADER
struct myBITMAPFILEHEADER
{
		tWord bfType; // Specifies the file type, must be BM.
		tDWord bfSize; // Specifies the size, in bytes, of the bitmap file. 
		tWord bfReserved1; //Reserved; must be zero. 
		tWord bfReserved2; //Reserved; must be zero. 
        tDWord bfOffBits;   //  Specifies the offset, in bytes, 
		                   //  from the beginning of the BITMAPFILEHEADER structure to the bitmap bits.
};
	
// BITMAPINFO structure immediately follows the BITMAPFILEHEADER structure in the BMP file 
struct myBITMAPINFOHEADER
{
		tDWord biSize;   // Specifies the number of bytes required by the structure.
		tLong biWidth;  // Specifies the width of the bitmap, in pixels. 
        tLong biHeight; // Specifies the height of the bitmap, in pixels. 
		                //      If biHeight is positive, the bitmap is a bottom-up DIB
		                //      and its origin is the lower-left corner. 
		                //
		                //      If biHeight is negative, the bitmap is a top-down DIB
		                //      and its origin is the upper-left corner.
        tWord biPlanes; // Specifies the number of planes for the target device. 
		                //      This value must be set to 1. 
        tWord biBitCount;  // Specifies the number of bits-per-pixel
		                   // Must be one of the following: 
		                   //  0 - for JPEG or PNG format - does not rely on extention only
		                   //  1 - Monochrome, for example Black and White image
		                   //  4 - 16 colors - remember ???
		                   //  8 - 256 colors - still used with Palette
		                   //         detailed explanation will follow later
		                   //  24 - this is what we need now 24=8*3
		                   //  32 - 32 = 8*4 = BGR and ALPHA - transparency
		                   //       nearly not used today inthe Windows XP
		                   //       but increase speed (32 bit = size of integer )
		                   //       will be intencively used in GUI of Vista
		tDWord biCompression;   // Must be one of the following (constanmts defined deep in SDK
			                   //  BI_RGB = 0 - An uncompressed format. - what we need now
							   //  BI_RLE8, BI_RLE4  A run-length encoded (RLE) format
							   //           explanation later
							   //  BI_BITFIELDS - no compression. Used with 16 and 32 bpp images
                               //  BI_JPEG - Indicates that the image is a JPEG image.
							   //  BI_PNG - Indicates that the image is a PNG image.
		tDWord biSizeImage;    //  Specifies the size, in bytes, of the image. 
		                       //  This may be set to zero for BI_RGB bitmaps - our case here.
		tLong biXPelsPerMeter; // Specifies the horizontal resolution, in pixels-per-meter, 
		                       //   of the target device for the bitmap. 
		                       //   Mostly not used and thus not set
		tLong biYPelsPerMeter; //Specifies the vertical resolution, n pixels-per-meter, 
		                       //   of the target device for the bitmap. 
		                       //   Mostly not used and thus not set 
		tDWord biClrUsed;       // 0 in our case
		tDWord biClrImportant;  // 0 in our case
};

// Internal Service function: Rounds up row to 4 byte boundary
int AlignedRowSize( int rowSize ) 
{                             
  return ( ( (rowSize+3)/4)*4);
}

// Beware: sizeof is not always what we need because of padding of compiler
tDWord sizeOfmyBITMAPINFOHEADER = 40; // sizeof( myBITMAPINFOHEADER ) = 40;
tDWord sizeOfmyBITMAPFILEHEADER = 14; // sizeof( myBITMAPFILEHEADER ) = 16 != 14;

// Needed for GrayPalette
#define myMAXCOLORS  (256)        
unsigned char localGrayPalette[ myMAXCOLORS*4]; // 4 not 3

void FillGrayPalette()
{
  for ( int i=0; i< myMAXCOLORS; i++) 
  { 
    localGrayPalette[i*4]     = (unsigned char) i;     // Blue 
    localGrayPalette[i*4+1]   = (unsigned char) i;     // Green 
    localGrayPalette[i*4+2]   = (unsigned char) i;     // Red 
    localGrayPalette[i*4+3]   =  0;                    // Reserved 
  } 
}


c2dByteGrayImage::c2dByteGrayImage(void)
{
  	ptrToPixels     = 0;
	numberOfRows    = 0;
	numberOfColumns = 0;
	Init( 1, 1 );  // Image always have at least one pixel  
 }

c2dByteGrayImage::c2dByteGrayImage(int numOfRows, int numOfColumns )
{
  	ptrToPixels     = 0;
	numberOfRows    = 0;
	numberOfColumns = 0;
	Init( numOfRows, numOfColumns );
 }

c2dByteGrayImage::c2dByteGrayImage(c2dByteGrayImage & other )
{
  	ptrToPixels     = 0;
	numberOfRows    = 0;
	numberOfColumns = 0;
	Init( other );
 }

c2dByteGrayImage::c2dByteGrayImage( const char * bmpFileName )
{
	ptrToPixels     = 0;
	numberOfRows    = 0;
	numberOfColumns = 0;
	Init( bmpFileName );
}

c2dByteGrayImage::~c2dByteGrayImage(void)
{
  Kill();
}

void c2dByteGrayImage::Kill()
{
	if ( ptrToPixels != 0 )
	{
		delete [] ptrToPixels;
        ptrToPixels     = 0;
		numberOfRows    = 0;
		numberOfColumns = 0;
	}
}

void c2dByteGrayImage::Init( int numOfRows, int numOfColumns )
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
       sprintf_s( errMessage, "Negative or zero number of rows = %d requested",
		          numOfRows);
	   throw errMessage;     // errMessage is declared in cpp file – kind of local string storage 
	 }
      if ( numOfColumns <= 0 )
	 {
	   sprintf_s( errMessage, "Negative or zero number of columns = %d requested", 
		          numOfColumns);
	   throw errMessage; 
	 }
	 Kill();
	 numberOfRows    = numOfRows;
	 numberOfColumns = numOfColumns;

	 ptrToPixels = new tByte [NumberOfPixels()];
	 if ( ptrToPixels == 0 ) // really impossible case in WindowsXP: allocation failed 
	 {
        sprintf_s(errMessage, "Can not allocate %d bytes", NumberOfPixels() );
	   throw errMessage;
	 }

	 Clear(0); // takes time, but to be sure...
}


void c2dByteGrayImage::Init( c2dByteGrayImage & other )
{
	Init( other.NumberOfRows(), other.NumberOfColumns() );

	tByte * ptrToThisPixels;
	tByte * ptrToOtherPixels;

    ptrToThisPixels  = ScanLine(0);
	ptrToOtherPixels = other.ScanLine(0);

	for ( int i=0; i<NumberOfPixels(); i++)
	{
      *ptrToThisPixels++ = *ptrToOtherPixels++;
	}
	// fast memcpy may be used instead ...
}

void c2dByteGrayImage::Init( const char * bmpFileName )
{
   LoadFromBmpFile( bmpFileName );
}
 
void c2dByteGrayImage::LoadFromBmpFile( const char * bmpFileName )
{
	FILE * bmpFile; 
	errno_t err;
 
	err = fopen_s( &bmpFile, bmpFileName, "rb" );  
    if ( err != 0 ) // old way of error handling combined with new one
    { 
		sprintf_s( errMessage,
			"Can not open file: \n%s\n  for reading\n",
                 bmpFileName);
		throw errMessage;		
    }
     
    myBITMAPFILEHEADER bf;

    size_t res;

	res = fread( &bf.bfType, 1, 2, bmpFile );
    
	if ( res != 2 ) 
    { 
     fclose( bmpFile );
	 sprintf_s( errMessage,
			"Can not read BITMAPFILEHEADER from file: \n%s\n",
                 bmpFileName);
 	 throw errMessage;
    } 
	res = fread( &bf.bfSize, 1, 4, bmpFile );
	res = fread( &bf.bfReserved1, 1, 2, bmpFile );
	res = fread( &bf.bfReserved2, 1, 2, bmpFile );
	res = fread( &bf.bfOffBits, 1, 4, bmpFile );

    // Provide some checks:
    if ( bf.bfType != myBMP_TAG )
    {
	 fclose( bmpFile );	     
     sprintf_s( errMessage,
			"File: \n%s\n  is not BMP file",
                 bmpFileName);
 	 throw errMessage;
    }

    myBITMAPINFOHEADER bi;

    res = fread( &bi, 1, sizeOfmyBITMAPINFOHEADER, bmpFile ); 
    if ( res != sizeOfmyBITMAPINFOHEADER ) 
    {
	 fclose( bmpFile );	    
	 sprintf_s( errMessage,
			"Can not read BITMAPINFOHEADER from file: \n%s\n",
                 bmpFileName);
 	 throw errMessage;
    }

	int rowSize;
	switch ( bi.biBitCount )
	{
	   case 8: rowSize = AlignedRowSize( bi.biWidth );
		                 break;
       case 24: rowSize = 3*AlignedRowSize( bi.biWidth );
		                 break;
	   case 32: rowSize = 4*AlignedRowSize( bi.biWidth );
		                 break; 
	   default: 
		           fclose( bmpFile );	    
	               sprintf_s( errMessage,
			              "Unsupported pixel format in the file: \n%s\n",
                             bmpFileName);
 	               throw errMessage;
	}

    if ( bi.biBitCount == 8 ) // Gray Image: read and check palette 
	{
	   res = fread( localGrayPalette, 1, myMAXCOLORS*4, bmpFile ); 
       if ( res != myMAXCOLORS*4 ) 
       { 
		  fclose( bmpFile );	    
	      sprintf_s( errMessage,
			"Can not read PALETTE from file: \n%s\n",
                 bmpFileName);
 	      throw errMessage;
	   } 
    
	   for ( int i=0; i< myMAXCOLORS; i++)
	   {
		   if ( 
			    ( localGrayPalette[4*i] != i )
				      ||
                ( localGrayPalette[4*i+1] != i )
				      ||
                ( localGrayPalette[4*i+2] != i )
				      ||
                ( localGrayPalette[4*i+3] != 0 )
			   )
		         {
			       fclose( bmpFile );	    
	               sprintf_s( errMessage,
			              "PALETTE in the file: \n%s\n Is not Gray\n",
                             bmpFileName);
 	               throw errMessage;
		          }
	   }
    }
    
    // Create temp row
	unsigned char * tempRow;
	tempRow = new unsigned char[rowSize];

	// Create Gray array of requirted size
	Init( bi.biHeight, bi.biWidth );

    unsigned char * ptrToTempRow;
	unsigned char * ptrToPixels;

    unsigned char A,R,G,B;
	int X;

    for ( int row = 0; row < NumberOfRows(); row++)
	{
       res = fread( tempRow, 1, rowSize , bmpFile );
       if ( res != rowSize ) 
       { 
		  fclose( bmpFile );
		  delete [] tempRow;
	      sprintf_s( errMessage,
			"Can not read pixels from file: \n%s\n",
                 bmpFileName);
 	      throw errMessage;
	   }       
       ptrToPixels = ScanLine(row);
       ptrToTempRow = tempRow;

	   for ( int column =0; column< NumberOfColumns(); column++)
	   {
		   switch( bi.biBitCount )
		   {
		     case 8: 
				      *ptrToPixels++ = *ptrToTempRow++;
					  break;
			 case 24:
				      B = *ptrToTempRow++;
					  G = *ptrToTempRow++;
					  R = *ptrToTempRow++;					  
                      X = ((int)B + (int)G + (int)R) / 3;
                      *ptrToPixels++ = (unsigned char) X;
                      break;
             case 32:
				      B = *ptrToTempRow++;
					  G = *ptrToTempRow++;
					  R = *ptrToTempRow++;
					  A = *ptrToTempRow++;  // ignore
                      X = ((int)B + (int)G + (int)R) / 3;
                      *ptrToPixels++ = (unsigned char) X;
			 default:
				      fclose( bmpFile );
		              delete [] tempRow;	    
	                  sprintf_s( errMessage,
			              "Unsupported pixel format in the file: \n%s\n",
                             bmpFileName);
 	                  throw errMessage;
		   } // of switch

	   } // of column

	} // of row

	fclose( bmpFile );
	delete [] tempRow; 
}

void c2dByteGrayImage::SaveToGrayBmpFile( const char * bmpFileName )
{
	myBITMAPFILEHEADER bf;
	
    bf.bfType = myBMP_TAG;

	bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;
 
    bf.bfOffBits =     sizeOfmyBITMAPINFOHEADER 
                     + sizeOfmyBITMAPFILEHEADER 
					 + myMAXCOLORS*4;  // Gray Palette present
		
    bf.bfSize =   (tDWord) NumberOfRows() 
                * (tDWord) AlignedRowSize ( NumberOfColumns() )
				+ (tDWord) bf.bfOffBits ; 
  
	
	myBITMAPINFOHEADER bi;
    
	bi.biSize        = sizeOfmyBITMAPINFOHEADER; // sizeof( myBITMAPINFOHEADER ); 
    bi.biWidth       = NumberOfColumns();
    bi.biHeight      = NumberOfRows();
    bi.biPlanes      = 1;
    bi.biBitCount    = 8;
    bi.biCompression = 0;
    
    bi.biSizeImage = NumberOfRows()*NumberOfColumns();	 // no need for alignment here ?                 
 
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed       = 0;
    bi.biClrImportant  = 0;

    /////////// 
    FILE * bmpFile; 
	errno_t err;
	
	err = fopen_s( &bmpFile, bmpFileName, "wb" );  
    if ( err != 0 ) // old way of error handling combined with new one
    { 
		sprintf_s( errMessage,
			"Can not open file: \n%s\n  for writing\n",
                 bmpFileName);
		throw errMessage;		
    }

	size_t res; 

	// wrap-out: hard manual work because 14 != 16
    res = fwrite( &bf.bfType, 1, 2, bmpFile ); 

	if ( res != 2 )
	{
		fclose( bmpFile );
		sprintf_s( errMessage,
			"Can write variable to file: \n%s\n",
                 bmpFileName);
		throw errMessage;
	}

	// Hope that write will be OK here, no error checks ...
	res = fwrite( &bf.bfSize, 1, 4, bmpFile ); 
    res = fwrite( &bf.bfReserved1, 1, 2, bmpFile );
	res = fwrite( &bf.bfReserved2, 1, 2, bmpFile );
	res = fwrite( &bf.bfOffBits, 1, 4, bmpFile );

    // This work because of size 40
    res = fwrite( &bi, 1, sizeOfmyBITMAPINFOHEADER, bmpFile ); 
    if ( res != sizeOfmyBITMAPINFOHEADER ) 
    { 
        fclose( bmpFile ); 
	    sprintf_s( errMessage,
			"Can write myBITMAPINFOHEADER to file: \n%s\n",
                 bmpFileName);
		throw errMessage; 
    } 

	FillGrayPalette();
	// write palette
	res = fwrite( localGrayPalette, 1, myMAXCOLORS*4, bmpFile ); 
    if ( res != myMAXCOLORS*4 ) 
    { 
        fclose( bmpFile ); 
	    sprintf_s( errMessage,
			"Can write myGrayPalette to file: \n%s\n",
                 bmpFileName);
		throw errMessage; 
    } 
	
    unsigned char * ptrToPixels;
    ptrToPixels = ScanLine(0);

	int alignedRowSize = AlignedRowSize ( NumberOfColumns() );
	if ( NumberOfColumns() == alignedRowSize ) // work fast
	{
   	  int dataSizeInBytes = NumberOfColumns()*NumberOfRows();
	  res = fwrite( ptrToPixels, 1, dataSizeInBytes , bmpFile ); 

      if ( res != (unsigned int) dataSizeInBytes ) 
      { 
        fclose( bmpFile ); 
	    sprintf_s( errMessage,
			"Can write pixels to file: \n%s\n",
                 bmpFileName);
		throw errMessage; 
	  }  
	} // of good alignment
	else // work row by row
	{
	    // temp array for row storage
        unsigned char * tempRow;
		tempRow = new unsigned char [alignedRowSize]; // do not forget to delete !!!
	    unsigned char * ptrToTempRow;
		ptrToTempRow = tempRow;
		for ( int i=0; i<alignedRowSize; i++ ) 
	    {
		  *ptrToTempRow++ = 0;
	    }
	    	

		for (int row=0; row < NumberOfRows(); row++)
	    {
            ptrToTempRow = tempRow; 
		    ptrToPixels = ScanLine(row);
			
			for (int col=0; col < NumberOfColumns(); col++)
	       {
              *ptrToTempRow++ = *ptrToPixels++;
	       }
           res = fwrite( tempRow, 1, alignedRowSize , bmpFile );
           if ( res != (unsigned int) alignedRowSize ) 
           { 
             fclose( bmpFile ); 
	         delete [] tempRow;
			 sprintf_s( errMessage,
			 "Can write pixels to file: \n%s\n",
                 bmpFileName);
		     throw errMessage; 
	       } 
	    }
		delete [] tempRow; 
	} // of else
	fclose( bmpFile ); 

}


void c2dByteGrayImage::Clear( tByte value )
{
    // very fast implementation
	memset( ptrToPixels, value, NumberOfPixels() );
}

tByte c2dByteGrayImage::GetPixelValue( int row, int column )
{
	if ( IsInside( row, column) )
	{
		return *(fastAndDangerousPointerToPixel(row,column) ) ;
	}
	else
	{
		sprintf_s( errMessage,
			"Can not GetPixelValue for\nRow = %d {0..%d}\nColumn = %d {0..%d}\n",
                 row, (NumberOfRows()-1), column, (NumberOfColumns()-1));
		throw errMessage;
	}
}

void c2dByteGrayImage::SetPixelValue( int row, int column, tByte value)
{
	if ( IsInside( row, column) )
	{
		*(fastAndDangerousPointerToPixel(row,column) ) = value ;
	}
	else
	{
		sprintf_s( errMessage,
			"Can not SetPixelValue for\nRow = %d {0..%d}\nColumn = %d {0..%d}\n",
                 row, (NumberOfRows()-1), column, (NumberOfColumns()-1));
		throw errMessage;
	}
}

bool c2dByteGrayImage::IsInside( int row, int column)
{
	if ( row < 0 )                   return false;
	if ( row >= numberOfRows )       return false;
	if ( column < 0 )                return false;
	if ( column >= numberOfColumns ) return false;
	return true;
}

tByte * c2dByteGrayImage::ScanLine( int row )
{
	return PointerToPixel( row, 0);
}


tByte * c2dByteGrayImage::PointerToPixel( int row, int column )
{
  	if ( IsInside( row, column) )
	{
		return fastAndDangerousPointerToPixel(row,column);
	}
	else
	{
		sprintf_s( errMessage,
			"Can not provide Pointer to Pixel for\nRow = %d {0..%d}\nColumn = %d {0..%d}\n",
                 row, (NumberOfRows()-1), column, (NumberOfColumns()-1));
		throw errMessage;
	}
}



