// ImageDistortion.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>


template<typename _Tp> cv::Mat applyDistrotionCorrection(cv::Mat distortedImage, const float k1 = 0.1, const float k2 = 0.09);
template<typename _Tp> _Tp getColorValueOfDistortedPixel(cv::Mat distortedImage, int xn, int yn, float k1, float k2, int cu, int cv, const float R);
template<typename _Tp> _Tp bilinearInterpolation(cv::Mat distortedImage, cv::Point leftDown, cv::Point leftUp, cv::Point rightDown, cv::Point rightUp);

int main()
{
	cv::Mat distortedImage = cv::imread("../images/distorted_panorama.jpg", CV_LOAD_IMAGE_COLOR);
    
	// cv::Vec3b image color datatype K1 = 0.1 K2 = 0.09

	cv::imshow("Distorted panorama", distortedImage);

	cv::waitKey(0);
}


template<typename _Tp> cv::Mat applyDistrotionCorrection(cv::Mat distortedImage,const float k1,const float k2) {
	int cu = distortedImage.cols * 0.5;
	int cv = distortedImage.rows * 0.5;
	
	R = sqrt(pow(cu,2) + pow(cv, 2))

	cv::Mat newNarrowImage = cv::Mat::zeros(distortedImage.rows, distortedImage.cols, (distortedImage.type()));

	for (int y = 0; y < distortedImage.rows; y++) {
		for (int x = 0; x < distortedImage.cols; x++) {
			newNarrowImage.at<_Tp>(y, x) = getColorValueOfDistortedPixel(x, y, k1, k2, cu, cv, R);
		}
	}
	return newNarrowImage;
}

template<typename _Tp> _Tp getColorValueOfDistortedPixel(cv::Mat distortedImage, int xn, int yn, float k1, float k2, int cu, int cv, const float R) {
	int xshift, yshift, rsquare;
	double xdistorted, ydistorted;
	cv::Point leftDown, rightDown, leftUp, rightUp;

	xshift = (xn - cu) / R;
	yshift = (yn - cv) / R;

	rsquare = pow(xshift, 2) + pow(yshift, 2);

	double radialApproximationDistortion = 1 + rsquare + pow(rsquare, 2);

	xdistorted = (xn - cu) * 1 / radialApproximationDistortion + cu;
	ydistorted = (yn - cv) * 1 / radialApproximationDistortion + cv;

	leftDown.x = floor(xdistorted);
	leftDown.y = floor(ydistorted);

	rightDown.x = ceil(xdistorted);
	rightDown.y = floor(ydistorted);

	leftUp.x = floor(xdistorted);
	leftUp.y = ceil(ydistorted);

	rightUp.x = ceil(xdistorted);
	rightUp.y = ceil(ydistorted);

	return bilinearInterpolation(leftDown, leftUp, rightDown, rightUp);
}

template<typename _Tp> _Tp bilinearInterpolation(cv::Mat distortedImage, cv::Point leftDown, cv::Point leftUp, cv::Point rightDown, cv::Point rightUp) {

}