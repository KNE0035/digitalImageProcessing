// ImageDistortion.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>


template<typename _Tp> cv::Mat applyDistrotionCorrection(cv::Mat distortedImage, const float k1 = 0.01, const float k2 = 0.009);
template<typename _Tp> _Tp getColorValueOfDistortedPixel(cv::Mat distortedImage, int xn, int yn, float k1, float k2, int cu, int cv, const float R);
template<typename _Tp> _Tp bilinearInterpolation(cv::Mat distortedImage, double xDistorted, double yDistorted);

int main()
{
	cv::Mat distortedImage = cv::imread("../images/distorted_panorama.jpg", CV_LOAD_IMAGE_COLOR);
    cv::Mat notDistorted = cv::Mat::zeros(distortedImage.rows, distortedImage.cols, (distortedImage.type()));
	// cv::Vec3b image color datatype K1 = 0.1 K2 = 0.09
	notDistorted = applyDistrotionCorrection<cv::Vec3b>(distortedImage);

	for (int y = 0; y < distortedImage.rows; y++) {
		for (int x = 0; x < distortedImage.cols; x++) {
			cv::Vec3b test = distortedImage.at<cv::Vec3b>(y, x);
			cv::Vec3b test2 = notDistorted.at<cv::Vec3b>(y, x);
			printf("%d ", test[0]-  test2[0]);
			printf("%d ", test[1] - test2[1]);
			printf("%d ", test[2] - test2[2]);
			printf("\n");
		}
	}

	cv::imshow("Distorted panorama", distortedImage);

	cv::imshow("not distorted panorama", notDistorted);

	cv::waitKey(0);
}


template<typename _Tp> cv::Mat applyDistrotionCorrection(cv::Mat distortedImage,const float k1,const float k2) {
	int cu = distortedImage.cols * 0.5;
	int cv = distortedImage.rows * 0.5;
	double R;
	R = sqrt(pow(cu, 2) + pow(cv, 2));

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

template<typename _Tp> _Tp getColorValueOfDistortedPixel(cv::Mat distortedImage, int xn, int yn, float k1, float k2, int cu, int cv, const float R) {
	double xshift, yshift, rsquare;
	double xdistorted, ydistorted;

	xshift = (xn - cu) / R;
	yshift = (yn - cv) / R;

	rsquare = pow(xshift, 2) + pow(yshift, 2);

	double radialApproximationDistortion = 1 + k1 * rsquare + k2 * pow(rsquare, 2);

	xdistorted = (xn - cu) / radialApproximationDistortion + cu;
	ydistorted = (yn - cv) / radialApproximationDistortion + cv;
	//return distortedImage.at<_Tp>(ceil(ydistorted), ceil(xdistorted));
	cv::Vec3b test = distortedImage.at<_Tp>(xn, yn);
	cv::Vec3b test2 = bilinearInterpolation<_Tp>(distortedImage, xdistorted, ydistorted);

	printf("%d : %d, ", test[0], test2[0]);
	printf("%d : %d, ", test[1], test2[1]);
	printf("%d : %d, ", test[2], test2[2]);
	printf("\n");

	return bilinearInterpolation<_Tp>(distortedImage, xdistorted, ydistorted);
}

template<typename _Tp> _Tp bilinearInterpolation(cv::Mat distortedImage, double xDistorted, double yDistorted) {
	double x1 = floor(xDistorted);
	double y1 = floor(yDistorted);
	double x2 = ceil(xDistorted);
	double y2 = ceil(yDistorted);

	cv::Vec3b f_00 = distortedImage.at<cv::Vec3b>(y1, x1);
	cv::Vec3b f_01 = distortedImage.at<cv::Vec3b>(y1, x2);
	cv::Vec3b f_10 = distortedImage.at<cv::Vec3b>(y2, x1);
	cv::Vec3b f_11 = distortedImage.at<cv::Vec3b>(y2, x2);
	double x_ = xDistorted - x1;
	double y_ = yDistorted - y1;

	return (f_00 * (1 - x_) * (1 - y_)) + (f_01 * x_ * (1 - y_)) + (f_10 * (1 - x_) * y_) + (f_11 * x_ * y_);
}