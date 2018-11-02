// ImageDistortion.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>


template<typename _Tp> cv::Mat applyDistrotionCorrection(const cv::Mat distortedImage);
template<typename _Tp> _Tp getColorValueOfDistortedPixel(const cv::Mat distortedImage, const int xn, const int yn, const float k1, const float k2, const int cu, const int cv, const float R);
template<typename _Tp> _Tp bilinearInterpolation(const cv::Mat distortedImage, const double xDistorted, const double yDistorted);

int main()
{
	cv::Mat distortedImage = cv::imread("../images/distorted_window.jpg", CV_LOAD_IMAGE_COLOR);
    cv::Mat notDistorted = cv::Mat::zeros(distortedImage.rows, distortedImage.cols, (distortedImage.type()));
	notDistorted = applyDistrotionCorrection<cv::Vec3b>(distortedImage);

	cv::imshow("Distorted panorama", distortedImage);

	cv::imshow("not distorted panorama", notDistorted);

	cv::waitKey(0);
}


template<typename _Tp> cv::Mat applyDistrotionCorrection(const cv::Mat distortedImage) {
	const int cu = distortedImage.cols * 0.5;
	const int cv = distortedImage.rows * 0.5;
	const double R = sqrt(pow(cu, 2) + pow(cv, 2));

	const float k1 = 0.1;
	const float k2 = 0.09;

	cv::Mat newNarrowImage = cv::Mat::zeros(distortedImage.rows, distortedImage.cols, (distortedImage.type()));

	for (int y = 0; y < distortedImage.rows; y++) {
		for (int x = 0; x < distortedImage.cols; x++) {
			cv::Vec3b test2 = distortedImage.at<_Tp>(y, x);
			cv::Vec3b test = getColorValueOfDistortedPixel<_Tp>(distortedImage, x, y, k1, k2, cu, cv, R);
			newNarrowImage.at<_Tp>(y, x) = getColorValueOfDistortedPixel<_Tp>(distortedImage, x, y, k1, k2, cu, cv, R);
		}
	}
	return newNarrowImage;
}

template<typename _Tp> _Tp getColorValueOfDistortedPixel(const cv::Mat distortedImage, const int xn, const int yn, const float k1, const float k2, const int cu, const int cv, const float R) {
	const double xshift = (xn - cu) / R;
	const double yshift = (yn - cv) / R;
	const double rsquare = pow(xshift, 2) + pow(yshift, 2);
	const double radialApproximationDistortion = 1 + k1 * rsquare + k2 * pow(rsquare, 2);

	const double xdistorted = (xn - cu) / radialApproximationDistortion + cu;
	const double ydistorted = (yn - cv) / radialApproximationDistortion + cv;

	return bilinearInterpolation<_Tp>(distortedImage, xdistorted, ydistorted);
}

template<typename _Tp> _Tp bilinearInterpolation(const cv::Mat distortedImage, const double xDistorted, const double yDistorted) {
	const double x1 = floor(xDistorted);
	const double y1 = floor(yDistorted);
	const double x2 = ceil(xDistorted);
	const double y2 = ceil(yDistorted);

	const cv::Vec3b f_00 = distortedImage.at<cv::Vec3b>(y1, x1);
	const cv::Vec3b f_01 = distortedImage.at<cv::Vec3b>(y1, x2);
	const cv::Vec3b f_10 = distortedImage.at<cv::Vec3b>(y2, x1);
	const cv::Vec3b f_11 = distortedImage.at<cv::Vec3b>(y2, x2);
	const double x_ = xDistorted - x1;
	const double y_ = yDistorted - y1;

	return (f_00 * (1 - x_) * (1 - y_)) + (f_01 * x_ * (1 - y_)) + (f_10 * (1 - x_) * y_) + (f_11 * x_ * y_);
}