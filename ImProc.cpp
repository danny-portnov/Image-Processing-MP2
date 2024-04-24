// CPP BMP Library

#include <conio.h> // for _getch

// the following is needed for cin and cout instead of printf
#include <iostream>
using namespace std;  // explain someday

// The following is needed for math calculations
#include <math.h>
#define _USE_MATH_DEFINES 
#include <math.h> 
#include <Windows.h>
#include <vector>

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

void DoGaussianFiltration(c2dByteGrayImage& source, double filter[], int filterHalfSize)
{
	int numRows = source.NumberOfRows();
	int numColumns = source.NumberOfColumns();
	c2dByteGrayImage temp_dest(numRows, numColumns);
	double summa;

	// x direction
	for (int row = 0; row < numRows; row++)
	{
		for (int column = filterHalfSize; column < numColumns - filterHalfSize; column++)
		{
			summa = 0;
			for (int x = -filterHalfSize; x <= filterHalfSize; x++)
			{
				summa = summa + filter[filterHalfSize + x] * (double)source.GetPixelValue(row, column + x);
			} // of x
			if (summa < 0)
				summa = 0;
			if (summa > 255)
				summa = 255;
			temp_dest.SetPixelValue(row, column, (tByte)(summa + 0.5));

		} // of column
	} // of row

	// Clear Source !!!
	source.Clear();

	// y direction
	for (int row = filterHalfSize; row < numRows - filterHalfSize; row++)
	{
		for (int column = 0; column < numColumns; column++)
		{
			summa = 0;
			for (int y = -filterHalfSize; y <= filterHalfSize; y++)
			{
				summa += filter[filterHalfSize + y] * (double)temp_dest.GetPixelValue(row + y, column);

			} // of y
			if (summa < 0) summa = 0;
			if (summa > 255) summa = 255;
			source.SetPixelValue(row, column, (tByte)(summa + 0.5));
		} // of column
	} // of row
}

void CreateGaussianFilter(c2dFloatGrayImage& image, tFloat sigmaRadius)
{
	tFloat x, y, r, z;
	double summa = 0;
	int numRows = image.NumberOfRows();
	int numColumns = image.NumberOfColumns();
	for (int row = 0; row < numRows; row++)
	{
		for (int column = 0; column < numColumns; column++)
		{
			y = row - numRows / 2;
			x = column - numColumns / 2;
			r = sqrt(y * y + x * x);
			z = r / sigmaRadius;
			z = exp(-z * z);
			summa += z;
			image.SetPixelValue(row, column, z);
		}
	}
	Normalize(image, image.NumberOfPixels(), summa);
}

void DoFiltrationByConvolution(c2dByteGrayImage& source, c2dByteGrayImage& dest, double filter[][3], double factor)
{
	int filterHalfSize = 1;
	dest.Clear();
	int numRows = source.NumberOfRows();
	int numColumns = source.NumberOfColumns();
	for (int row = filterHalfSize; row < numRows - filterHalfSize; row++)
	{
		for (int column = filterHalfSize; column < numColumns - filterHalfSize; column++)
		{
			double summa = 0;
			for (int y = -filterHalfSize; y <= filterHalfSize; y++)
			{
				for (int x = -filterHalfSize; x <= filterHalfSize; x++)
				{
					summa += filter[filterHalfSize + y][filterHalfSize + x] * (double)source.GetPixelValue(row + y, column + x);
				} // of x
			} // of y
			summa = summa * factor;
			if (summa < 0) summa = 0;
			if (summa > 255) summa = 255;
			dest.SetPixelValue(row, column, (tByte)(summa + 0.5));
		} // of column
	} // of row
}

void Normalize(c2dFloatGrayImage& filter, int numPixels, double summa)
{
	tFloat* pixelPointer = filter.PointerToPixel();
	for (int pixel = 0; pixel < numPixels; pixel++)
	{
		*pixelPointer /= summa;
		pixelPointer++;
	}
}

void InverseFilter(c2dFloatGrayImage& filter, double eps)
{
	double summa = 0;
	tFloat* pixelPointer = filter.PointerToPixel();
	int numPixels = filter.NumberOfPixels();
	for (int pixel = 0; pixel < numPixels; pixel++)
	{
		*pixelPointer = 1 / (*pixelPointer + eps);
		summa += *pixelPointer;
		pixelPointer++;
	}
	Normalize(filter, numPixels, summa);
}

string AddBMPExtention(string rawName)
{
	return rawName + ".BMP";
}

// Applies 2 LPF filters to the given image and returns the
// file names of the blurred images (with added "LPF5c" and "LPF7c" suffixes)
vector<string> ApplyFastGaussianFilterConvolution(string inputFileName)
{
	c2dByteGrayImage source(AddBMPExtention(inputFileName).c_str());
	double LPF1[5] = { 0.061,0.242,0.383,0.242,0.061 };
	double LPF2[7] = { 0.006,0.061,0.242,0.383,0.242,0.061,0.006 };
	DoGaussianFiltration(source, LPF1, 2);
	string LPF1Suffix = "LPF5c";
	string LPF2Suffix = "LPF7c";
	vector<string> outputFileNames =
	{
		inputFileName + LPF1Suffix,
		inputFileName + LPF2Suffix
	};
	source.SaveToGrayBmpFile(AddBMPExtention(outputFileNames[0]).c_str());
	source.Init(inputFileName.c_str());
	DoGaussianFiltration(source, LPF2, 3);
	source.SaveToGrayBmpFile(AddBMPExtention(outputFileNames[1]).c_str());
	return outputFileNames;
}

// Applies a Gaussian filter in the frequency domain
//  to the given image and saves the filtered image.
// returns the filtered image name with added "LPFF" suffix
string ApplyFFT(string inputFileName)
{
	c2dFloatGrayImage ImageRe(AddBMPExtention(inputFileName).c_str());
	c2dFloatGrayImage ImageIm(ImageRe.NumberOfRows(), ImageRe.NumberOfColumns());
	c2dFloatGrayImage GaussFilter(ImageRe.NumberOfRows(), ImageRe.NumberOfColumns());
	CreateGaussianFilter(GaussFilter, 80);
	ShiftHalfSize(ImageRe);
	ShiftHalfSize(ImageIm);
	DoFFT(ImageRe,
		ImageIm,
		FORWARD_FFT,
		NORMALIZE_BY_SQRT);
	// Todo: remove after powerpoint
	GaussFilter.SaveAsOptimalGrayBmpFile("GaussFilter.bmp");
	DoFiltrationInFD(ImageRe, ImageIm, GaussFilter);
	DoFFT(ImageRe,
		ImageIm,
		REVERSE_FFT,
		NORMALIZE_BY_SQRT);
	ShiftHalfSize(ImageRe);
	ShiftHalfSize(ImageIm);
	string outputFileName = inputFileName + "LPFF";
	ImageRe.SaveAsOptimalGrayBmpFile(AddBMPExtention(outputFileName).c_str());
	return outputFileName;
}

void ApplyHPFToBlurredImages(vector<string> inputFileNames)
{
	double HPF1[3][3] = { {-2, -2, -2},{-2, 17, -2},{-2, -2, -2} };
	double HPF2[3][3] = { {-8, -16 , -8},{-16, 112 , -16},{-8, -16 , -8} };
	double HTP2factor = 1.0 / 16.0;
	int fileNamesCount = inputFileNames.size();
	for (int i = 0; i < fileNamesCount; i++)
	{
		c2dByteGrayImage source(AddBMPExtention(inputFileNames[i]).c_str());
		c2dByteGrayImage dest(source.NumberOfRows(), source.NumberOfColumns());
		DoFiltrationByConvolution(source, dest, HPF1, 1);
		dest.SaveToGrayBmpFile(AddBMPExtention(inputFileNames[i] + "HPF1c").c_str());
		DoFiltrationByConvolution(source, dest, HPF2, HTP2factor);
		dest.SaveToGrayBmpFile(AddBMPExtention(inputFileNames[i] + "HPF2c").c_str());
	}
}

void ApplyFFTToBlurredImages(int NumOfRows, int NumOfColumns, vector<string> inputFileNames)
{
	c2dFloatGrayImage imageIm(NumOfRows, NumOfColumns);
	c2dFloatGrayImage filter(NumOfRows, NumOfColumns);
	CreateGaussianFilter(filter, 70);
	InverseFilter(filter, 0.000001);
	int fileNamesCount = inputFileNames.size();
	for (int i = 0; i < fileNamesCount; i++)
	{
		c2dFloatGrayImage ImageRe(AddBMPExtention(inputFileNames[i]).c_str());
		imageIm.Clear();
		ShiftHalfSize(ImageRe);
		ShiftHalfSize(imageIm);
		DoFFT(ImageRe,
			imageIm,
			FORWARD_FFT,
			NORMALIZE_BY_SQRT);
		DoFiltrationInFD(ImageRe, imageIm, filter);
		DoFFT(ImageRe,
			imageIm,
			REVERSE_FFT,
			NORMALIZE_BY_SQRT);
		ShiftHalfSize(ImageRe);
		ShiftHalfSize(imageIm);
		ImageRe.SaveToGrayBmpFile(
			AddBMPExtention(inputFileNames[i] + "HPFf").c_str(),
			0, 0.00005);
	}
}


void main()
{
	//Question 2
	string grayImageName = "Tim1";
	vector<string> blurredImagesNames = ApplyFastGaussianFilterConvolution(grayImageName);

	//Question 3
	blurredImagesNames.push_back(ApplyFFT(grayImageName));

	//Question 4
	ApplyHPFToBlurredImages(blurredImagesNames);

	//Question 5
	ApplyFFTToBlurredImages(480, 640, blurredImagesNames);

	//Question 7
	string TVTestImageName = "Tim2.bmp";
	vector<string> blurredTVImagesNames = ApplyFastGaussianFilterConvolution(TVTestImageName);
	blurredTVImagesNames.push_back(ApplyFFT(TVTestImageName));

	ApplyHPFToBlurredImages(blurredTVImagesNames);
	ApplyFFTToBlurredImages(320, 320, blurredTVImagesNames);

	// Check if the program is attached to a debugger
	if (!::IsDebuggerPresent()) {
		// Wait for user input before closing the console window only when .exe is run
		system("pause");
	}
}
