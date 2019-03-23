#include "pch.h"
#include "FeatureVector.h"


FeatureVector::FeatureVector() {

}

FeatureVector::FeatureVector(int usedFeaturesCount) {
	for (int i = 0; i < usedFeaturesCount; i++) {
		this->vector.push_back(0.0);
	}
}

FeatureVector::FeatureVector(ObjectFeatureModel objectFeatureModel, FeatureIndex* featureIndices, int usedFeaturesCount)
{
	this->usedFeaturesCount = usedFeaturesCount;
	this->usedFeaturesIndices = featureIndices;
	for (int i = 0; i < this->usedFeaturesCount; i++) {
		this->vector.push_back(objectFeatureModel.getFeature(this->usedFeaturesIndices[i]));
	}

	this->possitionInImage = cv::Point2i(objectFeatureModel.getFeature(FeatureIndex::massCenterX), objectFeatureModel.getFeature(FeatureIndex::massCenterY));
}

cv::Point2i FeatureVector::getPossitionInImage() {
	return this->possitionInImage;
}

void FeatureVector::setPossitionInImage(cv::Point2i point) {
	this->possitionInImage = point;
}

double FeatureVector::euclidDistance(FeatureVector p2) {
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

bool FeatureVector::operator <(const FeatureVector& p2) const {
	FeatureVector p1tmp = *this;
	FeatureVector p2tmp = p2;
	int maxDimSize = std::max(this->vector.size(), p2.vector.size());

	FeatureVector zeroPoint(p1tmp.vector.size());

	double dist1 = p1tmp.euclidDistance(zeroPoint);
	double dist2 = p2tmp.euclidDistance(zeroPoint);

	return dist1 < dist2;
}

void FeatureVector::enlargePointDimmension(int targetDimmension) {
	while (this->vector.size() < targetDimmension) {
		this->vector.push_back(0);
	}
}

double* FeatureVector::toArray() {
	double *array = &this->vector[0];
	return array;
}

void FeatureVector::normalize() {
	double dist = 0;
	for (double x : this->vector) {
		dist += pow(x, 2);
	}

	dist = sqrt(dist);

	for (int i = 0; i < this->vector.size(); i++) {
		this->vector[i] /= dist;
	}
}

FeatureVector::~FeatureVector()
{
}


