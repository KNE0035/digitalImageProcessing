// ImageDistortion.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>


template<typename _Tp> cv::Mat doHistogramEqualization(cv::Mat originalImage, int histogramDistinction);
template<typename _Tp > int* getHistogramOfImage(cv::Mat originalImage, int histogramDistinction);
int* getCumulativeHistogram(int* histogram, int histogramDistinction);
template<typename _Tp> _Tp getLowestBrightness(cv::Mat originalImage);


template<typename _Tp> void printArray(_Tp* array, int length) {
	for (int i = 0; i < length; i++) {
		printf("%d", array[i]);
		printf("\n");
	}
}

template<typename _Tp> _Tp getLowestBrightness(cv::Mat originalImage) {
	_Tp lowestBrightess = 255; //255 bightness is maximum
	for (int i = 0; i < originalImage.rows; i++) {
		for (int j = 0; j < originalImage.cols; j++) {
			if (lowestBrightess > originalImage.at<_Tp>(i, j)) {
				lowestBrightess = originalImage.at<_Tp>(i, j);
			}
		}
	}
	return lowestBrightess;
}

template<typename _Tp> cv::Mat doHistogramEqualization(cv::Mat originalImage, int histogramDistinction) {
	cv::Mat equalizedImage = cv::Mat::zeros(originalImage.rows, originalImage.cols, (originalImage.type()));
	
	int* histgramOfImage = getHistogramOfImage<_Tp>(originalImage, histogramDistinction);
	int* cumulativeHistogram = getCumulativeHistogram(histgramOfImage, histogramDistinction);
	
	int cdfMin = cumulativeHistogram[getLowestBrightness<_Tp>(originalImage)];

	for (int i = 0; i < originalImage.rows; i++) {
		for (int j = 0; j < originalImage.cols; j++) {
			equalizedImage.at<_Tp>(i, j) = round((((cumulativeHistogram[originalImage.at<_Tp>(i, j)] - cdfMin)) / double(originalImage.cols * originalImage.rows - cdfMin)) * (histogramDistinction - 1));
		}
	}
	delete histgramOfImage;
	delete cumulativeHistogram;
	return equalizedImage;
}

template<typename _Tp> int* getHistogramOfImage(cv::Mat originalImage, int histogramDistinction) {
	int* histogram = new int[histogramDistinction];

	for (int i = 0; i < histogramDistinction; i++) {
		histogram[i] = 0;
	}

	for (int i = 0; i < originalImage.rows; i++) {
		for (int j = 0; j < originalImage.cols; j++) {
			histogram[originalImage.at<_Tp>(i, j)]++;
		}
	}
	return histogram;
}

int* getCumulativeHistogram(int* histogram, int histogramDistinction) {
	int* cumulativeHistogram = new int[histogramDistinction];
	
	for (int i = 0; i < histogramDistinction; i++) {
		cumulativeHistogram[i] = 0;
	}
	
	for (int i = 0; i < histogramDistinction; i++) {
		for (int j = 0; j <= i; j++) {			
			cumulativeHistogram[i] = cumulativeHistogram[i] + histogram[j];
		}
	}

	return cumulativeHistogram;
}

int main()
{
	cv::Mat originalImage = cv::imread("../images/uneq.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat equalizedImage = cv::Mat::zeros(originalImage.rows, originalImage.cols, (originalImage.type()));

	equalizedImage = doHistogramEqualization<uchar>(originalImage, 256);
	cv::imshow("original image", originalImage);
	cv::imshow("equalized image", equalizedImage);

	cv::waitKey(0);
}



