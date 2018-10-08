#include "pch.h"
#include <math.h>
#include <cmath>
#include <iostream>
#include <complex> 

void doFourierTransformation(cv::Mat img, cv::Mat* powerImage, cv::Mat* phaseImage);
std::complex<double> getComplexFrequencySpectrumForPixel(int k, int l, cv::Mat img);
cv::Mat getPowerSpectrum(cv::Mat complexFrequencyMat);
cv::Mat getPhaseSpectrumMat(cv::Mat complexFrequencyMat);
void normalizeImageBySquareRoot(cv::Mat* img);
void convertImgTo64FC1(cv::Mat* img);
cv::Mat resizeImg(cv::Mat img, int width, int height);


cv::Mat resizeImg(cv::Mat img, int width, int height) {
	cv::Size size(width, height);
	cv::Mat dst;//dst image
	resize(img, dst, size);
	return dst;
}

void convertImgTo64FC1(cv::Mat* img) {
	cv::Mat* FC1_64_Img = img;

	if (img->type() != CV_32FC1) {
		FC1_64_Img->convertTo(*FC1_64_Img, CV_64FC1, 1.0 / 255.0);
	}
}

void normalizeImageBySquareRoot(cv::Mat* img) {
	for (int y = 0; y < img->rows; y++) {
		for (int x = 0; x < img->cols; x++) {
			img->at<double>(y, x) = img->at<double>(y, x) / sqrt(img->rows * img->cols);
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

	double min = 0, max = 0;
	cv::minMaxLoc(powerMat, &min, &max);

	for (int y = 0; y < complexFrequencyMat.rows; y++) {
		for (int x = 0; x < complexFrequencyMat.cols; x++) {
			powerMat.at<double>(y, x) = (powerMat.at<double>(y, x) - min) / (max - min);
		}
	}

	for (int y = 0; y < complexFrequencyMat.rows / 2 ; y++) {
		for (int x = 0; x < complexFrequencyMat.cols / 2; x++) {
			double pom24;
			double pom13;

			pom13 = powerMat.at<double>(y + complexFrequencyMat.rows / 2, x + complexFrequencyMat.cols / 2);
			powerMat.at<double>(y + complexFrequencyMat.rows / 2, x + complexFrequencyMat.cols / 2) = powerMat.at<double>(y, x);
			powerMat.at<double>(y, x) = pom13;

			pom24 = powerMat.at<double>(y + complexFrequencyMat.rows / 2, x);

			powerMat.at<double>(y + complexFrequencyMat.rows / 2, x) = powerMat.at<double>(y, x + complexFrequencyMat.cols / 2);
			powerMat.at<double>(y, x + complexFrequencyMat.cols / 2) = pom24;

		}
	}

	return powerMat;
}


cv::Mat getPhaseSpectrumMat(cv::Mat complexFrequencyMat) {
	cv::Mat phaseMat = cv::Mat::zeros(complexFrequencyMat.rows, complexFrequencyMat.cols, CV_64FC1);
	for (int y = 0; y < complexFrequencyMat.rows; y++) {
		for (int x = 0; x < complexFrequencyMat.cols; x++) {
			double imag, real;
			imag = (complexFrequencyMat.at<std::complex<double>>(y, x)).imag();
			real = (complexFrequencyMat.at<std::complex<double>>(y, x)).real();
			phaseMat.at<double>(y, x) =  atanf(imag / real);
		}
	}

	return phaseMat;
}

std::complex<double> getComplexFrequencySpectrumForPixel(int k, int l, cv::Mat img) {
	std::complex<double> result;
	double sumRe = 0, sumImag = 0;

	for (int n = 0; n < img.rows; n++) {
		for (int m = 0; m < img.cols; m++) {
			double argument = - 2 * 180.0 * (((n * l) / img.rows) + ((m * k) / img.cols));
			sumRe += img.at<double>(n, m) * cos(argument);
			sumImag += img.at<double>(n, m) * sin(argument);
		}
	}
	result.real(sumRe);
	result.imag(sumImag);
	return result;
}

void doFourierTransformation(cv::Mat img, cv::Mat* powerImage, cv::Mat* phaseImage){
	cv::Mat fComplexSpectrumMat = cv::Mat_<std::complex<double> >(img.rows, img.cols);
	cv::Mat powerMat = cv::Mat::zeros(img.rows, img.cols, (img.type()));
	cv::Mat phaseMat = cv::Mat::zeros(img.rows, img.cols, (img.type()));

	normalizeImageBySquareRoot(&img);

	for (int l = 0; l < img.rows; l++) {
		for (int k = 0; k < img.cols; k++) {
			fComplexSpectrumMat.at<std::complex<double>>(l, k) = getComplexFrequencySpectrumForPixel(k, l, img);
		}
	}

	phaseMat = getPhaseSpectrumMat(fComplexSpectrumMat);
	powerMat = getPowerSpectrum(fComplexSpectrumMat);

	*powerImage = powerMat;
	*phaseImage = phaseMat;
}

int main()
{
	cv::Mat lenaImg = cv::imread("../images/lena64.png", CV_LOAD_IMAGE_GRAYSCALE);
	convertImgTo64FC1(&lenaImg);
	cv::Mat phaseImg = cv::Mat::zeros(lenaImg.rows, lenaImg.cols, (lenaImg.type()));;
	cv::Mat powerImg = cv::Mat::zeros(lenaImg.rows, lenaImg.cols, (lenaImg.type()));;
	
	cv::imshow("Lena image", resizeImg(lenaImg, 162, 165));

	doFourierTransformation(lenaImg, &powerImg, &phaseImg);

	cv::imshow("Lena phase image", resizeImg(phaseImg, 162, 165));
	cv::imshow("Lena power image", resizeImg(powerImg, 162, 165));


	cv::waitKey(0); // wait until keypressed

	return 0;
}