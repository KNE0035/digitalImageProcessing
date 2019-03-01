#include "pch.h"
#include "ObjectFeatureModel.h"


ObjectFeatureModel::ObjectFeatureModel()
{
	for (int i = 1; i <= this->featuresQuantity; i++) {
		this->featuresValueMap[i] = 0;
	}
}


double ObjectFeatureModel::getDistanceOfFeatures(ObjectFeatureModel comparedObject) {
	return 0.0;
}

void ObjectFeatureModel::setFeature(FeatureIndex featureIndex, double value) {
	this->featuresValueMap[featureIndex] = value;
}

double ObjectFeatureModel::getFeature(FeatureIndex featureIndex) {
	return this->featuresValueMap[featureIndex];
}

void ObjectFeatureModel::contributeToFeature(FeatureIndex featureIndex, double contribution) {
	this->featuresValueMap[featureIndex] = this->featuresValueMap[featureIndex] + contribution;
}

std::string ObjectFeatureModel::toString() {
	std::stringstream ss;
	ss << "[features of object: ";
	for (int i = 1; i <= this->featuresQuantity; i++) {
		switch (i)
		{
			case FeatureIndex::m00:
				ss << "Area: ";
				break;
			case  FeatureIndex::m01:
				ss << "m01: ";
				break;
			case  FeatureIndex::m10:
				ss << "m10: ";
				break;
			case  FeatureIndex::massCenterX:
				ss << "mass center x: ";
				break;
			case  FeatureIndex::massCenterY:
				ss << "mass center y: ";
				break;
			case  FeatureIndex::perimeter:
				ss << "Perimeter: ";
				break;
			case  FeatureIndex::mMassCenter11:
				ss << "mMassCenter11: ";
				break;
			case  FeatureIndex::mMassCenter02:
				ss << "mMassCenter02: ";
				break;
			case  FeatureIndex::mMassCenter20:
				ss << "mMassCenter20: ";
				break;
			case  FeatureIndex::F1:
				ss << "F1: ";
				break;
			case  FeatureIndex::F2:
				ss << "F2: ";
				break;
		}

		ss << this->featuresValueMap[i] << ", ";
	}
	ss << "]";
	return ss.str();
}

ObjectFeatureModel::~ObjectFeatureModel()
{
}


