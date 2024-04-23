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

void work1()
{
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

}

void main()
{
	work1();
	work2();
	
	cout << "Press any key to exit" << endl;
	_getch();
}
