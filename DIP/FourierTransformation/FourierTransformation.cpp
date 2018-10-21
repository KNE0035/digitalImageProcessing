#include "pch.h"
#include <math.h>
#include <cmath>
#include <iostream>
#include <complex> 

void printImage(cv::Mat mat);
void doFourierTransformation(cv::Mat img, cv::Mat* powerImage, cv::Mat* phaseImage, cv::Mat* complexFrequencyMat);
cv::Mat doInverseFourierTransformation(cv::Mat_<std::complex<double>> complexFrequencyMat);
std::complex<double> getComplexFrequencySpectrumForPixel(int k, int l, cv::Mat img);
double getOriginalImageValueForPixel(int k, int l, cv::Mat_<std::complex<double>> complexFrequencyMat);
cv::Mat getPowerSpectrum(cv::Mat complexFrequencyMat);
cv::Mat getPhaseSpectrumMat(cv::Mat complexFrequencyMat);
template <typename _Tp> void normalizeRangeOfMat(cv::Mat* mat);
void convertImgTo64FC1(cv::Mat* img);
cv::Mat resizeImg(cv::Mat img, int width, int height);
template <typename _Tp> void switchQuadrants(cv::Mat *img);



cv::Mat resizeImg(cv::Mat img, int width, int height) {
	cv::Size size(width, height);
	cv::Mat dst;//dst image
	resize(img, dst, size);
	return dst;
}

template <typename _Tp> void normalizeRangeOfMat(cv::Mat* mat) {
	double min = 0, max = 0;
	cv::minMaxLoc(*mat, &min, &max);

	for (int y = 0; y < mat->rows; y++) {
		for (int x = 0; x < mat->cols; x++) {
			mat->at<_Tp>(y, x) = (mat->at<_Tp>(y, x) - min) / (max - min);
		}
	}
}

void convertImgTo64FC1(cv::Mat* img) {
	cv::Mat* FC1_64_Img = img;

	if (img->type() != CV_32FC1) {
		FC1_64_Img->convertTo(*FC1_64_Img, CV_64FC1, 1.0 / 255.0);
	}
}

template <typename _Tp> void switchQuadrants(cv::Mat *mat) {
	for (int y = 0; y < mat->rows / 2; y++) {
		for (int x = 0; x < mat->cols / 2; x++) {
			double pom24;
			double pom13;

			pom13 = mat->at<_Tp>(y + mat->rows / 2, x + mat->cols / 2);
			mat->at<_Tp>(y + mat->rows / 2, x + mat->cols / 2) = mat->at<_Tp>(y, x);
			mat->at<_Tp>(y, x) = pom13;

			pom24 = mat->at<_Tp>(y + mat->rows / 2, x);

			mat->at<_Tp>(y + mat->rows / 2, x) = mat->at<_Tp>(y, x + mat->cols / 2);
			mat->at<_Tp>(y, x + mat->cols / 2) = pom24;
		}
	}
}

cv::Mat getPowerSpectrum(cv::Mat complexFrequencyMat) {
	cv::Mat powerMat = cv::Mat::zeros(complexFrequencyMat.rows, complexFrequencyMat.cols, CV_64FC1);
	for (int y = 0; y < complexFrequencyMat.rows; y++) {
		for (int x = 0; x < complexFrequencyMat.cols; x++) {
			double imag, real;
			imag = (complexFrequencyMat.at<std::complex<double>>(y, x)).imag();
			real = (complexFrequencyMat.at<std::complex<double>>(y, x)).real();
			powerMat.at<double>(y, x) = log(pow(real, 2) + pow(imag, 2));
		}
	}

	normalizeRangeOfMat<double>(&powerMat);

	switchQuadrants<double>(&powerMat);
	return powerMat;
}


cv::Mat getPhaseSpectrumMat(cv::Mat complexFrequencyMat) {
	cv::Mat phaseMat = cv::Mat::zeros(complexFrequencyMat.rows, complexFrequencyMat.cols, CV_64FC1);
	for (int y = 0; y < complexFrequencyMat.rows; y++) {
		for (int x = 0; x < complexFrequencyMat.cols; x++) {
			double imag, real;
			imag = (complexFrequencyMat.at<std::complex<double>>(y, x)).imag();
			real = (complexFrequencyMat.at<std::complex<double>>(y, x)).real();
			phaseMat.at<double>(y, x) = atan(imag / real);
		}
	}
	normalizeRangeOfMat<double>(&phaseMat);

	return phaseMat;
}

std::complex<double> getComplexFrequencySpectrumForPixel(int k, int l, cv::Mat img) {
	std::complex<double> result;
	double sumRe = 0.0, sumImag = 0.0;

	for (int m = 0; m < img.rows; m++) {
		for (int n = 0; n < img.cols; n++) {
			double argument =  2 * M_PI * ((m * k) / (double)img.rows + (n * l) / (double)img.cols);

			sumRe += img.at<double>(m, n) * cos(argument);
			sumImag += -img.at<double>(m, n) * (sin(argument));
		}
	}
	result.real(sumRe);
	result.imag(sumImag);
	return result;
}

double getOriginalImageValueForPixel(int k, int l, cv::Mat_<std::complex<double>> complexFrequencyMat) {
	std::complex<double> sum = 0;

	sum.imag(0);
	sum.real(0);

	double normalizeConstant = 1.0 / (double)(complexFrequencyMat.rows * complexFrequencyMat.cols);
	std::complex<double> complexValueFromBase;
	for (int m = 0; m < complexFrequencyMat.rows; m++) {
		for (int n = 0; n < complexFrequencyMat.cols; n++) {
			double argument = 2 * M_PI * ((m * k) / (double)complexFrequencyMat.cols + (n * l) / (double)complexFrequencyMat.rows);

			complexValueFromBase.real(cos(argument));
			complexValueFromBase.imag(sin(argument));

			sum += complexFrequencyMat.at<std::complex<double>>(m, n).real() * complexValueFromBase.real() - 
				   complexFrequencyMat.at<std::complex<double>>(m, n).imag() * complexValueFromBase.imag();
		}
	}
	return (sum.real() * normalizeConstant);
}

void doFourierTransformation(cv::Mat img, cv::Mat* powerImage, cv::Mat* phaseImage, cv::Mat* complexFrequencyMat){
	cv::Mat fComplexSpectrumMat = cv::Mat_<std::complex<double> >(img.rows, img.cols);
	cv::Mat powerMat = cv::Mat::zeros(img.rows, img.cols, (img.type()));
	cv::Mat phaseMat = cv::Mat::zeros(img.rows, img.cols, (img.type()));

	for (int k = 0; k < img.rows; k++) {
		for (int l = 0; l < img.cols; l++) {
			fComplexSpectrumMat.at<std::complex<double>>(k, l) = getComplexFrequencySpectrumForPixel(k, l, img);
		}
	}

	phaseMat = getPhaseSpectrumMat(fComplexSpectrumMat);
	powerMat = getPowerSpectrum(fComplexSpectrumMat);

	*powerImage = powerMat;
	*phaseImage = phaseMat;
	*complexFrequencyMat = fComplexSpectrumMat;
}

cv::Mat doInverseFourierTransformation(cv::Mat_<std::complex<double>> complexFrequencyMat) {

	cv::Mat originalImage = cv::Mat::zeros(complexFrequencyMat.rows, complexFrequencyMat.cols, CV_64FC1);

	for (int k = 0; k < originalImage.rows; k++) {
		for (int l = 0; l < originalImage.cols; l++) {
			originalImage.at<double>(k, l) = getOriginalImageValueForPixel(k, l, complexFrequencyMat);
		}
	}
	return originalImage;
}

int main()
{
	cv::Mat lenaImg = cv::imread("../images/lena64.png", CV_LOAD_IMAGE_GRAYSCALE);
	convertImgTo64FC1(&lenaImg);

	cv::imshow("Lena image", resizeImg(lenaImg, 192, 192));

	cv::Mat phaseImg = cv::Mat::zeros(lenaImg.rows, lenaImg.cols, (lenaImg.type()));;
	cv::Mat powerImg = cv::Mat::zeros(lenaImg.rows, lenaImg.cols, (lenaImg.type()));;
	cv::Mat fComplexSpectrumMat = cv::Mat_<std::complex<double> >(lenaImg.rows, lenaImg.cols);
	cv::Mat inverseFourierImage = cv::Mat::zeros(lenaImg.rows, lenaImg.cols, (lenaImg.type()));;
	

	doFourierTransformation(lenaImg, &powerImg, &phaseImg, &fComplexSpectrumMat);
	cv::imshow("Lena phase image", resizeImg(phaseImg, 192, 192));
	cv::imshow("Lena power image", resizeImg(powerImg, 192, 192));

	inverseFourierImage = doInverseFourierTransformation(fComplexSpectrumMat);
	cv::imshow("Lena inverse fourier image", resizeImg(inverseFourierImage, 192, 192));

	for (int y = 0; y < lenaImg.rows; y++) {
		for (int x = 0; x < lenaImg.cols; x++) {
			printf("invers %f - orig %f = %f", inverseFourierImage.at<double>(y, x), lenaImg.at<double>(y, x), inverseFourierImage.at<double>(y, x) - lenaImg.at<double>(y, x));
			std::cout << std::endl;
		}
	}

	cv::waitKey(0);
	
	return 0;
}

void printImage(cv::Mat mat) {
	for (int y = 0; y < mat.rows; y++) {
		for (int x = 0; x < mat.cols; x++) {
			printf("%f    ", mat.at<double>(y, x));
		}
		printf("\n");
	}
}