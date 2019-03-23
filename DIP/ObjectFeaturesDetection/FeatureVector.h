#ifndef FEATURE_VECTOR
#define FEATURE_VECTOR

#pragma once

#include "ObjectFeatureModel.h"

class FeatureVector
{
public:
	FeatureVector();
	FeatureVector(const int usedFeaturesCount);
	FeatureVector(ObjectFeatureModel objectFeatureModel, FeatureIndex* featureIndices, int usedFeaturesCount);

	cv::Point2i getPossitionInImage();
	void setPossitionInImage(cv::Point2i point);
	void enlargePointDimmension(int targetDimmension);

	bool operator==(const FeatureVector& p2) const;
	bool operator <(const FeatureVector& p2) const;
	FeatureVector operator+(const FeatureVector & p2);
	FeatureVector operator/(const double divisor);

	double euclidDistance(FeatureVector p2);
	std::string toString();
	
	~FeatureVector();

	double* toArray();
	void normalize();

private: 
	std::vector<double> vector;
	int usedFeaturesCount;
	FeatureIndex* usedFeaturesIndices;
	cv::Point2i possitionInImage;
};
#endif
