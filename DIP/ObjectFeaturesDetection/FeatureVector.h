#ifndef FEATURE_VECTOR
#define FEATURE_VECTOR

#pragma once

#include "ObjectFeatureModel.h"

class FeatureVector
{
public:
	FeatureVector();
	FeatureVector(FeatureIndex* featureIndices, const int usedFeaturesCount);
	FeatureVector(ObjectFeatureModel objectFeatureModel, FeatureIndex* featureIndices, int usedFeaturesCount);

	bool operator==(const FeatureVector& p2) const;
	FeatureVector operator+(const FeatureVector & p2);
	FeatureVector operator/(const double divisor);

	double euclidDistance(FeatureVector p2);
	std::string toString();

	~FeatureVector();


private: 
	std::vector<double> vector;
	int usedFeaturesCount;
	FeatureIndex* usedFeaturesIndices;
};
#endif
