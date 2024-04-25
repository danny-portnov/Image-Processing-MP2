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


//Applies Gaussian filtration to the source image in both x and y directions.
void DoGaussianFiltration(c2dByteGrayImage& source, double filter[], int filterHalfSize)
{
	// Get the number of rows and columns in the source image
	int numRows = source.NumberOfRows();
	int numColumns = source.NumberOfColumns();

	// Create a temporary destination image with the same dimensions as the source image
	c2dByteGrayImage temp_dest(numRows, numColumns);
	double summa;

	// Apply filtration in the x direction
	for (int row = 0; row < numRows; row++)
	{
		for (int column = filterHalfSize; column < numColumns - filterHalfSize; column++)
		{
			summa = 0;
			// Convolution operation along the x direction
			for (int x = -filterHalfSize; x <= filterHalfSize; x++)
			{
				summa = summa + filter[filterHalfSize + x] * (double)source.GetPixelValue(row, column + x);
			} // of x
			// Clamp the result to the range [0, 255]
			if (summa < 0)
				summa = 0;
			if (summa > 255)
				summa = 255;
			// Store the result in the temporary destination image
			temp_dest.SetPixelValue(row, column, (tByte)(summa + 0.5));
		} // of column
	} // of row

	// Clear the source image
	source.Clear();

	// Apply filtration in the y direction
	for (int row = filterHalfSize; row < numRows - filterHalfSize; row++)
	{
		for (int column = 0; column < numColumns; column++)
		{
			summa = 0;
			// Convolution operation along the y direction
			for (int y = -filterHalfSize; y <= filterHalfSize; y++)
			{
				summa += filter[filterHalfSize + y] * (double)temp_dest.GetPixelValue(row + y, column);
			} // of y
			// Clamp the result to the range [0, 255]
			if (summa < 0)
				summa = 0;
			if (summa > 255)
				summa = 255;
			// Store the result back in the source image
			source.SetPixelValue(row, column, (tByte)(summa + 0.5));
		} // of column
	} // of row
}

//Creates a 2D Gaussian filter and stores it in the given image.
void CreateGaussianFilter(c2dFloatGrayImage& image, tFloat sigmaRadius)
{
	tFloat x, y, r, z;
	double summa = 0;
	int numRows = image.NumberOfRows();
	int numColumns = image.NumberOfColumns();

	// Iterate over each pixel in the image
	for (int row = 0; row < numRows; row++)
	{
		for (int column = 0; column < numColumns; column++)
		{
			// Calculate the distances from the center of the image
			y = row - numRows / 2;
			x = column - numColumns / 2;
			// Calculate the radial distance
			r = sqrt(y * y + x * x);
			// Calculate the Gaussian function value based on the distance and sigma
			z = r / sigmaRadius;
			z = exp(-z * z);
			// Accumulate the sum of Gaussian values for normalization
			summa += z;
			// Store the Gaussian value in the image
			image.SetPixelValue(row, column, z);
		}
	}
}


// Applies a convolution filter to the source image and stores the result in the destination image. 
void DoFiltrationByConvolution(c2dByteGrayImage& source, c2dByteGrayImage& dest, c2dFloatGrayImage& filter, double factor)
{
	int filterHalfSize = filter.NumberOfRows() / 2;
	// Clear the destination image
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
					// Perform the convolution operation
					summa += filter.GetPixelValue(filterHalfSize + y, filterHalfSize + x) * (double)source.GetPixelValue(row + y, column + x);
				} // of x
			} // of y
			summa = summa * factor;
			// Clamp the result to the range [0, 255]
			if (summa < 0) summa = 0;
			if (summa > 255) summa = 255;
			dest.SetPixelValue(row, column, (tByte)(summa + 0.5));
		} // of column
	} // of row
}

// Creates an inverse Gaussian filter and stores it in the given image.
void CreateInverseGaussianFilter(c2dFloatGrayImage& image, tFloat sigmaRadius)
{
	tFloat x, y, z, total;
	double summa = 0;
	int numRows = image.NumberOfRows();
	int numColumns = image.NumberOfColumns();
	for (int row = 0; row < numRows; row++)
	{
		for (int column = 0; column < numColumns; column++)
		{
			// Calculate squared distances from the center of the image
			y = (row - numRows / 2) * (row - numRows / 2);
			x = (column - numColumns / 2) * (column - numColumns / 2);
			// Calculate the total squared distance divided by sigma squared
			total = (x + y) / (sigmaRadius * sigmaRadius);
			// Apply the inverse Gaussian function and scaling
			z = 255.0 * (1 - exp(-total));
			image.SetPixelValue(row, column, z);
		}
	}
}


string AddBMPExtention(string rawName)
{
	return rawName + ".BMP";
}


// Applies fast Gaussian filter convolution to the input image using pre-defined filter kernels.
vector<string> ApplyFastGaussianFilterConvolution(string inputFileName)
{
	// Load the input image
	c2dByteGrayImage source(AddBMPExtention(inputFileName).c_str());
	// Define Gaussian filter kernels
	double LPF1[5] = { 0.061, 0.242, 0.383, 0.242, 0.061 };
	double LPF2[7] = { 0.006, 0.061, 0.242, 0.383, 0.242, 0.061, 0.006 };
	// Apply Gaussian filtration using the first kernel LPF1
	DoGaussianFiltration(source, LPF1, 2);
	string LPF1Suffix = "LPF5c"; 
	string LPF2Suffix = "LPF7c"; 
	vector<string> outputFileNames = {
		inputFileName + LPF1Suffix, 
		inputFileName + LPF2Suffix  
	};
	source.SaveToGrayBmpFile(AddBMPExtention(outputFileNames[0]).c_str());
	source.Init(AddBMPExtention(inputFileName).c_str());
	// Apply Gaussian filtration using the second kernel LPF2
	DoGaussianFiltration(source, LPF2, 3);
	source.SaveToGrayBmpFile(AddBMPExtention(outputFileNames[1]).c_str());
	return outputFileNames;
}




// Applies Fast Fourier Transform (FFT) to the input image and filters it using a Gaussian filter.
c2dFloatGrayImage ApplyFFT(string inputFileName, bool saveFilter,
	bool reverseFilter, int radius)
{
    // Load the input image
	c2dFloatGrayImage imageRe(AddBMPExtention(inputFileName).c_str()); 
	c2dFloatGrayImage imageIm(imageRe.NumberOfRows(), imageRe.NumberOfColumns());   
    // Create a Gaussian filter image with the same dimensions as the input image
	c2dFloatGrayImage gaussFilter(imageRe.NumberOfRows(), imageRe.NumberOfColumns());  
	CreateGaussianFilter(gaussFilter, radius);  
	string GaussianName = "GaussFilter.bmp";    
	if (reverseFilter)
	{
		CreateInverseGaussianFilter(gaussFilter, radius);
		GaussianName = "InversedGaussFilter.bmp";
	}   
    // Shift both real and imaginary part images
	ShiftHalfSize(imageRe);
	ShiftHalfSize(imageIm);   
    // Perform Forward FFT on the images
	DoFFT(imageRe, imageIm, FORWARD_FFT, NORMALIZE_BY_SQRT);
    // Filter the image in the frequency domain
	DoFiltrationInFD(imageRe, imageIm, gaussFilter); 
    // Perform Inverse FFT on the filtered images
	DoFFT(imageRe, imageIm, REVERSE_FFT, NORMALIZE_BY_SQRT);   
    // Shift both real and imaginary part images
	ShiftHalfSize(imageRe);
	ShiftHalfSize(imageIm);
	return imageRe;
}


// Applies a Gaussian filter in the frequency domain
//  to the given image and saves the filtered image.
// returns the filtered image name with added "LPFF" suffix
string ApplyFFTAndSave(string inputFileName,
	bool saveFilter, bool reverseFilter, int radius)
{
	string outputFileName = inputFileName + "LPFF";
	c2dFloatGrayImage imageRe = ApplyFFT(inputFileName, saveFilter, reverseFilter, radius);
	imageRe.SaveAsOptimalGrayBmpFile(AddBMPExtention(outputFileName).c_str());
	return outputFileName;
}

void Fill3X3Filter() {

}

//Applies High Pass Filters (HPF) to the blurred images specified by the input filenames.
void ApplyHPFToBlurredImages(vector<string> inputFileNames)
{
	// Define High Pass Filter kernels
	double HPF11[3][3] = { {-2, -2, -2}, {-2, 17, -2}, {-2, -2, -2} };
	c2dFloatGrayImage HPF1(3, 3);
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			HPF1.SetPixelValue(i, j, HPF11[i][j]);
		}
	}
	c2dFloatGrayImage HPF2(3, 3);
	double HPF22[3][3] = { {-8, -16 , -8}, {-16, 112 , -16}, {-8, -16 , -8} };
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			HPF2.SetPixelValue(i, j, HPF22[i][j]);
		}
	}
	// Define normalization factor for HPF2
	double HTP2factor = 1.0 / 16.0;
	int fileNamesCount = inputFileNames.size();
	for (int i = 0; i < fileNamesCount; i++)
	{
		// Load the input image
		c2dByteGrayImage source(AddBMPExtention(inputFileNames[i]).c_str());
		c2dByteGrayImage dest(source.NumberOfRows(), source.NumberOfColumns());
		// Apply HPF1 to the source image
		DoFiltrationByConvolution(source, dest, HPF1, 1);
		dest.SaveToGrayBmpFile(AddBMPExtention("HPF1c" + inputFileNames[i]).c_str());
		//// Apply HPF2 to the source image
		//DoFiltrationByConvolution(source, dest, HPF2, HTP2factor);
		//dest.SaveToGrayBmpFile(AddBMPExtention("HPF2c" + inputFileNames[i]).c_str());
	}
}


// Applies Fast Fourier Transform (FFT) to the blurred images specified by the input filenames and saves the filtered images.
void ApplyFFTToBlurredImages(vector<string> inputFileNames, int radius)
{
	int fileNamesCount = inputFileNames.size();

	for (int i = 0; i < fileNamesCount; i++)
	{
		c2dFloatGrayImage imageRe = ApplyFFT(inputFileNames[i], false, true, radius);
		string outputFileName = AddBMPExtention("HPFf" + inputFileNames[i]);
		imageRe.SaveAsOptimalGrayBmpFile(outputFileName.c_str());
	}
}


void main()
{
	//Question 2
	cout << "Beginning question 2." << endl;
	string grayImageName = "Tim1";
	vector<string> blurredImagesNames = ApplyFastGaussianFilterConvolution(grayImageName);
	int radiusForTextImage = 80;
	//Question 3
	cout << "Finished question 2. Beginning question 3." << endl;
	blurredImagesNames.push_back(ApplyFFTAndSave(
		grayImageName, true, false, radiusForTextImage));

	//Question 4
	cout << "Finished question 3. Beginning question 4." << endl;
	ApplyHPFToBlurredImages(blurredImagesNames);

	//Question 5
	cout << "Finished question 4. Beginning question 5." << endl;
	ApplyFFTToBlurredImages(blurredImagesNames, radiusForTextImage);

	//Question 7
	cout << "Finished question 5. Beginning question 7." << endl;
	int radiusForTV = 40;
	string TVTestImageName = "Tim2";
	vector<string> blurredTVImagesNames = ApplyFastGaussianFilterConvolution(TVTestImageName);
	blurredTVImagesNames.push_back(ApplyFFTAndSave(TVTestImageName, true, false, radiusForTV));

	ApplyHPFToBlurredImages(blurredTVImagesNames);
	ApplyFFTToBlurredImages(blurredTVImagesNames, radiusForTV);

	system("pause");
}
