// ImageDistortion.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>

cv::Mat doPerspectiveTransform(cv::Mat insertingImage, cv::Mat baseImage, cv::Point leftUp, cv::Point rightUp, cv::Point rightDown, cv::Point leftDown);


cv::Mat buildPerspectiveMatrix(cv::Mat insertingImage, cv::Point leftUp, cv::Point rightUp, cv::Point rightDown, cv::Point leftDown);
void fillABMatrticesForPersepctiveTransform(cv::Point* pointsInsert, cv::Point* pointsBase, cv::Mat &aMat, cv::Mat &bMat);
cv::Mat get3x3PerspectiveMatrix(cv::Mat resultVecor);

cv::Mat get3x3PerspectiveMatrix(cv::Mat resultVecor) {
	cv::Mat perspectiveMatrix = cv::Mat(3, 3, CV_64F);
	
	perspectiveMatrix.at<double>(0) = 1.0;

	for (int i = 1; i < 9; i++) {
		perspectiveMatrix.at<double>(i) = resultVecor.at<double>(i - 1);
	}

	return perspectiveMatrix;
}

void fillABMatrticesForPersepctiveTransform(cv::Point* pointsInsert, cv::Point* pointsBase, cv::Mat &aMat, cv::Mat &bMat) {
	int nOEquation = 8;

	for (int i = 0, j = 0; i < nOEquation; i = i + 2, j++) {

		aMat.at<double>(i, 0) = pointsBase[j].y;
		aMat.at<double>(i, 1) = 1;
		aMat.at<double>(i, 2) = 0;
		aMat.at<double>(i, 3) = 0;
		aMat.at<double>(i, 4) = 0;
		aMat.at<double>(i, 5) = -pointsInsert[j].x * pointsBase[j].x;
		aMat.at<double>(i, 6) = -pointsInsert[j].x * pointsBase[j].y;
		aMat.at<double>(i, 7) = -pointsInsert[j].x;
		
		bMat.at<double>(i, 0) = -pointsBase[j].x;

		aMat.at<double>(i + 1, 0) = 0;
		aMat.at<double>(i + 1, 1) = 0;
		aMat.at<double>(i + 1, 2) = pointsBase[j].x;
		aMat.at<double>(i + 1, 3) = pointsBase[j].y;
		aMat.at<double>(i + 1, 4) = 1;
		aMat.at<double>(i + 1, 5) = -pointsInsert[j].y * pointsBase[j].x;
		aMat.at<double>(i + 1, 6) = -pointsInsert[j].y * pointsBase[j].y;
		aMat.at<double>(i + 1, 7) = -pointsInsert[j].y;

		bMat.at<double>(i + 1, 0) = 0;
	}
}

cv::Mat buildPerspectiveMatrix(cv::Mat insertingImage, cv::Point leftUp, cv::Point rightUp, cv::Point rightDown, cv::Point leftDown) {
	int nOEquation = 8;
	
	cv::Point pointsInsert[4];
	cv::Point pointsBase[4];
	cv::Mat aMat = cv::Mat::zeros(nOEquation, nOEquation, CV_64F);
	cv::Mat bMat = cv::Mat::zeros(nOEquation, 1, CV_64F);
	cv::Mat resultVector;

	pointsBase[0] = leftUp;
	pointsBase[1] = rightUp;
	pointsBase[2] = rightDown;
	pointsBase[3] = leftDown;

	pointsInsert[0] = cv::Point(0, 0);
	pointsInsert[1] = cv::Point(insertingImage.cols - 1, 0);
	pointsInsert[2] = cv::Point(insertingImage.cols - 1, insertingImage.rows - 1);
	pointsInsert[3] = cv::Point(0, insertingImage.rows - 1);
 
	fillABMatrticesForPersepctiveTransform(pointsInsert, pointsBase, aMat, bMat);

	cv::solve(aMat, bMat, resultVector);

	return get3x3PerspectiveMatrix(resultVector);
}

cv::Mat doPerspectiveTransform(cv::Mat insertingImage, cv::Mat baseImage, cv::Point leftUp, cv::Point rightUp, cv::Point rightDown, cv::Point leftDown) {
	cv::Mat perspectiveTrasformMatrix = cv::Mat::zeros(3, 3, CV_32F);
	cv::Mat resultImage = baseImage.clone();

	perspectiveTrasformMatrix = buildPerspectiveMatrix(insertingImage, leftUp, rightUp, rightDown, leftDown);

	for (int y = 0; y < resultImage.rows; y++) {
		for (int x = 0; x < resultImage.cols; x++) {
			cv::Mat baseImageCords = cv::Mat(3, 1, CV_64F);
			baseImageCords.at<double>(0) = x;
			baseImageCords.at<double>(1) = y;
			baseImageCords.at<double>(2) = 1;


			cv::Mat flagImageCoords = perspectiveTrasformMatrix * baseImageCords;
			flagImageCoords /= flagImageCoords.at<double>(2);

			int insertingImageCoordX = flagImageCoords.at<double>(0);
			int insertingImageCoordY = flagImageCoords.at<double>(1);

			if (insertingImageCoordX > 0 && insertingImageCoordY > 0 && insertingImageCoordX < insertingImage.cols && insertingImageCoordY < insertingImage.rows) {
				resultImage.at<cv::Vec3b>(y, x) = insertingImage.at<cv::Vec3b>(insertingImageCoordY, insertingImageCoordX);
			}
		}
	}


	return resultImage;
}

int main()
{
	cv::Mat buildingImage = cv::imread("../images/vsb.jpg", CV_LOAD_IMAGE_COLOR);
	cv::Mat flagImage = cv::imread("../images/flag.png", CV_LOAD_IMAGE_COLOR);

	cv::Mat perspectiveTrasformImage = cv::Mat::zeros(buildingImage.rows, buildingImage.cols, (buildingImage.type()));

	perspectiveTrasformImage = doPerspectiveTransform(flagImage, buildingImage, cv::Point(69, 107), cv::Point(227, 76), cv::Point(228, 122), cv::Point(66, 134));

	cv::imshow("transformed image", perspectiveTrasformImage);

	cv::waitKey(0);
}



