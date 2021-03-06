// Tomograph.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>

cv::Mat rotateImage(const cv::Mat & inImg, const double angle);
cv::Mat createImage();
double* countPixels(const cv::Mat & inImg);
cv::Mat getDigitalizedTomograph(cv::Mat originalImage);
cv::Mat sumImages(cv::Mat img1, cv::Mat img2);
cv::Mat stretchRowValuesToFullImage(int originalImageRows, int originalImageCols, double* pixelCountTable);
template <typename _Tp> void normalizeRangeOfMat(cv::Mat* mat);

cv::Mat stretchRowValuesToFullImage(int originalImageRows, int originalImageCols, double* pixelCountTable) {
	cv::Mat resultImage = cv::Mat(originalImageRows, originalImageCols, CV_64FC1);

	for (int y = 0; y < originalImageRows; y++) {
		for (int x = 0; x < originalImageCols; x++) {
			resultImage.at<double>(y, x) = pixelCountTable[y];
		}
	}

	return resultImage;
}

template <typename _Tp> void normalizeRangeOfMat(cv::Mat* mat) {
	double min = 0, max = 0;
	cv::minMaxLoc(*mat, &min, &max);

	for (int y = 0; y < mat->rows; y++) {
		for (int x = 0; x < mat->cols; x++) {
			mat->at<_Tp>(y, x) = (mat->at<_Tp>(y, x) - min) / (max - min);
		}
	}
}

cv::Mat getDigitalizedTomograph(cv::Mat originalImage) {
	cv::Mat finalImg = cv::Mat::zeros(originalImage.rows, originalImage.cols, CV_64FC1);

	for (int i = 0; i < 360; i++) {
		cv::Mat rotatedImg = rotateImage(originalImage, i);
		double* pixelCountTable = countPixels(rotatedImg);
		cv::Mat stretchedProjectedImage = stretchRowValuesToFullImage(originalImage.rows, originalImage.cols, pixelCountTable);
		
		stretchedProjectedImage = rotateImage(stretchedProjectedImage, -i);

		finalImg = sumImages(finalImg, stretchedProjectedImage);
		delete pixelCountTable;
	}

	normalizeRangeOfMat<double>(&finalImg);
	return finalImg;
}

cv::Mat rotateImage(const cv::Mat & inImg, const double angle) {
	cv::Point center = cv::Point(inImg.cols / 2, inImg.rows / 2);
	double scale = 1.0;

	cv::Mat rotMat(2, 3, CV_32FC1);
	cv::Mat rotImg;

	rotMat = cv::getRotationMatrix2D(center, angle, scale);
	cv::warpAffine(inImg, rotImg, rotMat, inImg.size());

	return rotImg;
}

cv::Mat sumImages(cv::Mat img1, cv::Mat img2) {
	for (int y = 0; y < img1.rows; y++)
	{
		for (int x = 0; x < img1.cols; x++) {
			img1.at<double>(y, x) = img1.at<double>(y, x) + img2.at<double>(y, x);
		}
	}

	return img1;
}

double* countPixels(const cv::Mat & inImg){
	double* pixelTable = new double[inImg.rows];

	for (int i = 0; i < inImg.rows; i++) 
	{
		pixelTable[i] = 0;
	}
	
	for (int y = 0; y < inImg.rows; y++) 
	{
		for (int x = 0; x < inImg.cols; x++) {
			pixelTable[y] += inImg.at<double>(y, x);
		}
	}
	return pixelTable;
}

cv::Mat createImage() {
	cv::Mat img = cv::Mat::zeros(150, 160, CV_64FC1);
	cv::circle(img, cv::Point{ img.cols / 2, img.rows / 2 }, 50, 1);
	cv::rectangle(img, cv::Point{80, 80}, cv::Point{145, 145}, 0.4, -1);
	return img;
}


int main()
{
	cv::Mat originalImage = createImage();
	cv::Mat finalImg = getDigitalizedTomograph(originalImage);

	cv::imshow("source image", originalImage);
	cv::imshow("tomograph digital image", finalImg);
	cv::waitKey();
}
