// ObjectFeaturesDetection.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>

void backgroundThreshold(cv::Mat image);


int main()
{
	cv::Mat image = cv::imread("../images/train.png", CV_LOAD_IMAGE_GRAYSCALE);
	


	cv::waitKey(0);
}

void backgroundThreshold(cv::Mat image, int threshold) {
	
	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			if(> threshold)
			
			newNarrowImage.at<_Tp>(y, x) = getColorValueOfDistortedPixel<_Tp>(distortedImage, x, y, k1, k2, cu, cv, R);
		}
	}
}