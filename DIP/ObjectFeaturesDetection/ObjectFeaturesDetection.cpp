#include "pch.h"
#include "ObjectFeatureModel.h"
#include <iostream>
#include "FeatureVector.h"

constexpr int NO_CLASSES = 4;
constexpr int TRAINING_OBJECTS_PER_CLASS = 4;
FeatureIndex MAIN_FEATURES[] = { FeatureIndex::F1, FeatureIndex::F2, FeatureIndex::rectangularity };
constexpr int MAIN_FEATURES_COUNT = 3;

enum ObjectClassIndex { SQUARE = 0, STAR = 1, RECTANGLE = 2, CIRCLE = 3};

std::string getLabelFromIndex(ObjectClassIndex index) {
	std::string label;
	switch (index)
	{
		case ObjectClassIndex::RECTANGLE: 
			label = "Rectangle"; 
			break;
		case ObjectClassIndex::SQUARE:
			label = "Square";
			break;
		case ObjectClassIndex::STAR:
			label = "Star";
			break;
		case ObjectClassIndex::CIRCLE:
			label = "Circle";
			break;
	}
	return label;
}

cv::Mat doBackgroundThresholding(cv::Mat image, int threshold);
cv::Mat doIndexingOfObjects(cv::Mat thresholdedImage);
void floodFill(cv::Mat *thresholdedImage, cv::Mat *alreadyVisitedMat, int nextIndex, int x, int y);
std::vector<FeatureVector> extractEtalonsFromTrainingObjects(cv::Mat image);
std::vector<FeatureVector> extractFeaturesOfObjects(cv::Mat indexedImage);
std::vector<FeatureVector> getEtalonsFromFeatureVectors(std::vector<FeatureVector> objectFeatureModels);
void computeBasicFeaturesOfObjects(cv::Mat indexedImage, std::map<int, ObjectFeatureModel> &mapOfObjectsFeatures);
void computeMassCenterMoments(cv::Mat indexedImage, std::map<int, ObjectFeatureModel> &mapOfObjectsFeatures);
void printInfoAboutImage(cv::Mat image, std::vector<FeatureVector> etalons);
ObjectClassIndex classifyOBject(std::vector<FeatureVector> etalons, FeatureVector objectFeatureVector);
bool isPointContributingToPerimeter(cv::Mat indexedImage, int x, int y);
void cumputeMinWrittenRectOfObjects(cv::Mat indexedImage, std::map<int, ObjectFeatureModel> &mapOfObjectsFeatures);
cv::Point2i rotatePoint(cv::Point2i p1, cv::Point2i center, float angle);

int main()
{
	cv::Mat image = cv::imread("../images/train04.png", CV_LOAD_IMAGE_GRAYSCALE);
	std::vector<FeatureVector> etalons = extractEtalonsFromTrainingObjects(image);

	for (FeatureVector featureVector : etalons) {
		std::cout << featureVector.toString();
		std::cout << std::endl;
	}
	cv::Mat testImage = cv::imread("../images/test04.png", CV_LOAD_IMAGE_GRAYSCALE);
	printInfoAboutImage(testImage, etalons);
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

std::vector<FeatureVector> extractEtalonsFromTrainingObjects(cv::Mat image) {
	cv::Mat thresholdedImage =  doBackgroundThresholding(image, 127);
	cv::Mat indexedImage = doIndexingOfObjects(thresholdedImage);
	
	/*for (ObjectFeatureModel objectModel : extractFeaturesOfObjects(indexedImage)) {
		std::cout << objectModel.toString();
		std::cout << std::endl;
	}*/

	//cv::imshow("bacground threshold", thresholdedImage);
	//cv::imshow("indexed image", indexedImage);

	return getEtalonsFromFeatureVectors(extractFeaturesOfObjects(indexedImage));
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

std::vector<FeatureVector> extractFeaturesOfObjects(cv::Mat indexedImage) {
	std::vector<ObjectFeatureModel> objectFeatureModels;
	std::map<int, ObjectFeatureModel> mapOfObjectsFeatures;
	std::vector<FeatureVector> featureVectors;

	computeBasicFeaturesOfObjects(indexedImage, mapOfObjectsFeatures);
	computeMassCenterMoments(indexedImage, mapOfObjectsFeatures);
	cumputeMinWrittenRectOfObjects(indexedImage, mapOfObjectsFeatures);

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
		it->second.setFeature(FeatureIndex::rectangularity, it->second.getFeature(FeatureIndex::m00) / it->second.getMinAreaOfAllRotations());
		objectFeatureModels.push_back(it->second);
	}

	for (ObjectFeatureModel featureModel : objectFeatureModels) {
		FeatureVector featureVector = FeatureVector(featureModel, MAIN_FEATURES, MAIN_FEATURES_COUNT);
		featureVectors.push_back(featureVector);
	}

	return featureVectors;
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

std::vector<FeatureVector> getEtalonsFromFeatureVectors(std::vector<FeatureVector> featureVectors) {
	std::vector<FeatureVector> etalons;

	FeatureVector etalon;
	for (int i = 0; i < NO_CLASSES; i++) {
		etalon = featureVectors.at(i);
		for (int j = 1; j < TRAINING_OBJECTS_PER_CLASS; j++) {
			etalon = etalon + featureVectors.at(i * NO_CLASSES + j);
		}
		etalon = etalon / TRAINING_OBJECTS_PER_CLASS;
		etalons.push_back(etalon);
	}

	return etalons;
}

void printInfoAboutImage(cv::Mat image, std::vector<FeatureVector> etalons) {
	cv::Mat thresholdedImage = doBackgroundThresholding(image, 127);
	cv::Mat indexedImage = doIndexingOfObjects(thresholdedImage);
	std::vector<FeatureVector> featureVectors = extractFeaturesOfObjects(indexedImage);

	std::cout << "Objects in the picture: ";
	for (FeatureVector featureVector : featureVectors) {
		ObjectClassIndex classIndex = classifyOBject(etalons, featureVector);
		std::cout << getLabelFromIndex(classIndex) << ", ";
	}
}

ObjectClassIndex classifyOBject(std::vector<FeatureVector> etalons, FeatureVector objectFeatureVector) {
	double minDistance = etalons[0].euclidDistance(objectFeatureVector);
	ObjectClassIndex etalonIndex = (ObjectClassIndex)0;

	for (int i = 1; i < etalons.size(); i++) {
		double nextDistance = etalons[i].euclidDistance(objectFeatureVector);
		if (minDistance > nextDistance) {
			minDistance = nextDistance;
			etalonIndex = (ObjectClassIndex)i;
		}
	}

	return etalonIndex;
}

void cumputeMinWrittenRectOfObjects(cv::Mat indexedImage, std::map<int, ObjectFeatureModel> &mapOfObjectsFeatures) {
	cv::Point center = cv::Point(indexedImage.cols / 2, indexedImage.rows / 2);
	
	ObjectFeatureModel* currObject = NULL;
	for (int angle = 0; angle <= 90; angle += 5) {
		for (int y = 0; y < indexedImage.rows; y++) {
			for (int x = 0; x < indexedImage.cols; x++) {
				if (indexedImage.at<uchar>(y, x) != 0) {
					cv::Point2i rotatedPoint = rotatePoint(cv::Point2i(x, y), center, angle);
					currObject = &mapOfObjectsFeatures[indexedImage.at<uchar>(y, x)];
					currObject->updateMinMaxPoints(rotatedPoint);
				}
			}
		}

		for (std::map<int, ObjectFeatureModel>::iterator it = mapOfObjectsFeatures.begin(); it != mapOfObjectsFeatures.end(); it++)
		{
			it->second.updateWrittenRectMinArea();
		}
	}
}

cv::Point2i rotatePoint(cv::Point2i p1, cv::Point2i center, float angle)
{
	cv::Point2i rotatedPoint; 
	rotatedPoint.x = ((p1.x - center.x) * cos(angle)) - ((p1.y - center.y) * sin(angle)) + center.x;
	rotatedPoint.y = ((p1.x - center.x) * sin(angle)) + ((p1.y - center.y) * cos(angle)) + center.y;
	return rotatedPoint;
}