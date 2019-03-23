#ifndef KMEANS_H
#define KMEANS_H

#include "pch.h"
#include "FeatureVector.h"

std::vector<std::vector<FeatureVector>> kMeansAlg(std::vector<FeatureVector> points, int k, int nOIterations, std::vector<FeatureVector>& centroids);
std::map<FeatureVector, std::vector<FeatureVector>> distributePointsToGroups(std::vector<FeatureVector> points, std::vector<FeatureVector> centroids, int k);
std::vector<FeatureVector> recalculateNewAverageCentroids(std::map<FeatureVector, std::vector<FeatureVector>> pointGroups, std::vector<FeatureVector> centroids);
bool isGroupsIdentic(std::map<FeatureVector, std::vector<FeatureVector>> lastPointGrups, std::map<FeatureVector, std::vector<FeatureVector>> newPointGroups);
std::vector<std::vector<FeatureVector>> getListOfGroupsFromMapOfGroups(std::map<FeatureVector, std::vector<FeatureVector>> pointGroups);
double getSumOfSquaresOfGroups(std::map<FeatureVector, std::vector<FeatureVector>> groups);


#endif