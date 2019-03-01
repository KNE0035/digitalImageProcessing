#ifndef OBJECT_FEATURE_MODEL
#define OBJECT_FEATURE_MODEL
#pragma once

#include "pch.h"
enum FeatureIndex { 
	m00 = 1,
	m01 = 2,
	m10 = 3,
	massCenterX = 4, 
	massCenterY = 5, 
	perimeter = 6,
	mMassCenter11 = 7,
	mMassCenter02 = 8,
	mMassCenter20 = 9,
	F1 = 10, 
	F2 = 11 
};

class ObjectFeatureModel
{
public:
	static const int featuresQuantity = 11;
	double getDistanceOfFeatures(ObjectFeatureModel comparedObject);

	void setFeature(FeatureIndex featureIndex, double value);
	void contributeToFeature(FeatureIndex featureIndex, double contribution);
	double getFeature(FeatureIndex featureIndex);

	std::string toString();

	ObjectFeatureModel();
	~ObjectFeatureModel();
private:
	std::map<int, double> featuresValueMap;
	std::vector<FeatureIndex> classificationFeatures { FeatureIndex::F1, FeatureIndex::F2 };
};

#endif // !OBJECT_FEATURE_MODEL



