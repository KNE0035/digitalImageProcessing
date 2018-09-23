#include "pch.h"
#include <math.h>

struct ConvolutionMatInfo {
	cv::Mat* matrix;
	int scale;
	int boundaryValue;
};

inline ConvolutionMatInfo createGaussianBlurMatrix3x3Info();
inline ConvolutionMatInfo createBoxBlurMatrixInfo();
inline ConvolutionMatInfo createGaussianBlurMatrix5x5Info();
template<typename _Tp> int getScaleOfConvolusionMask(cv::Mat* convolutionMask);
int getBoundaryValueOfConvolusionMask(cv::Mat* convolutionMask);
bool isConvolutionMaskInRightFormat(cv::Mat* convolutionMask);
template<typename _Tp> int calculateConvolusionForPixel(int x, int y, ConvolutionMatInfo convolutionMask, cv::Mat* image);
template<typename _Tp> void convertNarrowBandImageToFullGrayScale(cv::Mat img);
template<typename _Tp> void doConvulsionOnImage(cv::Mat img, ConvolutionMatInfo convolusionMask);

inline ConvolutionMatInfo createGaussianBlurMatrix3x3Info() {
	ConvolutionMatInfo gaussBlur3x3MatrxixInfo;
	gaussBlur3x3MatrxixInfo.matrix = new cv::Mat(3, 3, CV_8UC1);

	gaussBlur3x3MatrxixInfo.matrix->at<uchar>(0, 0) = 1;
	gaussBlur3x3MatrxixInfo.matrix->at<uchar>(0, 1) = 2;
	gaussBlur3x3MatrxixInfo.matrix->at<uchar>(0, 2) = 1;

	gaussBlur3x3MatrxixInfo.matrix->at<uchar>(1, 0) = 2;
	gaussBlur3x3MatrxixInfo.matrix->at<uchar>(1, 1) = 4;
	gaussBlur3x3MatrxixInfo.matrix->at<uchar>(1, 2) = 2;

	gaussBlur3x3MatrxixInfo.matrix->at<uchar>(2, 0) = 1;
	gaussBlur3x3MatrxixInfo.matrix->at<uchar>(2, 1) = 2;
	gaussBlur3x3MatrxixInfo.matrix->at<uchar>(2, 2) = 1;

	gaussBlur3x3MatrxixInfo.boundaryValue = getBoundaryValueOfConvolusionMask(gaussBlur3x3MatrxixInfo.matrix);
	gaussBlur3x3MatrxixInfo.scale = getScaleOfConvolusionMask<uchar>(gaussBlur3x3MatrxixInfo.matrix);
	return gaussBlur3x3MatrxixInfo;
}

inline ConvolutionMatInfo createBoxBlurMatrixInfo() {
	ConvolutionMatInfo boxBlurMatrxixInfo;
	boxBlurMatrxixInfo.matrix = new cv::Mat(3, 3, CV_8UC1);

	boxBlurMatrxixInfo.matrix->at<uchar>(0, 0) = 1;
	boxBlurMatrxixInfo.matrix->at<uchar>(0, 1) = 1;
	boxBlurMatrxixInfo.matrix->at<uchar>(0, 2) = 1;

	boxBlurMatrxixInfo.matrix->at<uchar>(1, 0) = 1;
	boxBlurMatrxixInfo.matrix->at<uchar>(1, 1) = 1;
	boxBlurMatrxixInfo.matrix->at<uchar>(1, 2) = 1;

	boxBlurMatrxixInfo.matrix->at<uchar>(2, 0) = 1;
	boxBlurMatrxixInfo.matrix->at<uchar>(2, 1) = 1;
	boxBlurMatrxixInfo.matrix->at<uchar>(2, 2) = 1;

	boxBlurMatrxixInfo.boundaryValue = getBoundaryValueOfConvolusionMask(boxBlurMatrxixInfo.matrix);
	boxBlurMatrxixInfo.scale = getScaleOfConvolusionMask<uchar>(boxBlurMatrxixInfo.matrix);
	return boxBlurMatrxixInfo;
}

inline ConvolutionMatInfo createGaussianBlurMatrix5x5Info() {
	ConvolutionMatInfo gaussBlur5x5MatrxixInfo;
	gaussBlur5x5MatrxixInfo.matrix = new cv::Mat(5, 5, CV_8UC1);

	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(0, 0) = 1;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(0, 1) = 4;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(0, 2) = 6;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(0, 3) = 4;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(0, 4) = 1;

	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(1, 0) = 4;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(1, 1) = 16;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(1, 2) = 24;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(1, 3) = 16;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(1, 4) = 4;

	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(2, 0) = 6;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(2, 1) = 24;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(2, 2) = 36;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(2, 3) = 24;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(2, 4) = 6;

	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(3, 0) = 4;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(3, 1) = 16;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(3, 2) = 24;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(3, 3) = 16;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(3, 4) = 4;

	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(4, 0) = 1;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(4, 1) = 4;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(4, 2) = 6;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(4, 3) = 4;
	gaussBlur5x5MatrxixInfo.matrix->at<uchar>(4, 4) = 4;

	gaussBlur5x5MatrxixInfo.boundaryValue = getBoundaryValueOfConvolusionMask(gaussBlur5x5MatrxixInfo.matrix);
	gaussBlur5x5MatrxixInfo.scale = getScaleOfConvolusionMask<uchar>(gaussBlur5x5MatrxixInfo.matrix);

	return gaussBlur5x5MatrxixInfo;
}

template<typename _Tp> int getScaleOfConvolusionMask(cv::Mat* convolutionMask) {
	if (!isConvolutionMaskInRightFormat(convolutionMask)) {
		return -1;
	}

	int scale = 0;
	for (int y = 0; y < convolutionMask->rows; y++) {
		for (int x = 0; x < convolutionMask->cols; x++) {
			scale += convolutionMask->at<_Tp>(y, x);

		}
	}
	return scale;
}

int getBoundaryValueOfConvolusionMask(cv::Mat* convolutionMask) {
	if (!isConvolutionMaskInRightFormat(convolutionMask)) {
		return -1;
	}

	return convolutionMask->rows / 2;
}

bool isConvolutionMaskInRightFormat(cv::Mat* convolutionMask) {
	int cols = convolutionMask->cols, rows = convolutionMask->rows;

	return (cols % 2 != 0 && rows % 2 != 0 && cols == rows);
}

template<typename _Tp> int calculateConvolusionForPixel(int x, int y, ConvolutionMatInfo convolutionMask, cv::Mat* image) {
	int sumOfProducts = 0;
	
	for (int imY = y - convolutionMask.boundaryValue, maskY = 0, i = 0; i < convolutionMask.matrix->rows; i++, maskY++, imY++) {
		for (int imX = x - convolutionMask.boundaryValue, maskX = 0, j = 0; j < convolutionMask.matrix->cols; j++, maskX++, imX++) {
			if (imY == y && imX == x) {
				int test = image->at<_Tp>(imY, imX);
			}
			sumOfProducts += image->at<_Tp>(imY, imX) * convolutionMask.matrix->at<_Tp>(maskY, maskX);
		}
	}
	return sumOfProducts / convolutionMask.scale;
}

template<typename _Tp> void convertNarrowBandImageToFullGrayScale(cv::Mat img) {
	double min, max, offset;
	cv::minMaxLoc(img, &min, &max);
	offset = 255 / (max - min);
	//converting narrow band 
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			img.at<_Tp>(y, x) = (img.at<_Tp>(y, x) - min) * offset;
		}
	}
}


template<typename _Tp> void doConvulsionOnImage(cv::Mat img, ConvolutionMatInfo convolusionMask) {
	for (int y = convolusionMask.boundaryValue; y < img.rows - convolusionMask.boundaryValue; y++) {
		for (int x = convolusionMask.boundaryValue; x < img.cols - convolusionMask.boundaryValue; x++) {
			int sumOfProducts = 0;
			int test, pixelValue;

			pixelValue = img.at<uchar>(y, x);
			img.at<uchar>(y, x) = calculateConvolusionForPixel<uchar>(x, y, convolusionMask, &img);
		}
	}
}

int main()
{
	ConvolutionMatInfo convolutionMaskGausian5x5, convolutionMaskGausian3x3, convolutionMaskBoxBlur;

	convolutionMaskGausian5x5 = createGaussianBlurMatrix5x5Info();
	convolutionMaskGausian3x3 = createGaussianBlurMatrix3x3Info();
	convolutionMaskBoxBlur = createBoxBlurMatrixInfo();

	cv::Mat moon_8uc1_img = cv::imread("../images/moon.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	
	convertNarrowBandImageToFullGrayScale<uchar>(moon_8uc1_img);
	
	cv::imshow("before convulsion", moon_8uc1_img);

	doConvulsionOnImage<uchar>(moon_8uc1_img, convolutionMaskGausian5x5);
	cv::imshow("after gausian 5x5 convulsion", moon_8uc1_img);

	doConvulsionOnImage<uchar>(moon_8uc1_img, convolutionMaskGausian3x3);
	cv::imshow("after gausian 3x3 convulsion", moon_8uc1_img);

	doConvulsionOnImage<uchar>(moon_8uc1_img, convolutionMaskBoxBlur);
	cv::imshow("after box blur convulsion", moon_8uc1_img);

	cv::waitKey(0); // wait until keypressed
	return 0;
}