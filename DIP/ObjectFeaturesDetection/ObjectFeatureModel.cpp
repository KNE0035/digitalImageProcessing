#include "pch.h"
#include "ObjectFeatureModel.h"


ObjectFeatureModel::ObjectFeatureModel()
{
	this->m01 = 0;
	this->m10 = 0;

	for (int i = 1; i <= this->featuresQuantity; i++) {
		this->featuresValueMap[i] = 0;
	}
}


double ObjectFeatureModel::getDistanceOfFeatures(ObjectFeatureModel comparedObject) {
	return 0.0;
}

void ObjectFeatureModel::updateFeature(FeatureIndex featureIndex, double value) {
	this->featuresValueMap[featureIndex] = value;
}



void ObjectFeatureModel::contributeToFeature(FeatureIndex featureIndex, double contribution) {
	this->featuresValueMap[featureIndex] = this->featuresValueMap[featureIndex] + contribution;
}

void ObjectFeatureModel::contributeToM01(int contribution) {
	this->m01 += contribution;
}

void ObjectFeatureModel::contributeToM10(int contribution) {
	this->m10 += contribution;
}

double ObjectFeatureModel::getFeature(FeatureIndex featureIndex) {
	return this->featuresValueMap[featureIndex];
}

double ObjectFeatureModel::getM01() {
	return m01;
}

double ObjectFeatureModel::getM10() {
	return m10;
}

std::string ObjectFeatureModel::toString() {
	std::stringstream ss;
	ss << "[features of object: ";
	for (int i = 1; i <= this->featuresQuantity; i++) {
		switch (i)
		{
			case FeatureIndex::area:
				ss << "Area: ";
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
		}
		
		ss << this->featuresValueMap[i] << ", ";
	}
	ss << "]";
	return ss.str();
}

ObjectFeatureModel::~ObjectFeatureModel()
{
}


