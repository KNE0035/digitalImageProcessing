#include "pch.h"
#include "kMeansAlg.h"
#include "FeatureVector.h"


std::vector<std::vector<FeatureVector>> kMeansAlg(std::vector<FeatureVector> points, int k, int nOIterations, std::vector<FeatureVector>& centroids) {
	srand(time(0));
	std::vector<std::vector<FeatureVector>> result;

	double minSumOfSquares = INT_MAX;

	for (int i = 0; i < nOIterations; i++) {
		std::map<FeatureVector, std::vector<FeatureVector>> lastPointGrups;
		std::map<FeatureVector, std::vector<FeatureVector>> newPointGroups;
		std::vector<FeatureVector> newPossibleCentroids;
		std::vector<std::vector<FeatureVector>> newPossibleGroups;
		std::random_shuffle(points.begin(), points.end());
	
		#pragma omp parallel for
		for (int i = 0; i < k; i++) {
			newPossibleCentroids.push_back(points[i]);
		}

		do {
			lastPointGrups = distributePointsToGroups(points, newPossibleCentroids, k);
			newPossibleCentroids = recalculateNewAverageCentroids(lastPointGrups, newPossibleCentroids);
			newPointGroups = distributePointsToGroups(points, newPossibleCentroids, k);
		} while (!isGroupsIdentic(lastPointGrups, newPointGroups));

		double sumOfSquares = getSumOfSquaresOfGroups(newPointGroups);
		if (sumOfSquares < minSumOfSquares) {
			minSumOfSquares = sumOfSquares;
			std::cout << minSumOfSquares << std::endl;
			result = getListOfGroupsFromMapOfGroups(newPointGroups);
			centroids = newPossibleCentroids;
		}
	}
	return result;
}

double getSumOfSquaresOfGroups(std::map<FeatureVector, std::vector<FeatureVector>> groups) {
	double sumOfSquares = 0;
	
	std::vector<std::vector<FeatureVector>> result;

	for (std::map<FeatureVector, std::vector<FeatureVector>>::iterator it = groups.begin(); it != groups.end(); it++)
	{
		FeatureVector centroidOfGroup = it->first;
		std::vector<FeatureVector> pointsInGroup = it->second;
		for (FeatureVector pointInGroup : pointsInGroup) {
			sumOfSquares += pointInGroup.euclidDistance(centroidOfGroup);
		}
	}
	return sumOfSquares;
}

std::map<FeatureVector, std::vector<FeatureVector>> distributePointsToGroups(std::vector<FeatureVector> points, std::vector<FeatureVector> centroids, int k) {
	std::map<FeatureVector, std::vector<FeatureVector>> pointGrups;
	double* distances = new double[points.size() * k];
	#pragma omp parallel for
	for (int i = 0; i < k; i++) {
	#pragma omp parallel for
		for (int j = 0; j < points.size(); j++) {
			distances[points.size() * i + j] = centroids[i].euclidDistance(points[j]);
		}
	}

	#pragma omp parallel for
	for (int col = 0; col < points.size(); col++) {
		double minDist = distances[col];
		int minPointIndex = col;
		int centriodIndex = 0;

		for (int row = 1; row < k; row++) {
			if (minDist > distances[points.size() * row + col]) {
				minDist = distances[points.size() * row + col];
				centriodIndex = row;
			}
		}

		#pragma omp critical (insert_to_map) 
		{
			if (pointGrups.find(centroids[centriodIndex]) == pointGrups.end()) {
				pointGrups.insert(std::pair<FeatureVector, std::vector<FeatureVector>>(centroids[centriodIndex], {}));
			}

			if (std::find(points.begin(), points.end(), points[minPointIndex]) != points.end()) {
				pointGrups[centroids[centriodIndex]].push_back(points[minPointIndex]);
			}
		}
	}

	delete[] distances;
	return pointGrups;
}

std::vector<FeatureVector> recalculateNewAverageCentroids(std::map<FeatureVector, std::vector<FeatureVector>> pointGroups, std::vector<FeatureVector> centroids) {

	#pragma omp parallel for
	for (int i = 0; i < pointGroups.size(); i++) {
		auto it = pointGroups.begin();
		advance(it, i);
		std::vector<FeatureVector> pointsInGroup = it->second;
		FeatureVector newCentroid;
		
		newCentroid = pointsInGroup[0];
		#pragma omp parallel for shared(newCentroid)
		for (int i = 1; i < pointsInGroup.size(); i++) {
			newCentroid = newCentroid + pointsInGroup[i];
		}

		newCentroid = newCentroid / pointsInGroup.size();

		centroids[i] = newCentroid;
	}

	return centroids;
}

bool isGroupsIdentic(std::map<FeatureVector, std::vector<FeatureVector>> lastPointGrups, std::map<FeatureVector, std::vector<FeatureVector>> newPointGroups) {
	bool identic = true;
	if (lastPointGrups.size() != newPointGroups.size()) {
		return false;
	}

	#pragma omp parallel for
	for (int i = 0; i < lastPointGrups.size(); i++) {

		auto lastPointsGroupsIt = lastPointGrups.begin();
		auto newPointsGroupsIt = newPointGroups.begin();

		advance(lastPointsGroupsIt, i);
		advance(newPointsGroupsIt, i);

		std::vector<FeatureVector> lastGroup = lastPointsGroupsIt->second;
		std::vector<FeatureVector> newGroup = newPointsGroupsIt->second;

		if (lastGroup.size() != newGroup.size()) {
			identic = false;
			break;
		}

	#pragma omp parallel for
		for (int i = 0; i < lastGroup.size(); i++) {
			if (std::find(lastGroup.begin(), lastGroup.end(), newGroup[i]) == lastGroup.end()) {
				identic = false;
				break;
			}
		}
	}
	return identic;
}

std::vector<std::vector<FeatureVector>> getListOfGroupsFromMapOfGroups(std::map<FeatureVector, std::vector<FeatureVector>> pointGroups) {
	std::map<FeatureVector, std::vector<FeatureVector>>::iterator it;
	std::vector<std::vector<FeatureVector>> result;

	for (it = pointGroups.begin(); it != pointGroups.end(); it++)
	{
		std::vector<FeatureVector> pointsInGroup = it->second;
		result.push_back(pointsInGroup);
	}

	return result;
}