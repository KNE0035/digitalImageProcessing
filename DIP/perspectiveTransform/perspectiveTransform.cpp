// ImageDistortion.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>

cv::Mat doPerspectiveTransform(cv::Mat insertingImage, cv::Mat baseImage, cv::Point leftUp, cv::Point rightUp, cv::Point rightDown, cv::Point leftDown);


cv::Mat buildPerspectiveMatrix(cv::Mat insertingImage, cv::Point leftUp, cv::Point rightUp, cv::Point rightDown, cv::Point leftDown);

cv::Mat buildPerspectiveMatrix(cv::Mat insertingImage, cv::Point leftUp, cv::Point rightUp, cv::Point rightDown, cv::Point leftDown) {
	cv::Mat perspectiveMatrix = cv::Mat::zeros(3, 3, CV_32F);
	cv::Point pointsInsert[4];
	cv::Point pointsBase[4];
	int nOEquation = 8;
	cv::Mat aMat = cv::Mat::zeros(nOEquation, 5, CV_32F);
	cv::Mat bMat = cv::Mat::zeros(nOEquation, 1, CV_32F);
	cv::Mat resultVector;

	pointsBase[0] = leftUp;
	pointsBase[1] = rightUp;
	pointsBase[2] = rightDown;
	pointsBase[3] = leftDown;

	pointsInsert[0] = cv::Point(0, 0);
	pointsInsert[1] = cv::Point(insertingImage.cols - 1, 0);
	pointsInsert[2] = cv::Point(insertingImage.cols - 1, insertingImage.rows - 1);
	pointsInsert[3] = cv::Point(0, insertingImage.cols - 1);
 
	perspectiveMatrix.at<int>(1, 0) = 1;

	for (int i = 0; i < nOEquation; i++) {
		
		
		aMat.at<float>(i, 0) = pointsBase[nOEquation].y;
		aMat.at<float>(i, 1) = 1;
		aMat.at<float>(i, 2) = -pointsInsert[nOEquation].x * pointsBase[nOEquation].x;
		aMat.at<float>(i, 3) = -pointsInsert[nOEquation].x * pointsBase[nOEquation].y;
		aMat.at<float>(i, 4) = -pointsInsert[nOEquation].x;

		bMat.at<float>(i, 0) = nOEquation % 2 == 0 ? -pointsBase[nOEquation].x : 0;
	}

	cv::solve(aMat, aMat, resultVector, cv::DECOMP_SVD);
	
	std::cout << "Result: " << resultVector << std::endl;

 	return perspectiveMatrix;
}

cv::Mat doPerspectiveTransform(cv::Mat insertingImage, cv::Mat baseImage, cv::Point leftUp, cv::Point rightUp, cv::Point rightDown, cv::Point leftDown) {
	cv::Mat perspectiveTrasformImage = cv::Mat::zeros(3, 3, CV_32F);

	perspectiveTrasformImage = buildPerspectiveMatrix(insertingImage, leftUp, rightUp, rightDown, leftDown);


	return perspectiveTrasformImage;
}

int main()
{
	cv::Mat buildingImage = cv::imread("../images/vsb.jpg", CV_LOAD_IMAGE_COLOR);
	cv::Mat flagImage = cv::imread("../images/flag.png", CV_LOAD_IMAGE_COLOR);

	cv::Mat perspectiveTrasformImage = cv::Mat::zeros(buildingImage.rows, buildingImage.cols, (buildingImage.type()));

	perspectiveTrasformImage = doPerspectiveTransform(flagImage, buildingImage, cv::Point(69, 107), cv::Point(227, 76), cv::Point(228, 122), cv::Point(66, 134));

	printf("%d, %d", flagImage.rows, flagImage.cols);

	/*cv::imshow("original image", originalImage);
	cv::imshow("equalized image", equalizedImage);*/

	cv::waitKey(0);
}



