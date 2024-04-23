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
#include "c2dFloatGrayImage.h"
#include "FFT.h"
#include <time.h>  // for spead measurements

#define NUMBER_OF_ROWS 480
#define NUMBER_OF_COLUMNS 640


void PrepareGausssianFilter(double filter[], double sigma, int FilterHalfSize)
{
	double summa = 0;
	double temp;
	for (int i = 0; i < (2 * FilterHalfSize + 1); i++)
	{
		temp = (double)(i - FilterHalfSize) / sigma;
		temp = exp(-temp * temp);
		filter[i] = temp;
		summa += temp;
	}
	for (int i = 0; i < (2 * FilterHalfSize + 1); i++)
	{
		filter[i] = filter[i] / summa;
	}
}

// uses temp_dest inside
void DoGaussianFiltration(c2dByteGrayImage& src,
	double filter[], int FilterHalfSize)
{
	c2dByteGrayImage temp_dest(NUMBER_OF_ROWS,NUMBER_OF_COLUMNS);
	double summa;

	// x direction
	for (int row = 0; row < NUMBER_OF_ROWS; row++)
	{
		for (int column = FilterHalfSize; column < NUMBER_OF_COLUMNS - FilterHalfSize; column++)
		{
			summa = 0;
			for (int x = -FilterHalfSize; x <= FilterHalfSize; x++)
			{
				summa = summa + filter[FilterHalfSize + x] * (double)src.GetPixelValue(row,column + x);
			} // of x
			if (summa < 0) summa = 0;
			if (summa > 255) summa = 255;
			temp_dest.SetPixelValue(row, column,(unsigned char)summa);

		} // of column
	} // of row

	// Clear Source !!!
	src.Clear();

	// y direction
	for (int row = FilterHalfSize; row < NUMBER_OF_ROWS - FilterHalfSize; row++)
	{
		for (int column = 0; column < NUMBER_OF_COLUMNS; column++)
		{
			summa = 0;
			for (int y = -FilterHalfSize; y <= FilterHalfSize; y++)
			{
				summa += filter[FilterHalfSize + y] * (double)temp_dest.GetPixelValue(row + y,column);

			} // of y
			if (summa < 0) summa = 0;
			if (summa > 255) summa = 255;
			src.SetPixelValue(row,column,(unsigned char)summa);
		} // of column
	} // of row
}


void CreateGaussianFilter(c2dFloatGrayImage& image, tFloat sigmaRadius)
{
	tFloat x, y, r, z;
	for (int row = 0; row < image.NumberOfRows(); row++)
	{
		for (int column = 0; column < image.NumberOfColumns(); column++)
		{
			y = row - image.NumberOfRows() / 2;
			x = column - image.NumberOfColumns() / 2;
			r = sqrt(y * y + x * x);
			z = r / sigmaRadius;
			z = exp(-z * z);
			image.SetPixelValue(row, column, z);
		}
	}
}


void work1()
{
	// Question 2 

	c2dByteGrayImage src("Tim1.bmp");
	double filter1[5] = { 0.066,0.246,0.388,0.246,0.066 }; //sum above 1..
	double filter2[7];
	PrepareGausssianFilter(filter1, 5.0, 2);
	PrepareGausssianFilter(filter2, 1, 3);
	DoGaussianFiltration(src, filter1, 5 / 2);
	src.SaveToGrayBmpFile("Tim1LPF5c.bmp");
	src.Init("Tim1.bmp");
	DoGaussianFiltration(src, filter2, 7 / 2);
	src.SaveToGrayBmpFile("Tim1LPF7c.bmp");

}

void work2()
{
	// Question 3
	
	c2dFloatGrayImage ImageRe("Tim1.bmp");
	c2dFloatGrayImage ImageIm(ImageRe.NumberOfRows(), ImageRe.NumberOfColumns());
	c2dFloatGrayImage GaussFilter(ImageRe.NumberOfRows(), ImageRe.NumberOfColumns());
	CreateGaussianFilter(GaussFilter, 80);
	ShiftHalfSize(ImageRe);
	ShiftHalfSize(ImageIm);
	DoFFT(ImageRe,
		ImageIm,
		FORWARD_FFT,
		NORMALIZE_BY_SQRT);
	ImageRe.SaveAsOptimalGrayBmpFile("OriginalReInFD.bmp");
	ImageIm.SaveAsOptimalGrayBmpFile("OriginalImInFD.bmp");
	GaussFilter.SaveAsOptimalGrayBmpFile("GaussFilter.bmp");
	DoFiltrationInFD(ImageRe, ImageIm, GaussFilter);
	DoFFT(ImageRe,
		ImageIm,
		REVERSE_FFT,
		NORMALIZE_BY_SQRT);
	ShiftHalfSize(ImageRe);
	ShiftHalfSize(ImageIm);
	ImageRe.SaveAsOptimalGrayBmpFile("Filtered.bmp");

}

void main()
{
	//work1();
	work2();
	
	cout << "Press any key to exit" << endl;
	_getch();
}
