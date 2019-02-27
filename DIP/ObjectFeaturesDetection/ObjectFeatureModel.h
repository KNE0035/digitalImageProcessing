#ifndef OBJECT_FEATURE_MODEL
#define OBJECT_FEATURE_MODEL
#pragma once

#include "pch.h"
enum FeatureIndex { area = 1, massCenterX = 2, massCenterY = 3, perimeter = 4 };

class ObjectFeatureModel
{
public:
	int featuresQuantity = 4;
	double getDistanceOfFeatures(ObjectFeatureModel comparedObject);
	void updateFeature(FeatureIndex featureIndex, double value);
	void contributeToFeature(FeatureIndex featureIndex, double contribution);
	void contributeToM01(int contribution);
	void contributeToM10(int contribution);

	double getFeature(FeatureIndex featureIndex);
	double getM01();
	double getM10();

	std::string toString();

	ObjectFeatureModel();
	~ObjectFeatureModel();
private: 
	std::map<int, double> featuresValueMap;
	int m01;
	int m10;
};

#endif // !OBJECT_FEATURE_MODEL



