#include "pch.h"
#include "FeatureVector.h"


FeatureVector::FeatureVector() {

}

FeatureVector::FeatureVector(FeatureIndex* featureIndices, int usedFeaturesCount) {
	this->usedFeaturesCount = usedFeaturesCount;
	this->usedFeaturesIndices = featureIndices;
}

FeatureVector::FeatureVector(ObjectFeatureModel objectFeatureModel, FeatureIndex* featureIndices, int usedFeaturesCount) : FeatureVector(featureIndices, usedFeaturesCount)
{
	for (int i = 0; i < this->usedFeaturesCount; i++) {
		this->vector.push_back(objectFeatureModel.getFeature(this->usedFeaturesIndices[i]));
	}
}

double FeatureVector::euclidDistance(FeatureVector p2) {
	int maxDimSize = std::max(this->vector.size(), p2.vector.size());

	double sumOfSquares = 0;

	for (int i = 0; i < this->vector.size(); i++) {
		sumOfSquares += pow(this->vector[i] - p2.vector[i], 2);
	}

	return sqrt(sumOfSquares);
}

bool FeatureVector::operator ==(const FeatureVector& p2) const {
	for (int i = 0; i < this->vector.size(); i++) {
		if (this->vector[i] != p2.vector[i]) {
			return false;
		}
	}

	return true;
}

FeatureVector FeatureVector::operator+(const FeatureVector & p2) {
	
	for (int i = 0; i < this->vector.size(); i++) {
		this->vector[i] += p2.vector[i];
	}

	return *this;
}

FeatureVector FeatureVector::operator/(const double divisor) {
	
	for (int i = 0; i < this->vector.size(); i++) {
		this->vector[i] /= divisor;
	}

	return *this;
}

std::string FeatureVector::toString() {
	std::ostringstream oss;
	oss << "[";

	std::copy(this->vector.begin(), this->vector.end() - 1,
		std::ostream_iterator<double>(oss, ", "));

	// Now add the last element with no delimiter
	oss << this->vector.back();

	oss << "]";

	return oss.str();
}


FeatureVector::~FeatureVector()
{
}
