// HistogramOfOrientedGradients.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>

constexpr int BLOCK_DIM_X = 2;
constexpr int BLOCK_DIM_Y = 2;

constexpr int CELL_DIM_X = 8;
constexpr int CELL_DIM_Y = 8;

constexpr int BINS = 8;


struct GradientInfo {
	cv::Mat gradientOrientationMat;
	cv::Mat gradientSizeMat;
};

float* getImgHOGVector(cv::Mat img);
GradientInfo getImgGradientInfo(cv::Mat img);
float* getHistogramsPerBlock(GradientInfo gradientInfo, int blockTopCoordX, int blockTopCoordY);
float* getHistogramInCell(GradientInfo gradientInfo, int blockTopCoordX, int blockTopCoordY, int cellCoordX, int cellCoordY);


int main()
{
	cv::Mat img = cv::imread("../images/hog_test.png", CV_LOAD_IMAGE_GRAYSCALE);
	float* imgHOGVector = getImgHOGVector(img);

	/*cv::imshow("image", img);
	cv::waitKey(0);*/
}


float* getImgHOGVector(cv::Mat img) {
	GradientInfo gradientInfo = getImgGradientInfo(img);
	int nOBlocksX = img.cols - BLOCK_DIM_X * CELL_DIM_X + 1;
	int nOBlocksY = img.rows - BLOCK_DIM_Y * CELL_DIM_Y + 1;
	float* histograms = new float[BLOCK_DIM_X * BLOCK_DIM_Y * nOBlocksX * nOBlocksY * BINS];

	for (int i = 0; i < nOBlocksY; i++) {
		for (int j = 0; j < nOBlocksX; j++) {
			memcpy(histograms, getHistogramsPerBlock(gradientInfo, j, i), BLOCK_DIM_X * BLOCK_DIM_Y * BINS * sizeof(float));
			histograms += (BLOCK_DIM_X * BLOCK_DIM_Y * BINS);
		}
	}

	return histograms;
}

float* getHistogramsPerBlock(GradientInfo gradientInfo, int blockTopCoordX, int blockTopCoordY) {
	const int histogramsPerBlockLength = BLOCK_DIM_X * BLOCK_DIM_Y * BINS;
	float histogramsPerBlock[histogramsPerBlockLength];
	float* histogramNextPointer = histogramsPerBlock;
	
	/*if (blockTopCoordX == 14 && blockTopCoordY == 0) {
		printf("as");
	}*/

	for (int i = 0; i < BLOCK_DIM_Y; i++) {
		for (int j = 0; j < BLOCK_DIM_X; j++) {
			memcpy(histogramNextPointer, getHistogramInCell(gradientInfo, blockTopCoordX, blockTopCoordY, j, i), BINS * sizeof(float));
			histogramNextPointer += BINS;
		}
	}
	
	//normalize over block
	float l2Norm = 0;

	for (int i = 0; i < histogramsPerBlockLength; i++) {
		l2Norm += histogramsPerBlock[i] * histogramsPerBlock[i];
	}
	l2Norm = sqrtf(l2Norm);

	if (l2Norm != 0) {
		for (int i = 0; i < histogramsPerBlockLength; i++) {
			histogramsPerBlock[i] /= l2Norm;
		}
	}
	
	return histogramsPerBlock;
}

float* getHistogramInCell(GradientInfo gradientInfo, int blockTopCoordX, int blockTopCoordY, int cellCoordX, int cellCoordY) {
	float histogram[BINS];
	std::fill(histogram, histogram + BINS, 0);
	const int binWidth = 360 / BINS;

	for (int i = 0; i < CELL_DIM_Y; i++) {
		for (int j = 0; j < CELL_DIM_X; j++) {
			int yOffset = blockTopCoordY + CELL_DIM_Y * cellCoordY + i;
			int xOffset = blockTopCoordX + CELL_DIM_X * cellCoordX + j;

			int binIndex = gradientInfo.gradientOrientationMat.at<float>(yOffset, xOffset) / binWidth;
			histogram[binIndex] += gradientInfo.gradientSizeMat.at<float>(yOffset, xOffset);
		}
	}
	return histogram;
}

GradientInfo getImgGradientInfo(cv::Mat img) {
	GradientInfo gradientInfo;
	gradientInfo.gradientOrientationMat = cv::Mat::zeros(img.rows, img.cols, CV_32FC1);
	gradientInfo.gradientSizeMat = cv::Mat::zeros(img.rows, img.cols, CV_32FC1);

	for (int y = 1; y < img.rows - 1; y++) {
		for (int x = 1; x < img.cols - 1; x++) {
			float fx = (double)img.at<uchar>(y, x + 1) - img.at<uchar>(y, x);
			float fy = (double)img.at<uchar>(y + 1, x) - img.at<uchar>(y, x);

			gradientInfo.gradientOrientationMat.at<float>(y, x) = atan2f(fy, fx);
			gradientInfo.gradientSizeMat.at<float>(y, x) = sqrtf(fx * fx + fy * fy);
		}
	}
	return gradientInfo;
}