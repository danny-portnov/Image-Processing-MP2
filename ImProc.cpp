// CPP BMP Library

#include <conio.h> // for _getch

// the following is needed for cin and cout instead of printf
#include <iostream>
using namespace std;  // explain someday

// The following is needed for math calculations
#include <math.h>
#define _USE_MATH_DEFINES 
#include <math.h> 

#include "c2dByteGrayImage.h"

#include <time.h>  // for spead measurements


enum eInterpolationType
{
	NoInterpolation, NearestNeigborInterpolation, BilinearInterpolation
};
struct sAffineTransform
{
	double a11;
	double a12;
	double a21;
	double a22;
	double b1;
	double b2;
};

void CalculateReverseAffineTransform(
	sAffineTransform& forward, sAffineTransform& reverse)
{
	// Forward Afine Transform:
	// Xnew = a11*Xold + a12*Yold + b1;
	// Ynew = a21*Xold + a22*Yold + b2;
	// for reverse Affine Transform see MAPLE solution
	double det;
	det = forward.a11 * forward.a22 - forward.a21 * forward.a12;
	if (det == 0)
	{
		throw "Can not calculate reverse Affine Transform: det = 0";
	}
	reverse.a11 = forward.a22 / det;
	reverse.a12 = -forward.a12 / det;
	reverse.b1 = (forward.a12 * forward.b2 - forward.a22 * forward.b1) / det;
	reverse.a21 = -forward.a21 / det;
	reverse.a22 = +forward.a11 / det;
	reverse.b2 = -(forward.a11 * forward.b2 - forward.a21 * forward.b1) / det;
}

unsigned char GetInterpolatedPixelValue(c2dByteGrayImage& src,
	double oldRow,
	double oldColumn,
	eInterpolationType interpolationType)
{
	int intOldRow;
	int intOldColumn;
	if (interpolationType == BilinearInterpolation)
	{
		double x, y;
		double a, b, c, d;
		double I, I00, I01, I10, I11; // order XY
		intOldRow = (int)oldRow; // integer part
		y = oldRow - intOldRow; // double part

		intOldColumn = (int)oldColumn; // integer part
		x = oldColumn - intOldColumn; // double part

		if (src.IsInside(intOldRow, intOldColumn) == false || src.IsInside(intOldRow + 1, intOldColumn + 1) == false)
			return 0;

		// I = a + b*x + c*y + d*x*y
		// x = 0; y = 0;
		I00 = *src.fastAndDangerousPointerToPixel(intOldRow, intOldColumn);
		// I00 = a + 0 + 0 + 0 --> a = I00
		a = I00;
		// x = 1; y = 0;
		I10 = *src.fastAndDangerousPointerToPixel(intOldRow, intOldColumn + 1);;
		// I10 = a + b*1 + 0 + 0 --> I10 = a + b*1 --> b = I10 - a
		b = I10 - a;
		// x = 0; y = 1;
		I01 = *src.fastAndDangerousPointerToPixel(intOldRow + 1, intOldColumn);
		// I01 = a + 0 + c*1 + 0 --> I01 = a + c*1 --> c = I01 - a
		c = I01 - a;
		// x = 1; y = 1;
		I11 = *src.fastAndDangerousPointerToPixel(intOldRow + 1, intOldColumn + 1);
		// I11 = a + b*1 + c*1 + d*1*1 --> I11 = a + b*1 + c*1 +d*1*1 --> d = I11 - a - b - c;
		d = I11 - a - b - c;
		I = a + b * x + c * y + d * x * y;

		return (unsigned char)((int)(I + 0.5)); // rounding to unsigned char
	}
	switch (interpolationType)
	{
	case NoInterpolation:
		intOldRow = (int)oldRow;
		intOldColumn = (int)oldColumn;
		break;

	case NearestNeigborInterpolation:
		intOldRow = (int)(oldRow + 0.5);
		intOldColumn = (int)(oldColumn + 0.5);
		break;
	}
	if (src.IsInside(intOldRow, intOldColumn) == true)
	{
		return *src.fastAndDangerousPointerToPixel(intOldRow, intOldColumn);
	}
	else
	{
		return 0;
	}
}

void DoExactAffineTransform(c2dByteGrayImage& src,
	c2dByteGrayImage& dest,
	sAffineTransform& forwardAffineTransform,
	eInterpolationType interpolationType)
{
	sAffineTransform reverseAffineTransform;
	CalculateReverseAffineTransform(
		forwardAffineTransform, reverseAffineTransform);
	double oldRow, oldColumn, interpolatedValue;
	unsigned char* ptrToDestPixel;
	ptrToDestPixel = dest.ScanLine(0);
	for (int row = 0; row < dest.NumberOfRows(); row++)
	{
		for (int column = 0; column < dest.NumberOfColumns(); column++)
		{
			oldColumn =
				reverseAffineTransform.a11 * column
				+ reverseAffineTransform.a12 * row
				+ reverseAffineTransform.b1;
			oldRow =
				reverseAffineTransform.a21 * column
				+ reverseAffineTransform.a22 * row
				+ reverseAffineTransform.b2;
			interpolatedValue = GetInterpolatedPixelValue(src, oldRow, oldColumn, interpolationType);
			*ptrToDestPixel++ = (unsigned char)((int)(interpolatedValue + 0.5));
		}
	}
}

void DoSmartAffineTransform(c2dByteGrayImage& src, c2dByteGrayImage& dest,
	sAffineTransform& forwardAffineTransform,
	eInterpolationType interpolationType)
{
	double MinC;
	double MaxC;
	double MinR;
	double MaxR;
	double tempC;
	double tempR;
	// Scan 4 corners
	tempC = forwardAffineTransform.a11 * 0
		+ forwardAffineTransform.a12 * 0
		+ forwardAffineTransform.b1;
	tempR = forwardAffineTransform.a21 * 0
		+ forwardAffineTransform.a22 * 0
		+ forwardAffineTransform.b2;

	MinC = tempC;
	MaxC = MinC;
	MinR = tempR;
	MaxR = MinR;
	tempC = forwardAffineTransform.a11 * (src.NumberOfColumns() - 1)
		+ forwardAffineTransform.a12 * 0
		+ forwardAffineTransform.b1;
	tempR = forwardAffineTransform.a21 * (src.NumberOfColumns() - 1)
		+ forwardAffineTransform.a22 * 0
		+ forwardAffineTransform.b2;
	if (MinC > tempC) MinC = tempC;
	if (MaxC < tempC) MaxC = tempC;
	if (MinR > tempR) MinR = tempR;
	if (MaxR < tempR) MaxR = tempR;
	tempC = forwardAffineTransform.a11 * 0
		+ forwardAffineTransform.a12 * (src.NumberOfRows() - 1)
		+ forwardAffineTransform.b1;
	tempR = forwardAffineTransform.a21 * 0
		+ forwardAffineTransform.a22 * (src.NumberOfRows() - 1)
		+ forwardAffineTransform.b2;
	if (MinC > tempC) MinC = tempC;
	if (MaxC < tempC) MaxC = tempC;
	if (MinR > tempR) MinR = tempR;
	if (MaxR < tempR) MaxR = tempR;
	tempC = forwardAffineTransform.a11 * (src.NumberOfColumns() - 1)
		+ forwardAffineTransform.a12 * (src.NumberOfRows() - 1)
		+ forwardAffineTransform.b1;
	tempR = forwardAffineTransform.a21 * (src.NumberOfColumns() - 1)
		+ forwardAffineTransform.a22 * (src.NumberOfRows() - 1)
		+ forwardAffineTransform.b2;
	if (MinC > tempC) MinC = tempC;
	if (MaxC < tempC) MaxC = tempC;
	if (MinR > tempR) MinR = tempR;
	if (MaxR < tempR) MaxR = tempR;
	int newNumberOfRows = (int)(MaxR - MinR + 1.5); // round up
	int newNumberOfColumns = (int)(MaxC - MinC + 1.5); // round up
	dest.Init(newNumberOfRows, newNumberOfColumns);
	// recreate according to new sizes
	// be smart here is to use brute force:
	forwardAffineTransform.b1 -= MinC;
	forwardAffineTransform.b2 -= MinR;
	DoExactAffineTransform(src, dest,
		forwardAffineTransform, interpolationType);
}


void main()
{

	sAffineTransform transform = { 2.15335784317537,0.611484451022928,-0.224951054343865,1.19847517968584,-73.97715033,9.510640878};//parameters for the transform
	c2dByteGrayImage ImageSrc("grayImage13.bmp");
	c2dByteGrayImage ImageDest;
	DoSmartAffineTransform(ImageSrc, ImageDest, transform, NearestNeigborInterpolation);
	ImageDest.SaveToGrayBmpFile("result-NearestNeigborInterpolation.bmp");
	ImageDest.Clear();
	DoSmartAffineTransform(ImageSrc, ImageDest, transform, BilinearInterpolation);
	ImageDest.SaveToGrayBmpFile("result-BilinearInterpolation.bmp");

	cout << "Press any key to exit" << endl;
	_getch();
}
