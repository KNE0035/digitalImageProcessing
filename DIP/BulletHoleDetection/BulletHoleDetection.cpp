// BulletHoleDetection.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>



using namespace cv;
using namespace std;

int lowThreshold = 10;
int ratio = 2;


struct TargetFeatures {
	int index;
	int area;
	float massCenterX;
	float massCenterY;
	Point2i leftTopCorner;
	Point2i rightTopCorner;
	Point2i leftBottomCorner;
	Point2i rightBottomCorner;
};

Vec3f  biggestTargetCircledetection(cv::Mat targetImage);
std::vector<Point> targetCornersDetection(cv::Mat targetImage);
cv::Mat segmetnOutTarget(cv::Mat targetImage);
cv::Mat doIndexingOfObjects(cv::Mat thresholdedImage, cv::Mat edgeImage);
void floodFill(cv::Mat *indexedImage, cv::Mat *thresholdedImage, cv::Mat *alreadyVisitedMat, cv::Mat* edgeImage, int nextIndex, int x, int y);
std::vector<TargetFeatures> computeBasicFeaturesOfObjects(cv::Mat indexedImage, int areaThreshold);
cv::Mat applyGamaCorection(cv::Mat* img, float gamaCoef);
void CallBackFunc(int event, int x, int y, int flags, void* userdata);

TargetFeatures getTargetObject(std::vector<TargetFeatures> potentialTargetObjects);
void findNearestTargetCorners(TargetFeatures& targetObject, cv::Mat indexedImage);
Point2i findNearestTargetCorner(int targetIndex, Point2i basePoint, cv::Mat indexedImage, bool goRightX, bool goBottomY, int neigberhoodLength);
cv::Mat transformToRect(int width, int height, cv::Mat source, TargetFeatures targetFeatures);

int main()
{

	cv::Mat sourceImg = cv::imread("../terce/IMG_20190523_173111.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat targetImage;
	//IMG_20190523_172901 bad image
	//IMG_20190523_171600 good image
	cv::resize(sourceImg, sourceImg, cv::Size(), 0.25, 0.25);
	cv::Mat thresholded;
	cv::threshold(sourceImg, thresholded, 100, 255, 0);
	blur(sourceImg, targetImage, Size(3, 3));
	imshow("input", targetImage);
	imshow("canvasOutput", thresholded);

	Canny(targetImage, targetImage, lowThreshold, lowThreshold*ratio);

	Mat dilatationKernel = cv::getStructuringElement(cv::MORPH_RECT, Size(3, 3));
	cv::Mat dilated;
	dilate(targetImage, dilated, dilatationKernel);
	imshow("dilated", dilated);

	cv::Mat s = doIndexingOfObjects(thresholded, dilated);
	std::vector<TargetFeatures> objects = computeBasicFeaturesOfObjects(s, 10000);
	namedWindow("indexing", 1);
	setMouseCallback("indexing", CallBackFunc, &s);
	
	TargetFeatures targetObj = getTargetObject(objects);

	findNearestTargetCorners(targetObj, s);
	circle(s, targetObj.leftTopCorner, 10, Scalar(50000), 3, 8, 0);
	circle(s, targetObj.leftBottomCorner, 10, Scalar(50000), 3, 8, 0);
	circle(s, targetObj.rightTopCorner, 10, Scalar(50000), 3, 8, 0);
	circle(s, targetObj.rightBottomCorner, 10, Scalar(50000), 3, 8, 0);
	
	for (int y = 0; y < s.rows; y++)
	{
		for (int x = 0; x < s.cols; x++)
		{
			if (s.at<int>(y, x) == targetObj.index) {
				s.at<int>(y, x) = 700000;
			}
		}
	}

	cv::Mat result;
	result = transformToRect(1000, 1000, sourceImg, targetObj);

	imshow("indexing", s);

	imshow("test img", dilated);
	//cv::Mat myThresh = segmetnOutTarget(targetImage);
	//imshow("mythresh", myThresh);
	//Canny(targetImage, targetImage, lowThreshold, lowThreshold*ratio, kernel_size);

	//targetCornersDetection(thresholded);


	imshow("canny", targetImage);
	//circle detection
	/*Point center(cvRound(biggestCircle[0]), cvRound(biggestCircle[1]));
	int radius = cvRound(biggestCircle[2]);
	circle(biggestCicrcleVisualizeImage, center, 3, Scalar(120), 3, 8, 0);
	circle(biggestCicrcleVisualizeImage, center, radius, Scalar(120), 3, 8, 0);*/

	//namedWindow("Hough Circle Transform", CV_WINDOW_AUTOSIZE);
	//imshow("src", targetImage);
	//imshow("Hough Circle Transform Demo", biggestCicrcleVisualizeImage);
	waitKey(0);
	return 0;
}

cv::Mat transformToRect(int width, int height, cv::Mat source, TargetFeatures targetFeatures) {
	cv::Mat rectTarget = cv::Mat::zeros(height, width, CV_8UC1);

	std::vector<cv::Point2f> quad_pts;
	quad_pts.push_back(cv::Point2f(0, 0));
	quad_pts.push_back(cv::Point2f(rectTarget.cols, 0));
	quad_pts.push_back(cv::Point2f(rectTarget.cols, rectTarget.rows));
	quad_pts.push_back(cv::Point2f(0, rectTarget.rows));

	std::vector<cv::Point2f> corners;
	corners.push_back(targetFeatures.leftTopCorner);
	corners.push_back(targetFeatures.rightTopCorner);
	corners.push_back(targetFeatures.rightBottomCorner);
	corners.push_back(targetFeatures.leftBottomCorner);

	cv::Mat transmtx = cv::getPerspectiveTransform(corners, quad_pts);
	cv::warpPerspective(source, rectTarget, transmtx, rectTarget.size());

	cv::imshow("quadrilateral", rectTarget);
	return rectTarget;
}

Vec3f  biggestTargetCircledetection(cv::Mat targetImage) {
	cv::Mat result = targetImage.clone();
	GaussianBlur(result, result, Size(15, 15), 2, 2);

	std::vector<Vec3f> circles;
	HoughCircles(result, circles, CV_HOUGH_GRADIENT, 1, INT32_MAX, 8, 60, 265, 0);
	return circles[0];
}

TargetFeatures getTargetObject(std::vector<TargetFeatures> potentialTargetObjects) {
	TargetFeatures targetObject;
	for (TargetFeatures object : potentialTargetObjects) {
		if (targetObject.area < object.area) {
			targetObject = object;
		}
	}
	
	return targetObject;
}

void findNearestTargetCorners(TargetFeatures& targetObject, cv::Mat indexedImage) {
	int neigberhoodLength = 150;
	bool cornerFound = false;

	targetObject.leftTopCorner = findNearestTargetCorner(targetObject.index, targetObject.leftTopCorner, indexedImage, true, true, neigberhoodLength);
	targetObject.rightTopCorner = findNearestTargetCorner(targetObject.index, targetObject.rightTopCorner, indexedImage, false, true, neigberhoodLength);
	targetObject.leftBottomCorner = findNearestTargetCorner(targetObject.index, targetObject.leftBottomCorner, indexedImage, true, false, neigberhoodLength);
	targetObject.rightBottomCorner = findNearestTargetCorner(targetObject.index, targetObject.rightBottomCorner, indexedImage, false, false, neigberhoodLength);
}

Point2i findNearestTargetCorner(int targetIndex, Point2i basePoint, cv::Mat indexedImage, bool goRightX, bool goBottomY, int neigberhoodLength) {
	Point2i nearestCorner = basePoint;
	bool cornerFound = false;
	int minDistance = neigberhoodLength;
	for (int y = 0; y < neigberhoodLength;  y++)
	{
		for (int x = 0; x < neigberhoodLength; x++)
		{
			Point2i nextPoint = Point2i(basePoint.x + (goRightX ? x : -x), basePoint.y + (goBottomY ? y : -y));
			if (indexedImage.at<int>(nextPoint) == targetIndex) {
				int nextPointDistance = sqrt(x * y);
				if (minDistance > nextPointDistance) {
					nearestCorner = nextPoint;
					minDistance = nextPointDistance;
				}
			}
		}
	}
	return nearestCorner;
}



std::vector<Point> targetCornersDetection(cv::Mat targetImage) {
	float thresh = 200;
	const char* corners_window = "Corners detected";
	std::vector<Point> corners;

	int blockSize = 4;
	int apertureSize = 3;
	double k = 0.05;
	Mat dst = Mat::zeros(targetImage.size(), CV_32FC1);
	cornerHarris(targetImage, dst, blockSize, apertureSize, k);
	
	Mat dst_norm, dst_norm_scaled;
	normalize(dst, dst_norm, 0, 255, NORM_MINMAX, CV_32FC1, Mat());
	convertScaleAbs(dst_norm, dst_norm_scaled);
	for (int i = 0; i < dst_norm.rows; i++)
	{
		for (int j = 0; j < dst_norm.cols; j++)
		{
			if (dst_norm.at<float>(i, j) > thresh)
			{
				circle(targetImage, Point(j, i), 5, Scalar(120), 2, 8, 0);
				corners.push_back(Point(j, i));
			}
		}
	}
	namedWindow(corners_window);
	imshow(corners_window, targetImage);
	std::cout << corners.size();
	return corners;
}

cv::Mat segmetnOutTarget(cv::Mat targetImage) {
	cv::Mat result = targetImage.clone();
	int histogram[256];
	int pixelValueFrequencyThresh = 14000;
	int offset = 10;

	for (int i = 0; i < 256; i++) {
		histogram[i] = 0;
	}

	for (int y = 0; y < targetImage.rows; y++)
	{
		for (int x = 0; x < targetImage.cols; x++)
		{
			histogram[targetImage.at<uchar>(y, x)]++;
		}
	}

	for (int y = 0; y < targetImage.rows; y++)
	{
		for (int x = 0; x < targetImage.cols; x++)
		{
			int good_histogram = histogram[targetImage.at<uchar>(y, x)];
			if (good_histogram >=  pixelValueFrequencyThresh) {
				result.at<uchar>(y, x) = 255;
			}
			else {
				result.at<uchar>(y, x) = 0;
			}
		}
	}

	for (int i = 0; i < 256; i++) {
		std::cout << histogram[i] << std::endl;
	}
	return result;
}

cv::Mat doIndexingOfObjects(cv::Mat thresholdedImage, cv::Mat edgeImage) {
	cv::Mat indexedImage = cv::Mat::zeros(thresholdedImage.size(), CV_32SC1);
	cv::Mat alreadyVisitedMat = cv::Mat::zeros(thresholdedImage.rows, thresholdedImage.cols, CV_8UC1);
	int nextIndex = 1;

	for (int y = 0; y < thresholdedImage.rows; y++) {
		for (int x = 0; x < thresholdedImage.cols; x++) {
			if (thresholdedImage.at<uchar>(y, x) != 0 && alreadyVisitedMat.at<uchar>(y, x) != 1) {
				floodFill(&indexedImage, &thresholdedImage, &alreadyVisitedMat, &edgeImage, nextIndex, x, y);
				nextIndex++;
			}
		}
	}
	printf("%d\n", nextIndex);
	return indexedImage;
}

void floodFill(cv::Mat *indexedImage, cv::Mat *thresholdedImage, cv::Mat *alreadyVisitedMat, cv::Mat* edgeImage, int nextIndex, int x, int y) {
	if (x < 0 || x > thresholdedImage->cols - 1 || y < 0 || y > thresholdedImage->rows - 1) {
		return;
	}

	if (thresholdedImage->at<uchar>(y, x) == 0 || alreadyVisitedMat->at<uchar>(y, x) == 1 || edgeImage->at<uchar>(y, x) != 0) {
		return;
	}

	indexedImage->at<int>(y, x) = nextIndex;
	alreadyVisitedMat->at<uchar>(y, x) = 1;

	floodFill(indexedImage, thresholdedImage, alreadyVisitedMat, edgeImage, nextIndex, x - 1, y - 1);
	floodFill(indexedImage, thresholdedImage, alreadyVisitedMat, edgeImage, nextIndex, x + 1, y - 1);
	floodFill(indexedImage, thresholdedImage, alreadyVisitedMat, edgeImage, nextIndex, x - 1, y + 1);
	floodFill(indexedImage, thresholdedImage, alreadyVisitedMat, edgeImage, nextIndex, x + 1, y + 1);
	floodFill(indexedImage, thresholdedImage, alreadyVisitedMat, edgeImage, nextIndex, x, y - 1);
	floodFill(indexedImage, thresholdedImage, alreadyVisitedMat, edgeImage, nextIndex, x, y + 1);
	floodFill(indexedImage, thresholdedImage, alreadyVisitedMat, edgeImage, nextIndex, x - 1, y);
	floodFill(indexedImage, thresholdedImage, alreadyVisitedMat, edgeImage, nextIndex, x + 1, y);
}


cv::Mat applyGamaCorection(cv::Mat* img, float gamaCoef) {
	cv::Mat result = img->clone();
	cv::Mat* FC1_32_Img = img;

	if (img->type() != CV_32FC1) {
		FC1_32_Img->convertTo(*FC1_32_Img, CV_32FC1, 1.0 / 255.0);
	}

	for (int y = 0; y < FC1_32_Img->rows; y++) {
		for (int x = 0; x < FC1_32_Img->cols; x++) {
			FC1_32_Img->at<float>(y, x) = pow(FC1_32_Img->at<float>(y, x), gamaCoef);
		}
	}
	
	for (int y = 0; y < FC1_32_Img->rows; y++) {
		for (int x = 0; x < FC1_32_Img->cols; x++) {
			result.at<uchar>(y, x) = FC1_32_Img->at<float>(y, x) * 255;
		}
	}

	return result;
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
	Mat* rgb = (Mat*)userdata;
	if (event == EVENT_FLAG_LBUTTON)
	{
		printf("%d %d: value: %d\n",
			x, y,
			(int)(*rgb).at<int>(y, x));
	}
}

std::vector<TargetFeatures> computeBasicFeaturesOfObjects(cv::Mat indexedImage, int areaThreshold) {
	std::map<int, TargetFeatures> mapOfObjectsFeatures;
	std::vector<TargetFeatures> result;

	for (int y = 0; y < indexedImage.rows; y++) {
		for (int x = 0; x < indexedImage.cols; x++) {
			if (indexedImage.at<int>(y, x) != 0) {
				if (mapOfObjectsFeatures.find(indexedImage.at<int>(y, x)) == mapOfObjectsFeatures.end()) {
					TargetFeatures newObj = TargetFeatures();
					newObj.leftTopCorner.x = indexedImage.cols - 1;
					newObj.leftTopCorner.y = indexedImage.rows - 1;

					newObj.rightTopCorner.x = 0;
					newObj.rightTopCorner.y = indexedImage.rows - 1;

					newObj.leftBottomCorner.x = indexedImage.cols - 1;
					newObj.leftBottomCorner.y = 0;

					newObj.rightBottomCorner.x = 0;
					newObj.rightBottomCorner.y = 0;

					mapOfObjectsFeatures.insert(std::pair<int, TargetFeatures>(indexedImage.at<int>(y, x), newObj));
				}
				int currIndex = indexedImage.at<int>(y, x);
				TargetFeatures* currObject = &mapOfObjectsFeatures[currIndex];
				currObject->area++;
				currObject->massCenterX = currObject->massCenterX + x;
				currObject->massCenterY = currObject->massCenterY + y;

				currObject->leftTopCorner.x = std::min(currObject->leftTopCorner.x, x);
				currObject->leftTopCorner.y = std::min(currObject->leftTopCorner.y, y);
				
				currObject->rightTopCorner.x = std::max(currObject->rightTopCorner.x, x);
				currObject->rightTopCorner.y = std::min(currObject->rightTopCorner.y, y);
				
				currObject->leftBottomCorner.x = std::min(currObject->leftBottomCorner.x, x);
				currObject->leftBottomCorner.y = std::max(currObject->leftBottomCorner.y, y);

				currObject->rightBottomCorner.x = std::max(currObject->rightBottomCorner.x, x);
				currObject->rightBottomCorner.y = std::max(currObject->rightBottomCorner.y, y);
			}
		}
	}

	for (std::map<int, TargetFeatures>::iterator it = mapOfObjectsFeatures.begin(); it != mapOfObjectsFeatures.end(); it++)
	{
		if (it->second.area >= areaThreshold) {
			it->second.massCenterX = it->second.massCenterX / it->second.area;
			it->second.massCenterY = it->second.massCenterY / it->second.area;
			it->second.index = it->first;
			result.push_back(it->second);
		}
	}

	return result;
}