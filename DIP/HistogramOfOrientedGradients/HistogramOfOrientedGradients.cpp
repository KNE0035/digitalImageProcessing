// HistogramOfOrientedGradients.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <stdio.h>

constexpr int BLOCK_DIM_X = 2;
constexpr int BLOCK_DIM_Y = 2;

constexpr int CELL_DIM_X = 7;
constexpr int CELL_DIM_Y = 7;

constexpr int BINS = 6;

using namespace std;

struct GradientInfo {
	cv::Mat gradientOrientationMat;
	cv::Mat gradientSizeMat;
};

float* getImgHOGVector(cv::Mat img, int* histogramLength);
GradientInfo getImgGradientInfo(cv::Mat img);
float* getHistogramsPerBlock(GradientInfo gradientInfo, int blockTopCoordX, int blockTopCoordY);
float* getHistogramInCell(GradientInfo gradientInfo, int blockTopCoordX, int blockTopCoordY, int cellCoordX, int cellCoordY);
void neural_network(cv::Mat features, cv::Mat labels);
std::string numberToNDigitsString(int n, int number);
vector<string> listFilesInDirectory(string folder);
vector<int> labelsToClass(cv::Mat labels);

int main()
{
	int datasetLength = 9774;
	int nOClasses = 10;
	std::string basePath = "../testing/";
	cv::Mat img = cv::imread("../training/0/0000.png", CV_LOAD_IMAGE_GRAYSCALE);
	int nfeatures = 96;
	cv::Mat features(datasetLength, nfeatures, CV_32F);
	cv::Mat labels(datasetLength, nOClasses, CV_32F);

	// load data
	int k = 0;
	int total = 0;
	for (int classIndex = 0; classIndex < nOClasses; classIndex++) {
		std::string concreteNumberClassPath = basePath + std::to_string(classIndex) + "/";
		
		vector<string> listOfFiles = listFilesInDirectory(concreteNumberClassPath);
		total += listOfFiles.size();
		for(string file : listOfFiles) {
			float label = classIndex;
			std::string concreteNumberPath = concreteNumberClassPath + file;
			cv::Mat img = cv::imread(concreteNumberPath, CV_LOAD_IMAGE_GRAYSCALE);
			
			int histogramLength;
			float* imgHOGVector = getImgHOGVector(img, &histogramLength);

			for (int featureIndex = 0; featureIndex < histogramLength; featureIndex++) {
				features.at<float>(k, featureIndex) = imgHOGVector[featureIndex];
			}

			for (int labelIndex = 0; labelIndex < nOClasses; labelIndex++) {
				if (labelIndex == classIndex) {
					labels.at<float>(k, labelIndex) = 1.0f;
				}
				else {
					labels.at<float>(k, labelIndex) = 0.0f;
				}
			}
			k++;
		}
	}
	neural_network(features, labels);


	return 0;
}


float* getImgHOGVector(cv::Mat img, int* histogramLength) {
	GradientInfo gradientInfo = getImgGradientInfo(img);

	//int nOBlocksX = img.cols - BLOCK_DIM_X * CELL_DIM_X + 1;
	//int nOBlocksY = img.rows - BLOCK_DIM_Y * CELL_DIM_Y + 1;

	int nOBlocksX = img.cols / (BLOCK_DIM_X * CELL_DIM_X);
	int nOBlocksY = img.rows / (BLOCK_DIM_Y * CELL_DIM_Y);


	*histogramLength = nOBlocksX * nOBlocksY * BLOCK_DIM_X * BLOCK_DIM_Y * BINS;
	float* histograms = new float[*histogramLength];
	float* nextHistogramsBlock = histograms;

	for (int i = 0; i < nOBlocksY; i++) {
		for (int j = 0; j < nOBlocksX; j++) {
			memcpy(nextHistogramsBlock, getHistogramsPerBlock(gradientInfo, j, i), BLOCK_DIM_X * BLOCK_DIM_Y * BINS * sizeof(float));
			nextHistogramsBlock += (BLOCK_DIM_X * BLOCK_DIM_Y * BINS);
		}
	}
	return histograms;
}

float* getHistogramsPerBlock(GradientInfo gradientInfo, int blockTopCoordX, int blockTopCoordY) {
	const int histogramsPerBlockLength = BLOCK_DIM_X * BLOCK_DIM_Y * BINS;
	float histogramsPerBlock[histogramsPerBlockLength];
	float* histogramNextPointer = histogramsPerBlock;
	
	/*if (blockTopCoordX == 14 && blockTopCoordY == 0) {
		printf("as");
	}*/

	for (int i = 0; i < BLOCK_DIM_Y; i++) {
		for (int j = 0; j < BLOCK_DIM_X; j++) {
			memcpy(histogramNextPointer, getHistogramInCell(gradientInfo, blockTopCoordX, blockTopCoordY, j, i), BINS * sizeof(float));
			histogramNextPointer += BINS;
		}
	}
	
	//normalize over block
	float l2Norm = 0;

	for (int i = 0; i < histogramsPerBlockLength; i++) {
		l2Norm += histogramsPerBlock[i] * histogramsPerBlock[i];
	}
	l2Norm = sqrtf(l2Norm);

	if (l2Norm != 0) {
		for (int i = 0; i < histogramsPerBlockLength; i++) {
			histogramsPerBlock[i] /= l2Norm;
		}
	}
	
	return histogramsPerBlock;
}

float* getHistogramInCell(GradientInfo gradientInfo, int blockTopCoordX, int blockTopCoordY, int cellCoordX, int cellCoordY) {
	float histogram[BINS];
	std::fill(histogram, histogram + BINS, 0);
	const int binWidth = 180 / BINS;

	for (int i = 0; i < CELL_DIM_Y; i++) {
		for (int j = 0; j < CELL_DIM_X; j++) {
			int yOffset = CELL_DIM_Y * BLOCK_DIM_Y * blockTopCoordY + CELL_DIM_Y * cellCoordY + i;
			int xOffset = CELL_DIM_X * BLOCK_DIM_X * blockTopCoordX + CELL_DIM_X * cellCoordX + j;

			int binIndex = (gradientInfo.gradientOrientationMat.at<float>(yOffset, xOffset) - 1) / binWidth;
			histogram[binIndex] += gradientInfo.gradientSizeMat.at<float>(yOffset, xOffset);
		}
	}
	return histogram;
}

GradientInfo getImgGradientInfo(cv::Mat img) {
	GradientInfo gradientInfo;
	gradientInfo.gradientOrientationMat = cv::Mat::zeros(img.rows, img.cols, CV_32FC1);
	gradientInfo.gradientSizeMat = cv::Mat::zeros(img.rows, img.cols, CV_32FC1);

	for (int y = 1; y < img.rows - 1; y++) {
		for (int x = 1; x < img.cols - 1; x++) {
			float fx = (double)img.at<uchar>(y, x + 1) - img.at<uchar>(y, x);
			float fy = (double)img.at<uchar>(y + 1, x) - img.at<uchar>(y, x);

			gradientInfo.gradientOrientationMat.at<float>(y, x) = (atan2f(fy, fx) < 0 ? atan2f(fy, fx) + M_PI : atan2f(fy, fx)) * (180 / M_PI);
			gradientInfo.gradientSizeMat.at<float>(y, x) = sqrtf(fx * fx + fy * fy);
		}
	}
	return gradientInfo;
}



void neural_network(cv::Mat features, cv::Mat labels)
{
	vector<int> trainingLabels = labelsToClass(labels);

	int nclasses = labels.cols;
	int nfeatures = features.cols;
	cv::Ptr<cv::ml::ANN_MLP> ann = cv::ml::ANN_MLP::create();
	cv::Mat_<int> layers(5, 1);
	layers.at<int>(0, 0) = nfeatures;
	layers.at<int>(1, 0) = 60;
	layers.at<int>(2, 0) = 30;
	layers.at<int>(3, 0) = 15;
	layers.at<int>(4, 0) = nclasses;
	ann->setLayerSizes(layers);
	ann->setActivationFunction(cv::ml::ANN_MLP::SIGMOID_SYM); 

	int value = cv::TermCriteria::MAX_ITER;
	int value2 = cv::TermCriteria::EPS;

	ann->setTermCriteria(cv::TermCriteria(cv::TermCriteria::EPS, 1470900, 0.00001));
	ann->setTrainMethod(cv::ml::ANN_MLP::BACKPROP, 0.0001);
	printf("Training...\n");

	//ann->train(features, cv::ml::ROW_SAMPLE, labels);
	
	//ann->save("nn.yml");

	ann = cv::Algorithm::load<cv::ml::ANN_MLP>("nn.yml");

	int truePredCount = 0;
	for (int i = 0; i < features.rows; i++) {
		cv::Mat result;
		int pred = ann->predict(features.row(i), result);
		
		printf("%d : %d\n", trainingLabels[i], pred);
		if (trainingLabels[i] == pred) {
			truePredCount++;
		}
	}
	printf("\n\n");
	printf("%d / %d", truePredCount, labels.rows);
}

std::string numberToNDigitsString(int n, int number) {
	std::string numberString = std::to_string(number);
	
	if (numberString.length() > n) {
		return "";
	}

	std::stringstream ss;
	ss << std::setw(n) << std::setfill('0') << number;
	return ss.str();
}

vector<string> listFilesInDirectory(string folder)
{
	vector<string> names;
	string search_path = folder + "/*.*";
	WIN32_FIND_DATA fd;

	wstring wide_string = wstring(search_path.begin(), search_path.end());

	HANDLE hFind = ::FindFirstFile(wide_string.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE) {
		do {
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
				wstring ws(fd.cFileName);
				std::string str(ws.begin(), ws.end());
				names.push_back(str);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	return names;
}

vector<int> labelsToClass(cv::Mat labels) {
	vector<int> result;
	for (int y = 0; y < labels.rows; y++)
	{
		float maxValue = -1;
		int label = 0;
		for (int x = 0; x < labels.cols; x++)
		{
			if (maxValue < labels.at<float>(y, x)) {
				maxValue = labels.at<float>(y, x);
				label = x;
			}
		}
		result.push_back(label);
	}
	return result;
}