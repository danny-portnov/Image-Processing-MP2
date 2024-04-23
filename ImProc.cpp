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
void DoGaussianFiltration(c2dByteGrayImage& src, double filter[], int FilterHalfSize)
{
	c2dByteGrayImage temp_dest(NUMBER_OF_ROWS, NUMBER_OF_COLUMNS);
	double summa;

	// x direction
	for (int row = 0; row < NUMBER_OF_ROWS; row++)
	{
		for (int column = FilterHalfSize; column < NUMBER_OF_COLUMNS - FilterHalfSize; column++)
		{
			summa = 0;
			for (int x = -FilterHalfSize; x <= FilterHalfSize; x++)
			{
				summa = summa + filter[FilterHalfSize + x] * (double)src.GetPixelValue(row, column + x);
			} // of x
			if (summa < 0) summa = 0;
			if (summa > 255) summa = 255;
			temp_dest.SetPixelValue(row, column, (unsigned char)summa);

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
				summa += filter[FilterHalfSize + y] * (double)temp_dest.GetPixelValue(row + y, column);

			} // of y
			if (summa < 0) summa = 0;
			if (summa > 255) summa = 255;
			src.SetPixelValue(row, column, (unsigned char)summa);
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

void DoFiltrationByConvolution(c2dFloatGrayImage& src, c2dFloatGrayImage& dest, double filter[][5], double factor)
{
	int FilterHalfSize = 2;
	dest.Clear();
	for (int row = FilterHalfSize; row < NUMBER_OF_ROWS - FilterHalfSize; row++)
	{
		for (int column = FilterHalfSize; column < NUMBER_OF_COLUMNS - FilterHalfSize; column++)
		{
			double summa = 0;
			for (int y = -FilterHalfSize; y <= FilterHalfSize; y++)
			{
				for (int x = -FilterHalfSize; x <= FilterHalfSize; x++)
				{
					summa += filter[FilterHalfSize + y][FilterHalfSize + x] * (double)src.GetPixelValue(row + y, column + x);
				} // of x
			} // of y
			summa = summa * factor;
			if (summa < 0) summa = 0;
			if (summa > 255) summa = 255;
			dest.SetPixelValue(row, column, (unsigned char)summa);
		} // of column
	} // of row
}

void inverseFilter(c2dFloatGrayImage& filter, double eps)
{
	tFloat* PixelPtr = filter.PointerToPixel();
	for (int pixel = 0; pixel < filter.NumberOfPixels(); pixel++)
	{
		*PixelPtr = 1 / (*PixelPtr + eps);
		PixelPtr++;
	}
}

void CreateCircle(c2dFloatGrayImage& Image, int  raduis, tFloat peripheryValue, tFloat centralValue)
{
	int centX;
	int centY;
	for (int row = 0; row < NUMBER_OF_ROWS; row++)
	{
		for (int column = 0; column < NUMBER_OF_COLUMNS; column++)
		{
			centX = NUMBER_OF_ROWS / 2;
			centY = NUMBER_OF_COLUMNS / 2;

			int dx = centX - row;
			int dy = centY - column;
			if (((dx * dx) + (dy * dy)) <= (raduis * raduis))
				Image.SetPixelValue(row,column,centralValue);
			else
				Image.SetPixelValue(row, column, peripheryValue);
		}
	}
}

void Question2()
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

void Question3()
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
	GaussFilter.SaveAsOptimalGrayBmpFile("GaussFilter.bmp");
	DoFiltrationInFD(ImageRe, ImageIm, GaussFilter);
	DoFFT(ImageRe,
		ImageIm,
		REVERSE_FFT,
		NORMALIZE_BY_SQRT);
	ShiftHalfSize(ImageRe);
	ShiftHalfSize(ImageIm);
	ImageRe.SaveAsOptimalGrayBmpFile("Tim1LPFF.bmp");

}

void Question4()
{
	// TODO: put filters in function prepare filter.
	double filter1[5][5] = { {-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1},{-1,-1,24,-1,-1},{-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1} };
	double filter2[5][5] = { {-0.5,-0.5,-0.5,-0.5,-0.5},{-0.5,-0.5,-0.5,-0.5,-0.5},{-0.5,-0.5,23.5,-0.5,-0.5},{-0.5,-0.5,-0.5,-0.5,-0.5},{-0.5,-0.5,-0.5,-0.5,-0.5} };
	double filter3[5][5] = { {-1,-4,-6,-4,-1},{-4,-16,-24,-16,-4},{-6,-24,476,-24,-6},{-4,-16,-24,-16,-4},{-1,-4,-6,-4,-1} };
	double filter4[5][5] = { {-1,-4,-6,-4,-1},{-4,-16,-24,-16,-4},{-6,-24,221,-24,-6},{-4,-16,-24,-16,-4},{-1,-4,-6,-4,-1} };
	double filter5[3][3] = { {0, -1, 0},{-1, 5,-1},{0, -1, 0} };
	double filter6[5][5] = { {0,0,-1,0,0},{0,-8,-21,-8,0},{-1,-21,299,-21,-1},{0,-8,-21,-8,0},{0,0,-1,0,0} };

	c2dFloatGrayImage src("Tim1LPF5c.bmp");
	c2dFloatGrayImage dest(src.NumberOfRows(), src.NumberOfColumns());
	double factor1 = 1.0 / 25.0;
	double factor2 = 1.0 / 256.0;
	double factor6 = 1.0 / 179.0;
	DoFiltrationByConvolution(src, dest, filter6, factor6);
	//dest.SaveToGrayBmpFile("Tim1HPF1c.bmp",0,10);
	dest.SaveAsOptimalGrayBmpFile("Tim1HPF1c.bmp");
}

void Question5()
{
	c2dFloatGrayImage ImageRe("Tim1LPFF.bmp");
	c2dFloatGrayImage ImageIm(ImageRe.NumberOfRows(), ImageRe.NumberOfColumns());
	c2dFloatGrayImage Filter(ImageRe.NumberOfRows(), ImageRe.NumberOfColumns());
	//CreateGaussianFilter(Filter, 80);
	//inverseFilter(Filter, 0.01);
	CreateCircle(Filter, 80, 20, 220);
	ShiftHalfSize(ImageRe);
	ShiftHalfSize(ImageIm);
	DoFFT(ImageRe,
		ImageIm,
		FORWARD_FFT,
		NORMALIZE_BY_SQRT);
	Filter.SaveAsOptimalGrayBmpFile("Filter.bmp");
	DoFiltrationInFD(ImageRe, ImageIm, Filter);
	DoFFT(ImageRe,
		ImageIm,
		REVERSE_FFT,
		NORMALIZE_BY_SQRT);
	ShiftHalfSize(ImageRe);
	ShiftHalfSize(ImageIm);
	ImageRe.SaveAsOptimalGrayBmpFile("Tim1HPFf.bmp");
}

void main()
{
	//Question2();
	//Question3();
	//Question4();
	Question5();

	cout << "Press any key to exit" << endl;
	_getch();
}
