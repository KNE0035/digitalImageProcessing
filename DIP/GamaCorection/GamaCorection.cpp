#include "stdafx.h"
#include <math.h>

template<typename _Tp> void convertNarrowBandImageToFullGrayScale(cv::Mat img);
template<typename _Tp> cv::Mat* applyGamaCorection(cv::Mat* img, float gamaCoef);

template<typename _Tp> void convertNarrowBandImageToFullGrayScale(cv::Mat img) {
	double min, max, offset;
	cv::minMaxLoc(img, &min, &max);
	offset = 255 / (max - min);
 
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			img.at<_Tp>(y, x) = (img.at<_Tp>(y, x) - min) * offset;
		}
	}
}

template<typename _Tp> cv::Mat* applyGamaCorection(cv::Mat* img, float gamaCoef) {
	if (img == nullptr) {
		return nullptr;
	}
	
	cv::Mat* FC1_32_Img = img;
	
	if (img->type() != CV_32FC1) {
		FC1_32_Img->convertTo(*FC1_32_Img, CV_32FC1, 1.0 / 255.0);
	}
	
	for (int y = 0; y < FC1_32_Img->rows; y++) {
		for (int x = 0; x < FC1_32_Img->cols; x++) {
			FC1_32_Img->at<_Tp>(y, x) = pow(FC1_32_Img->at<_Tp>(y, x), gamaCoef);
		}
	}
		
	return FC1_32_Img;
}

int main()
{
	cv::Mat moon_img = cv::imread("../images/moon.jpg", CV_LOAD_IMAGE_GRAYSCALE);

	cv::imshow("narrow band moon", moon_img);

	convertNarrowBandImageToFullGrayScale<uchar>(moon_img);
	cv::imshow("moon before gama corection", moon_img);

	moon_img = *(applyGamaCorection<float>(&moon_img, 2));
	cv::imshow("moon gama corection", moon_img);

	cv::waitKey(0); // wait until keypressed

	return 0;
}