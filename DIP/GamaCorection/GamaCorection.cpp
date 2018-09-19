#include "stdafx.h"
#include <math.h>

int main()
{
	cv::Mat moon_8uc3_img = cv::imread("../images/moon.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat moon_32fc1_img;

	cv::imshow("narrow band moon", moon_8uc3_img);

	double min, max, offset;
	cv::minMaxLoc(moon_8uc3_img, &min, &max);
	offset = 255 / (max - min);

	//converting narrow band 
	for (int y = 0; y < moon_8uc3_img.rows; y++) {
		for (int x = 0; x < moon_8uc3_img.cols; x++) {
			moon_8uc3_img.at<uchar>(y, x) = (moon_8uc3_img.at<uchar>(y, x) - min) * offset;
		}
	}


	cv::imshow("moon before gama corection", moon_8uc3_img);

	moon_8uc3_img.convertTo(moon_32fc1_img, CV_32FC1, 1.0 / 255.0);
	float gama = 2;

	//gama corection
	for (int y = 0; y < moon_32fc1_img.rows; y++) {
		for (int x = 0; x < moon_32fc1_img.cols; x++) {
			moon_32fc1_img.at<float>(y, x) = pow(moon_32fc1_img.at<float>(y, x), gama);
		}
	}

	cv::imshow("moon gama corection", moon_32fc1_img);

	cv::waitKey(0); // wait until keypressed

	return 0;
}