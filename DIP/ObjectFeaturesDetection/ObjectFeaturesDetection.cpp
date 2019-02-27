// ObjectFeaturesDetection.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include "ObjectFeatureModel.h"
#include <iostream>


cv::Mat doBackgroundThresholding(cv::Mat image, int threshold);
cv::Mat doIndexingOfObjects(cv::Mat thresholdedImage);
void floodFill(cv::Mat *thresholdedImage, cv::Mat *alreadyVisitedMat, int nextIndex, int x, int y);
void analyzeObjectsInImg(cv::Mat image);
std::vector<ObjectFeatureModel> getFeaturesOfObjects(cv::Mat indexedImage);
bool isPointContributingToPerimeter(cv::Mat indexedImage, int x, int y);


int main()
{
	cv::Mat image = cv::imread("../images/train.png", CV_LOAD_IMAGE_GRAYSCALE);
	cv::imshow("original", image);
	analyzeObjectsInImg(image);

	cv::waitKey(0);
}

cv::Mat doBackgroundThresholding(cv::Mat image, int threshold) {
	cv::Mat thresholdedImage = cv::Mat::zeros(image.rows, image.cols, CV_8UC1);
	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			if (image.at<uchar>(y, x) >= threshold) {
				thresholdedImage.at<uchar>(y, x) = 255;
			}
			else {
				thresholdedImage.at<uchar>(y, x) = 0;
			}
		}
	}

	return thresholdedImage;
}

void analyzeObjectsInImg(cv::Mat image) { 
	cv::Mat thresholdedImage =  doBackgroundThresholding(image, 127);
	cv::Mat indexedImage = doIndexingOfObjects(thresholdedImage);
	for (ObjectFeatureModel objectModel : getFeaturesOfObjects(indexedImage)) {
		std::cout << objectModel.toString();
		std::cout << std::endl;
	}

	cv::imshow("bacground threshold", thresholdedImage);
	cv::imshow("indexed image", indexedImage);
}

cv::Mat doIndexingOfObjects(cv::Mat thresholdedImage) {
	cv::Mat indexedImage = thresholdedImage.clone();
	cv::Mat alreadyVisitedMat = cv::Mat::zeros(thresholdedImage.rows, thresholdedImage.cols, CV_8UC1);
	int nextIndex = 1;

	for (int y = 0; y < thresholdedImage.rows; y++) {
		for (int x = 0; x < thresholdedImage.cols; x++) {
			if (thresholdedImage.at<uchar>(y, x) != 0 && alreadyVisitedMat.at<uchar>(y, x) != 1) {
				floodFill(&indexedImage, &alreadyVisitedMat, nextIndex, x, y);
				nextIndex++;
			}
		}
	}
	return indexedImage;
}

void floodFill(cv::Mat *indexedImage, cv::Mat *alreadyVisitedMat, int nextIndex, int x, int y) {
	if (x < 0 || x > indexedImage->cols - 1 || y < 0 || y > indexedImage->rows - 1) {
		return;
	}

	if (indexedImage->at<uchar>(y, x) == 0 || alreadyVisitedMat->at<uchar>(y, x) == 1) {
		return;
	}

	indexedImage->at<uchar>(y, x) = nextIndex;
	alreadyVisitedMat->at<uchar>(y, x) = 1;

	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x - 1, y - 1);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x + 1, y - 1);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x - 1, y + 1);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x + 1, y + 1);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x, y - 1);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x, y + 1);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x - 1, y);
	floodFill(indexedImage, alreadyVisitedMat, nextIndex, x + 1, y);
}

std::vector<ObjectFeatureModel> getFeaturesOfObjects(cv::Mat indexedImage) {
	std::vector<ObjectFeatureModel> objectFeatureModels;
	std::map<int, ObjectFeatureModel> mapOfObjects;

	for (int y = 0; y < indexedImage.rows; y++) {
		for (int x = 0; x < indexedImage.cols; x++) {
			if (indexedImage.at<uchar>(y, x) != 0) {
				if (mapOfObjects.find(indexedImage.at<uchar>(y, x)) == mapOfObjects.end()) {
					mapOfObjects.insert(std::pair<int, ObjectFeatureModel>(indexedImage.at<uchar>(y, x), ObjectFeatureModel()));
				}
				mapOfObjects[indexedImage.at<uchar>(y, x)].contributeToFeature(FeatureIndex::area, 1);
				mapOfObjects[indexedImage.at<uchar>(y, x)].contributeToM10(x);
				mapOfObjects[indexedImage.at<uchar>(y, x)].contributeToM01(y);

				if (isPointContributingToPerimeter(indexedImage, x, y)) {
					mapOfObjects[indexedImage.at<uchar>(y, x)].contributeToFeature(FeatureIndex::perimeter, 1);
				}
			}
		}
	}

	std::map<int, ObjectFeatureModel>::iterator it;
	for (it = mapOfObjects.begin(); it != mapOfObjects.end(); it++)
	{
		objectFeatureModels.push_back(it->second);
	}

	for (ObjectFeatureModel &model : objectFeatureModels) {
		model.updateFeature(FeatureIndex::massCenterX, model.getM10() / (int)model.getFeature(FeatureIndex::area));
		model.updateFeature(FeatureIndex::massCenterY, model.getM01() / (int)model.getFeature(FeatureIndex::area));
	}

	return objectFeatureModels;
}

bool isPointContributingToPerimeter(cv::Mat indexedImage, int x, int y) {
	int zeroPointSum = 0;

	for (int xShift = -1; xShift < 2; xShift++) {
		for (int yShift = -1; yShift < 2; yShift++) {
			if ((xShift == 0 && yShift == 0) || (xShift != 0 && yShift != 0)) {
				continue;
			}
			
			int neigborX = x + xShift;
			int neigborY = y + yShift;

			if (neigborX < 0 || neigborX > indexedImage.cols - 1 || neigborY < 0 || neigborY > indexedImage.rows - 1) {
				zeroPointSum++;
				continue;
			}

			if (indexedImage.at<uchar>(neigborY, neigborX) == 0) {
				zeroPointSum++;
			}
		}
	}

	return zeroPointSum != 0;
}