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
	c2dByteGrayImage temp_dest(src.NumberOfRows(), src.NumberOfColumns());
	double summa;

	// x direction
	for (int row = 0; row < src.NumberOfRows(); row++)
	{
		for (int column = FilterHalfSize; column < src.NumberOfColumns() - FilterHalfSize; column++)
		{
			summa = 0;
			for (int x = -FilterHalfSize; x <= FilterHalfSize; x++)
			{
				summa = summa + filter[FilterHalfSize + x] * (double)src.GetPixelValue(row, column + x);
			} // of x
			if (summa < 0)
				summa = 0;
			if (summa > 255)
				summa = 255;
			temp_dest.SetPixelValue(row, column, (tByte)(summa + 0.5));

		} // of column
	} // of row

	// Clear Source !!!
	src.Clear();

	// y direction
	for (int row = FilterHalfSize; row < src.NumberOfRows() - FilterHalfSize; row++)
	{
		for (int column = 0; column < src.NumberOfColumns(); column++)
		{
			summa = 0;
			for (int y = -FilterHalfSize; y <= FilterHalfSize; y++)
			{
				summa += filter[FilterHalfSize + y] * (double)temp_dest.GetPixelValue(row + y, column);

			} // of y
			if (summa < 0) summa = 0;
			if (summa > 255) summa = 255;
			src.SetPixelValue(row, column, (tByte)(summa + 0.5));
		} // of column
	} // of row
}

void CreateGaussianFilter(c2dFloatGrayImage& image, tFloat sigmaRadius)
{
	tFloat x, y, r, z;
	double summa = 0;
	for (int row = 0; row < image.NumberOfRows(); row++)
	{
		for (int column = 0; column < image.NumberOfColumns(); column++)
		{
			y = row - image.NumberOfRows() / 2;
			x = column - image.NumberOfColumns() / 2;
			r = sqrt(y * y + x * x);
			z = r / sigmaRadius;
			z = exp(-z * z);
			summa += z;
			image.SetPixelValue(row, column, z);
		}
	}
	for (int row = 0; row < image.NumberOfRows(); row++)
	{
		for (int column = 0; column < image.NumberOfColumns(); column++)
		{
			tFloat* ptr = image.fastAndDangerousPointerToPixel(row, column);
			*ptr /= summa;
		}
	}
}

void DoFiltrationByConvolution(c2dByteGrayImage& src, c2dByteGrayImage& dest, double filter[][3], double factor)
{
	int FilterHalfSize = 1;
	dest.Clear();
	for (int row = FilterHalfSize; row < src.NumberOfRows() - FilterHalfSize; row++)
	{
		for (int column = FilterHalfSize; column < src.NumberOfColumns() - FilterHalfSize; column++)
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
			dest.SetPixelValue(row, column, (tByte)(summa + 0.5));
		} // of column
	} // of row
}

void inverseFilter(c2dFloatGrayImage& filter, double eps)
{
	double summa = 0;
	tFloat* PixelPtr = filter.PointerToPixel();
	for (int pixel = 0; pixel < filter.NumberOfPixels(); pixel++)
	{
		*PixelPtr = 1 / (*PixelPtr + eps);
		summa += *PixelPtr;
		PixelPtr++;
	}
	PixelPtr = filter.PointerToPixel();
	for (int pixel = 0; pixel < filter.NumberOfPixels(); pixel++)
	{
		*PixelPtr /= summa;
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
				Image.SetPixelValue(row, column, centralValue);
			else
				Image.SetPixelValue(row, column, peripheryValue);
		}
	}
}

void Question2(char fig_src[], char fig_dest[][30])
{
	// // Question 2 
	////Option1:
	//c2dByteGrayImage src(fig_src);
	//double filter1[5];
	//double filter2[7];
	//PrepareGausssianFilter(filter1, 5.0, 2);
	//PrepareGausssianFilter(filter2, 2, 3);
	//DoGaussianFiltration(src, filter1, 2);
	//src.SaveToGrayBmpFile(fig_dest[0]);
	//src.Init("Tim1.bmp");
	//DoGaussianFiltration(src, filter2, 3);
	//src.SaveToGrayBmpFile(fig_dest[1]);

	////Option2:
	//c2dByteGrayImage src(fig_src);
	//c2dByteGrayImage dest(src.NumberOfRows(), src.NumberOfColumns());
	//double filter1[5][5] = { { 1,4,7,4,1 },{4,16,26,16,4},{7,26,41,26,7},{4,16,26,16,4},{ 1,4,7,4,1 } };
	//double factor1 = 1.0 / 273.0;
	//double filter2[7][7] = { {0,0,1,2,1,0,0},{0,3,13,22,13,3,0},{1,13,59,97,59,13,1},{2,22,97,159,97,22,2},{1,13,59,97,59,13,1},{0,3,13,22,13,3,0},{0,0,1,2,1,0,0} };
	//double factor2 = 1.0 / 1003.0;
	////DoFiltrationByConvolution(src, dest, filter1, factor1);
	////dest.SaveToGrayBmpFile(fig_dest[0]);
	//DoFiltrationByConvolution(src, dest, filter2, factor2);
	//dest.SaveToGrayBmpFile(fig_dest[1]);

	//Option3:
	c2dByteGrayImage src(fig_src);
	double filter1[5] = { 0.061,0.242,0.383,0.242,0.061 };
	double filter2[7] = { 0.006,0.061,0.242,0.383,0.242,0.061,0.006 };
	DoGaussianFiltration(src, filter1, 2);
	src.SaveToGrayBmpFile(fig_dest[0]);
	src.Init(fig_src);
	DoGaussianFiltration(src, filter2, 3);
	src.SaveToGrayBmpFile(fig_dest[1]);

}

void Question3(char fig_src[], char fig_dest[])
{
	// Question 3

	c2dFloatGrayImage ImageRe(fig_src);
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
	ImageRe.SaveAsOptimalGrayBmpFile(fig_dest);

}

void Question4(char files_src[][30], char files_dest1[][30], char files_dest2[][30])
{
	// TODO: put filters in function prepare filter.
	double filter1[5][5] = { {-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1},{-1,-1,49,-1,-1},{-1,-1,-1,-1,-1},{-1,-1,-1,-1,-1} };
	double filter2[5][5] = { {-0.5,-0.5,-0.5,-0.5,-0.5},{-0.5,-0.5,-0.5,-0.5,-0.5},{-0.5,-0.5,23.5,-0.5,-0.5},{-0.5,-0.5,-0.5,-0.5,-0.5},{-0.5,-0.5,-0.5,-0.5,-0.5} };
	double filter3[5][5] = { {-1,-4,-6,-4,-1},{-4,-16,-24,-16,-4},{-6,-24,476,-24,-6},{-4,-16,-24,-16,-4},{-1,-4,-6,-4,-1} };
	double filter4[5][5] = { {-1,-4,-6,-4,-1},{-4,-16,-24,-16,-4},{-6,-24,221,-24,-6},{-4,-16,-24,-16,-4},{-1,-4,-6,-4,-1} };
	double filter5[3][3] = { {0, -1, 0},{-1, 5,-1},{0, -1, 0} };
	double filter6[5][5] = { {0,0,-1,0,0},{0,-8,-21,-8,0},{-1,-21,299,-21,-1},{0,-8,-21,-8,0},{0,0,-1,0,0} };
	//the filters above didn't give good results.
	double filter7[3][3] = { {0, -1.0 / 4, 0},{-1.0 / 4, 2,-1.0 / 4},{0, -1.0 / 4, 0} };
	//selected filters
	double filter8[3][3] = { {-1, -1, -1},{-1, 9, -1},{-1, -1, -1} };
	double filter9[3][3] = { {-1, -2 , -1},{-2, 28 , -2},{-1, -2 , -1} };
	double filter10[3][3] = { {-2, -2, -2},{-2, 17, -2},{-2, -2, -2} };
	double filter11[3][3] = { {-4, -8 , -4},{-8, 64 , -8},{-4, -8 , -4} };
	double filter12[3][3] = { {-8, -16 , -8},{-16, 112 , -16},{-8, -16 , -8} };

	//c2dByteGrayImage src("Tim1LPF5c.bmp");
	//c2dByteGrayImage dest(src.NumberOfRows(), src.NumberOfColumns());
	double factor1 = 1.0 / 25.0;
	double factor3 = 1.0 / 256.0;
	double factor6 = 1.0 / 179.0;
	double factor9 = 1.0 / 16.0;
	//DoFiltrationByConvolution(src, dest, filter9, factor9);
	//dest.SaveToGrayBmpFile("Tim1HPF1c.bmp");

	for (int i = 0; i < 3; i++)
	{
		c2dByteGrayImage src(files_src[i]);
		c2dByteGrayImage dest(src.NumberOfRows(), src.NumberOfColumns());
		DoFiltrationByConvolution(src, dest, filter10, 1);//instead of filter8
		dest.SaveToGrayBmpFile(files_dest1[i]);
		DoFiltrationByConvolution(src, dest, filter12, factor9);//instead of filter9,filter11
		dest.SaveToGrayBmpFile(files_dest2[i]);
	}
}

void Question5(int NumOfRows, int NumOfColumns, char files_src[][30], char files_dest[][30])
{
	//char files_src[3][30] = { {"Tim1LPF5c.bmp"},{"Tim1LPF7c.bmp"},{"Tim1LPFF.bmp"} };
	//char files_dest[3][30] = { {"Tim1LPF5cHPFf.bmp"},{"Tim1LPF7cHPFf.bmp"},{"Tim1LPFFHPFf.bmp"} };
	c2dFloatGrayImage ImageIm(NumOfRows, NumOfColumns);
	c2dFloatGrayImage Filter(NumOfRows, NumOfColumns);
	CreateGaussianFilter(Filter, 70);
	inverseFilter(Filter, 0.000001);
	for (int i = 0; i < 3; i++)
	{
		c2dFloatGrayImage ImageRe(files_src[i]);
		ImageIm.Clear();
		ShiftHalfSize(ImageRe);
		ShiftHalfSize(ImageIm);
		DoFFT(ImageRe,
			ImageIm,
			FORWARD_FFT,
			NORMALIZE_BY_SQRT);
		DoFiltrationInFD(ImageRe, ImageIm, Filter);
		DoFFT(ImageRe,
			ImageIm,
			REVERSE_FFT,
			NORMALIZE_BY_SQRT);
		ShiftHalfSize(ImageRe);
		ShiftHalfSize(ImageIm);
		//ImageRe.SaveAsOptimalGrayBmpFile(files_dest[i]);
		ImageRe.SaveToGrayBmpFile(files_dest[i], 0, 0.00005);
	}
}

void Question7()
{
	char fig_src[] = "Tim2.bmp";
	char dest_q2[2][30] = { {"Tim2LPF5c.bmp"},{"Tim2LPF7c.bmp"} };
	Question2(fig_src, dest_q2);
	char dest_q3[] = "Tim2LPFF.bmp";
	Question3(fig_src, dest_q3);
	char files_src_q4[3][30] = { {"Tim2LPF5c.bmp"},{"Tim2LPF7c.bmp"},{"Tim2LPFF.bmp"} };
	char files_dest1_q4[3][30] = { {"Tim2LPF5cHPF1c.bmp"},{"Tim2LPF7cHPF1c.bmp"},{"Tim2LPFFHPF1c.bmp"} };
	char files_dest2_q4[3][30] = { {"Tim2LPF5cHPF2c.bmp"},{"Tim2LPF7cHPF2c.bmp"},{"Tim2LPFFHPF2c.bmp"} };
	Question4(files_src_q4, files_dest1_q4, files_dest2_q4);
	char files_src_q5[3][30] = { {"Tim2LPF5c.bmp"},{"Tim2LPF7c.bmp"},{"Tim2LPFF.bmp"} };
	char files_dest_q5[3][30] = { {"Tim2LPF5cHPFf.bmp"},{"Tim2LPF7cHPFf.bmp"},{"Tim2LPFFHPFf.bmp"} };
	Question5(320, 320, files_src_q5, files_dest_q5);
}

void main()
{
	char fig_src[] = "Tim1.bmp";
	char dest_q2[2][30] = { {"Tim1LPF5c.bmp"},{"Tim1LPF7c.bmp"} };
	Question2(fig_src, dest_q2);
	char dest_q3[] = "Tim1LPFF.bmp";
	Question3(fig_src, dest_q3);
	char files_src_q4[3][30] = { {"Tim1LPF5c.bmp"},{"Tim1LPF7c.bmp"},{"Tim1LPFF.bmp"} };
	char files_dest1_q4[3][30] = { {"Tim1LPF5cHPF1c.bmp"},{"Tim1LPF7cHPF1c.bmp"},{"Tim1LPFFHPF1c.bmp"} };
	char files_dest2_q4[3][30] = { {"Tim1LPF5cHPF2c.bmp"},{"Tim1LPF7cHPF2c.bmp"},{"Tim1LPFFHPF2c.bmp"} };
	Question4(files_src_q4, files_dest1_q4, files_dest2_q4);
	char files_src_q5[3][30] = { {"Tim1LPF5c.bmp"},{"Tim1LPF7c.bmp"},{"Tim1LPFF.bmp"} };
	char files_dest_q5[3][30] = { {"Tim1LPF5cHPFf.bmp"},{"Tim1LPF7cHPFf.bmp"},{"Tim1LPFFHPFf.bmp"} };
	Question5(480,640, files_src_q5, files_dest_q5);

	//Question7();

	cout << "Press any key to exit" << endl;
	_getch();
}
