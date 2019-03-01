// ObjectFeaturesDetection.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include "ObjectFeatureModel.h"
#include <iostream>


cv::Mat doBackgroundThresholding(cv::Mat image, int threshold);
cv::Mat doIndexingOfObjects(cv::Mat thresholdedImage);
void floodFill(cv::Mat *thresholdedImage, cv::Mat *alreadyVisitedMat, int nextIndex, int x, int y);
void analyzeObjectsInImg(cv::Mat image);
std::vector<ObjectFeatureModel> extractFeaturesOfObjects(cv::Mat indexedImage);
void computeBasicFeaturesOfObjects(cv::Mat indexedImage, std::map<int, ObjectFeatureModel> &mapOfObjectsFeatures);
void computeMassCenterMoments(cv::Mat indexedImage, std::map<int, ObjectFeatureModel> &mapOfObjectsFeatures);

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
	for (ObjectFeatureModel objectModel : extractFeaturesOfObjects(indexedImage)) {
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

std::vector<ObjectFeatureModel> extractFeaturesOfObjects(cv::Mat indexedImage) {
	std::vector<ObjectFeatureModel> objectFeatureModels;
	std::map<int, ObjectFeatureModel> mapOfObjectsFeatures;

	computeBasicFeaturesOfObjects(indexedImage, mapOfObjectsFeatures);
	computeMassCenterMoments(indexedImage, mapOfObjectsFeatures);

	for (std::map<int, ObjectFeatureModel>::iterator it = mapOfObjectsFeatures.begin(); it != mapOfObjectsFeatures.end(); it++)
	{
		it->second.setFeature(FeatureIndex::F1, pow(it->second.getFeature(FeatureIndex::perimeter), 2) / (100 * it->second.getFeature(FeatureIndex::m00)));

		double mC11 = it->second.getFeature(FeatureIndex::mMassCenter11);
		double mC02 = it->second.getFeature(FeatureIndex::mMassCenter02);
		double mC20 = it->second.getFeature(FeatureIndex::mMassCenter20);

		double mcMax, mcMin;
		mcMax = 0.5 * (mC20 + mC02) + 0.5 * sqrt(4 * pow(mC11, 2) + pow(mC20 - mC02, 2));
		mcMin = 0.5 * (mC20 + mC02) - 0.5 * sqrt(4 * pow(mC11, 2) + pow(mC20 - mC02, 2));

		it->second.setFeature(FeatureIndex::F2, mcMin / mcMax);
		objectFeatureModels.push_back(it->second);
	}

	return objectFeatureModels;
}

void computeBasicFeaturesOfObjects(cv::Mat indexedImage, std::map<int, ObjectFeatureModel> &mapOfObjectsFeatures) {
	for (int y = 0; y < indexedImage.rows; y++) {
		for (int x = 0; x < indexedImage.cols; x++) {
			if (indexedImage.at<uchar>(y, x) != 0) {
				if (mapOfObjectsFeatures.find(indexedImage.at<uchar>(y, x)) == mapOfObjectsFeatures.end()) {
					mapOfObjectsFeatures.insert(std::pair<int, ObjectFeatureModel>(indexedImage.at<uchar>(y, x), ObjectFeatureModel()));
				}
				ObjectFeatureModel* currObject = &mapOfObjectsFeatures[indexedImage.at<uchar>(y, x)];
				currObject->contributeToFeature(FeatureIndex::m00, 1);
				currObject->contributeToFeature(FeatureIndex::m10, x);
				currObject->contributeToFeature(FeatureIndex::m01, y);

				if (isPointContributingToPerimeter(indexedImage, x, y)) {
					currObject->contributeToFeature(FeatureIndex::perimeter, 1);
				}
			}
		}
	}

	for (std::map<int, ObjectFeatureModel>::iterator it = mapOfObjectsFeatures.begin(); it != mapOfObjectsFeatures.end(); it++)
	{
		it->second.setFeature(FeatureIndex::massCenterX, it->second.getFeature(FeatureIndex::m10) / it->second.getFeature(FeatureIndex::m00));
		it->second.setFeature(FeatureIndex::massCenterY, it->second.getFeature(FeatureIndex::m01) / it->second.getFeature(FeatureIndex::m00));
	}
}

void computeMassCenterMoments(cv::Mat indexedImage, std::map<int, ObjectFeatureModel> &mapOfObjectsFeatures) {
	for (int y = 0; y < indexedImage.rows; y++) {
		for (int x = 0; x < indexedImage.cols; x++) {
			if (indexedImage.at<uchar>(y, x) != 0) {
				ObjectFeatureModel* currObject = &mapOfObjectsFeatures[indexedImage.at<uchar>(y, x)];
				currObject->contributeToFeature(FeatureIndex::mMassCenter11, (x - currObject->getFeature(FeatureIndex::massCenterX)) *
																		    (y - currObject->getFeature(FeatureIndex::massCenterY)));
				currObject->contributeToFeature(FeatureIndex::mMassCenter20, pow(x - currObject->getFeature(FeatureIndex::massCenterX), 2));
				currObject->contributeToFeature(FeatureIndex::mMassCenter02, pow(y - currObject->getFeature(FeatureIndex::massCenterY), 2));
			}
		}
	}
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