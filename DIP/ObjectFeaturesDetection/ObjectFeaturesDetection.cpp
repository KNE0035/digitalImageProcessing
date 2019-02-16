// ObjectFeaturesDetection.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>

cv::Mat doBackgroundThresholding(cv::Mat image, int threshold);
cv::Mat doIndexingOfObjects(cv::Mat thresholdedImage);
void floodFill(cv::Mat *thresholdedImage, cv::Mat *alreadyVisitedMat, int nextIndex, int x, int y);
void analyzeObjectInImg(cv::Mat image);

int main()
{
	cv::Mat image = cv::imread("../images/train.png", CV_LOAD_IMAGE_GRAYSCALE);
	cv::imshow("original", image);
	analyzeObjectInImg(image);

	cv::waitKey(0);
}

cv::Mat doBackgroundThresholding(cv::Mat image, int threshold) {
	cv::Mat thresholdedImage = cv::Mat::zeros(image.rows, image.cols, CV_8UC1);
	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			if (image.at<uchar>(y, x) >= threshold) {
				thresholdedImage.at<uchar>(y, x) = 255;
			}
			else {
				thresholdedImage.at<uchar>(y, x) = 0;
			}
		}
	}

	return thresholdedImage;
}

void analyzeObjectInImg(cv::Mat image) { 
	cv::Mat thresholdedImage =  doBackgroundThresholding(image, 127);
	cv::Mat indexedImage = doIndexingOfObjects(thresholdedImage);

	cv::imshow("bacground threshold", thresholdedImage);
	cv::imshow("indexed image", indexedImage);
}

cv::Mat doIndexingOfObjects(cv::Mat thresholdedImage) {
	cv::Mat indexedImage = thresholdedImage.clone();
	cv::Mat alreadyVisitedMat = cv::Mat::zeros(thresholdedImage.rows, thresholdedImage.cols, CV_8UC1);
	int nextIndex = 1;

	for (int y = 0; y < thresholdedImage.rows; y++) {
		for (int x = 0; x < thresholdedImage.cols; x++) {
			if (thresholdedImage.at<uchar>(y, x) != 0 && alreadyVisitedMat.at<uchar>(y, x) != 1) {
				floodFill(&indexedImage, &alreadyVisitedMat, nextIndex, x, y);
				nextIndex++;
			}
		}
	}
	return indexedImage;
}

void floodFill(cv::Mat *indexedImage, cv::Mat *alreadyVisitedMat, int nextIndex, int x, int y) {
	if (x < 0 || x > indexedImage->cols - 1 || y < 0 || y > indexedImage->rows - 1) {
		return;
	}

	if (indexedImage->at<uchar>(y, x) == 0 || alreadyVisitedMat->at<uchar>(y, x) == 1) {
		return;
	}

	indexedImage->at<uchar>(y, x) = nextIndex * 19;
	alreadyVisitedMat->at<uchar>(y, x) = 1;

	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x - 1, y - 1);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x + 1, y - 1);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x - 1, y + 1);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x + 1, y + 1);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x, y - 1);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x, y + 1);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x - 1, y);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x + 1, y);
}