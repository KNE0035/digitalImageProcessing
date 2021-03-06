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
template <typename _Tp> void switchQuadrants(cv::Mat *mat);
cv::Mat_<std::complex<double>> applyPassFilter(cv::Mat_<std::complex<double>> complexFrequencyMat, cv::Point centerOfFiltrationCircle, int radiusOfFiltrationCircle, bool lowPass);
cv::Mat_<std::complex<double>> applyBarsFilter(cv::Mat_<std::complex<double>> complexFrequencyMat);
cv::Mat_<std::complex<double>> applyMask(cv::Mat mask, cv::Mat_<std::complex<double>> complexFrequencyMat);




cv::Mat resizeImg(cv::Mat img, int width, int height) {
	cv::Size size(width, height);
	cv::Mat dst;//dst image
	resize(img, dst, size);
	return dst;
}

cv::Mat_<std::complex<double>> applyMask(cv::Mat mask, cv::Mat_<std::complex<double>> complexFrequencyMat) {
	for (int y = 0; y < complexFrequencyMat.rows; y++) {
		for (int x = 0; x < complexFrequencyMat.cols; x++) {
			complexFrequencyMat.at<std::complex<double>>(y, x) = complexFrequencyMat.at<std::complex<double>>(y, x) * mask.at<double>(y, x);
		}
	}
	return complexFrequencyMat;
}

cv::Mat_<std::complex<double>> applyPassFilter(cv::Mat_<std::complex<double>> complexFrequencyMat, cv::Point centerOfFiltrationCircle, int radiusOfFiltrationCircle, bool lowPass) {
	cv::Mat passFilter;
	switchQuadrants<std::complex<double>>(&complexFrequencyMat);
	
	if (lowPass) {
		passFilter = cv::Mat::zeros(complexFrequencyMat.rows, complexFrequencyMat.cols, CV_64FC1);
		cv::circle(passFilter, cv::Point{ centerOfFiltrationCircle.x, centerOfFiltrationCircle.y }, radiusOfFiltrationCircle, 1, -1);
	}
	else {
		passFilter = cv::Mat::ones(complexFrequencyMat.rows, complexFrequencyMat.cols, CV_64FC1);
		cv::circle(passFilter, cv::Point{ centerOfFiltrationCircle.x, centerOfFiltrationCircle.y }, radiusOfFiltrationCircle, 0, -1);
	}
	
	complexFrequencyMat = applyMask(passFilter, complexFrequencyMat);
	
	switchQuadrants<std::complex<double>>(&complexFrequencyMat);

	return complexFrequencyMat;
}


cv::Mat_<std::complex<double>> applyBarsFilter(cv::Mat_<std::complex<double>> complexFrequencyMat) {
	int rectWidth = 1;
	int centerTolerance = 12;
	cv::Mat passFilter = cv::Mat::ones(complexFrequencyMat.rows, complexFrequencyMat.cols, CV_64FC1);
	switchQuadrants<std::complex<double>>(&complexFrequencyMat);

	cv::rectangle(passFilter, cv::Point{ 0, complexFrequencyMat.rows / 2 - rectWidth / 2}, cv::Point{ complexFrequencyMat.cols / 2 - centerTolerance, complexFrequencyMat.rows / 2 + rectWidth / 2 }, 0, -1);
	cv::rectangle(passFilter, cv::Point{ complexFrequencyMat.cols / 2 + centerTolerance, complexFrequencyMat.rows / 2 - rectWidth / 2 }, cv::Point{ complexFrequencyMat.cols, complexFrequencyMat.rows / 2 + rectWidth / 2}, 0, -1);

	//cv::imshow("pass filter", resizeImg(passFilter, 192, 192));
	
	complexFrequencyMat = applyMask(passFilter, complexFrequencyMat);

	switchQuadrants<std::complex<double>>(&complexFrequencyMat);
	return complexFrequencyMat;
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
			_Tp pom24;
			_Tp pom13;

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

	if (phaseImage != NULL) {
		phaseMat = getPhaseSpectrumMat(fComplexSpectrumMat);
		*phaseImage = phaseMat;
	}
	
	
	if (powerImage != NULL) {
		powerMat = getPowerSpectrum(fComplexSpectrumMat);
		*powerImage = powerMat;
	}

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
	cv::Mat lenaImg = cv::imread("../images/lena64_noise.png", CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat lenaBarsImg = cv::imread("../images/lena64_bars.png", CV_LOAD_IMAGE_GRAYSCALE);
	convertImgTo64FC1(&lenaImg);
	convertImgTo64FC1(&lenaBarsImg);

	cv::imshow("Lena image", resizeImg(lenaImg, 192, 192));
	cv::imshow("Lena bars image", resizeImg(lenaBarsImg, 192, 192));

	cv::Mat phaseImg = cv::Mat::zeros(lenaImg.rows, lenaImg.cols, (lenaImg.type()));
	cv::Mat powerImg = cv::Mat::zeros(lenaImg.rows, lenaImg.cols, (lenaImg.type()));

	cv::Mat fComplexSpectrumMat = cv::Mat_<std::complex<double> >(lenaImg.rows, lenaImg.cols);
	cv::Mat fComplexSpectrumMatLenaBars = cv::Mat_<std::complex<double> >(lenaBarsImg.rows, lenaBarsImg.cols);

	cv::Mat fComplexSpectrumMatFilteredLowPass = cv::Mat_<std::complex<double> >(lenaImg.rows, lenaImg.cols);
	cv::Mat fComplexSpectrumMatFilteredHighPass = cv::Mat_<std::complex<double> >(lenaImg.rows, lenaImg.cols);
	cv::Mat fComplexSpectrumMatLenaFilteredBars = cv::Mat_<std::complex<double> >(lenaBarsImg.rows, lenaBarsImg.cols);

	cv::Mat inverseFourierImageLowPassFilter = cv::Mat::zeros(lenaImg.rows, lenaImg.cols, (lenaImg.type()));;
	cv::Mat inverseFourierImageHighPassFilter = cv::Mat::zeros(lenaImg.rows, lenaImg.cols, (lenaImg.type()));;
	cv::Mat lenaWithoutBars = cv::Mat::zeros(lenaBarsImg.rows, lenaBarsImg.cols, (lenaBarsImg.type()));;

	doFourierTransformation(lenaImg, &powerImg, &phaseImg, &fComplexSpectrumMat);
	doFourierTransformation(lenaBarsImg, NULL, NULL, &fComplexSpectrumMatLenaBars);

	cv::imshow("Lena phase image", resizeImg(phaseImg, 192, 192));
	cv::imshow("Lena power image", resizeImg(powerImg, 192, 192));

	fComplexSpectrumMatFilteredLowPass = applyPassFilter(fComplexSpectrumMat.clone(), cv::Point{ fComplexSpectrumMat.cols / 2, fComplexSpectrumMat.rows / 2 },22, true);
	fComplexSpectrumMatFilteredHighPass = applyPassFilter(fComplexSpectrumMat.clone(), cv::Point{ fComplexSpectrumMat.cols / 2, fComplexSpectrumMat.rows / 2 }, 5, false);
	fComplexSpectrumMatLenaFilteredBars = applyBarsFilter(fComplexSpectrumMatLenaBars.clone());

	inverseFourierImageLowPassFilter = doInverseFourierTransformation(fComplexSpectrumMatFilteredLowPass);
	cv::imshow("Lena low pass filter image", resizeImg(inverseFourierImageLowPassFilter, 192, 192));

	inverseFourierImageHighPassFilter = doInverseFourierTransformation(fComplexSpectrumMatFilteredHighPass);
	cv::imshow("Lena high pass filter image", resizeImg(inverseFourierImageHighPassFilter, 192, 192));

	lenaWithoutBars = doInverseFourierTransformation(fComplexSpectrumMatLenaFilteredBars);
	cv::imshow("Lena without bars image", resizeImg(lenaWithoutBars, 192, 192));

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