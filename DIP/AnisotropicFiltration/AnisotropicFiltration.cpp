#include "pch.h"
#include <math.h>
#include <cmath>
#include <iostream>

template<typename _Tp> cv::Mat doAnysotropicFiltration(cv::Mat img, int numberOfIterations);
template<typename _Tp> cv::Mat doCycleOfAnysotropicFiltration(cv::Mat img);
template<typename _Tp> double calculateAnysotropicFilterationForPixel(int x, int y, cv::Mat img, double RO = 0.015, double LAMBDA = 0.1);
template<typename _Tp> void setCxIxVariables(int x, int y, int y_shifted, int x_shifted, double* Cx, double* Ix , cv::Mat img, double RO);
void convertImgTo64FC1(cv::Mat* img);

void convertImgTo64FC1(cv::Mat* img) {
	cv::Mat* FC1_64_Img = img;

	if (img->type() != CV_32FC1) {
		FC1_64_Img->convertTo(*FC1_64_Img, CV_64FC1, 1.0 / 255.0);
	}
}

template<typename _Tp> cv::Mat doAnysotropicFiltration(cv::Mat img, int numberOfIterations) {
	convertImgTo64FC1(&img);
	cv::Mat latestImg = img.clone();

	img.release();

	for (int i = 0; i < numberOfIterations; i++) {
		latestImg = doCycleOfAnysotropicFiltration<_Tp>(latestImg);
		std::cout << i << std::endl;
	}

	return latestImg;
}

template<typename _Tp> cv::Mat doCycleOfAnysotropicFiltration(cv::Mat img) {
	cv::Mat newImage = img.clone();
	for (int y = 1; y < img.rows - 1; y++) {
		for (int x = 1; x < img.cols - 1; x++) {
			newImage.at<_Tp>(y, x) = calculateAnysotropicFilterationForPixel<_Tp>(x, y, img);
		}
	}

	return newImage;
}

template<typename _Tp> double calculateAnysotropicFilterationForPixel(int x, int y, cv::Mat img, double SIGMA, double LAMBDA) { 
	double Cn, Cs, Ce, Cw, In, Is, Ie, Iw;

	setCxIxVariables<_Tp>(x, y, y - 1, x, &Cn, &In, img, SIGMA);
	setCxIxVariables<_Tp>(x, y, y + 1, x, &Cs, &Is, img, SIGMA);
	setCxIxVariables<_Tp>(x, y, y, x + 1, &Ce, &Ie, img, SIGMA);
	setCxIxVariables<_Tp>(x, y, y, x - 1, &Cw, &Iw, img, SIGMA);

	return img.at<_Tp>(y, x) * (1 - LAMBDA * (Cn + Cs + Ce + Cw)) + LAMBDA * (Cn * In + Cs * Is + Ce * Ie + Cw * Iw);
}

template<typename _Tp> void setCxIxVariables(int x, int y, int y_shifted, int x_shifted, double* Cx, double* Ix, cv::Mat img, double SIGMA) {
	*Cx = std::exp(-(pow(img.at<_Tp>(y_shifted, x_shifted) - img.at<_Tp>(y, x), 2) / pow(SIGMA, 2)));
	*Ix = img.at<_Tp>(y_shifted, x_shifted);
}

int main()
{
	cv::Mat valveImg = cv::imread("../images/valve.png", CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat testImg = valveImg.clone();
	cv::imshow("before anysotropic filtration", valveImg);

	valveImg = doAnysotropicFiltration<double>(valveImg, 250);

	cv::imshow("after anysotropic filtration", valveImg);

	cv::waitKey(0); // wait until keypressed

	return 0;
}