#include "pch.h"
#include <math.h>

struct ConvolutionMatInfo {
	cv::Mat* matrix;
	int scale;
	int boundaryValue;
};

cv::Mat doEdgeDetectionByConvMasks(cv::Mat sourceImage);
cv::Mat doEdgeDetectionByNumericDerivation(cv::Mat sourceImage);


cv::Mat numericDeriveByX(cv::Mat & image);
cv::Mat numericDeriveByY(cv::Mat & image);
cv::Mat createGImage(cv::Mat & gMaskX, cv::Mat & gMaskY);
inline ConvolutionMatInfo createEdgeGConvMaskY();
inline ConvolutionMatInfo createEdgeGConvMaskX();
inline ConvolutionMatInfo createGaussianBlurMatrix3x3Info();
inline ConvolutionMatInfo createBoxBlurMatrixInfo();
inline ConvolutionMatInfo createGaussianBlurMatrix5x5Info();
template<typename _Tp> int getScaleOfConvolusionMask(cv::Mat* convolutionMask);
int getBoundaryValueOfConvolusionMask(cv::Mat* convolutionMask);
bool isConvolutionMaskInRightFormat(cv::Mat* convolutionMask);
template<typename _Tp> int calculateConvolusionForPixel(int x, int y, ConvolutionMatInfo convolutionMask, cv::Mat* image);
template<typename _Tp> void convertNarrowBandImageToFullGrayScale(cv::Mat img);
template<typename _Tp> void doConvulsionOnImage(cv::Mat img, cv::Mat newImg, ConvolutionMatInfo convolusionMask);

cv::Mat doEdgeDetectionByNumericDerivation(cv::Mat sourceImage) {
	cv::Mat floatImage;
	sourceImage.convertTo(floatImage, CV_32FC1, 1.0 / 255.0);
	
	cv::Mat gxDervied = numericDeriveByX(floatImage);
	cv::Mat gyDerived = numericDeriveByY(floatImage);
	cv::Mat gDerived = createGImage(gxDervied, gyDerived);

	return gDerived;
}

cv::Mat doEdgeDetectionByConvMasks(cv::Mat sourceImage) {
	cv::Mat floatImage;
	sourceImage.convertTo(floatImage, CV_32FC1, 1.0 / 255.0);

	
	ConvolutionMatInfo maskGxInfo = createEdgeGConvMaskX();
	ConvolutionMatInfo maskGyInfo = createEdgeGConvMaskY();

	cv::Mat Gx = cv::Mat(floatImage.rows, floatImage.cols, floatImage.type());
	cv::Mat Gy = cv::Mat(floatImage.rows, floatImage.cols, floatImage.type());

	doConvulsionOnImage<float>(floatImage, Gx, maskGxInfo);
	doConvulsionOnImage<float>(floatImage, Gy, maskGyInfo);
	
	cv::Mat resultImage = createGImage(Gx, Gy);

	return resultImage;
}

cv::Mat createGImage(cv::Mat & gMaskX, cv::Mat & gMaskY)
{
	cv::Mat gImage = gMaskX.clone();
	for (int y = 0; y < gImage.rows; y++)
	{
		for (int x = 0; x < gImage.cols; x++)
		{
			gImage.at<float>(y, x) = sqrt(pow(gMaskX.at<float>(y, x), 2) + pow(gMaskY.at<float>(y, x), 2));
		}
	}

	return gImage;
}

cv::Mat numericDeriveByX(cv::Mat & image)
{
	cv::Mat derivedImage = image.clone();

	for (int y = 0; y < image.rows; y++)
	{
		for (int x = 1; x < image.cols - 1; x++)
		{
			derivedImage.at<float>(y, x) = (image.at<float>(y, x - 1) - image.at<float>(y, x + 1)) / 2;
		}
	}

	return derivedImage;
}

cv::Mat numericDeriveByY(cv::Mat & image)
{
	cv::Mat derivedImage = image.clone();

	for (int y = 1; y < image.rows - 1; y++)
	{
		for (int x = 0; x < image.cols; x++)
		{
			derivedImage.at<float>(y, x) = (image.at<float>(y - 1, x) - image.at<float>(y + 1, x)) / 2;
		}
	}

	return derivedImage;
}

inline ConvolutionMatInfo createEdgeGConvMaskY()
{
	ConvolutionMatInfo maskInfo;
	maskInfo.matrix = new cv::Mat(3, 3, CV_32FC1);

	maskInfo.matrix->at<float>(0, 0) = 1;
	maskInfo.matrix->at<float>(0, 1) = 2;
	maskInfo.matrix->at<float>(0, 2) = 1;

	maskInfo.matrix->at<float>(1, 0) = 0;
	maskInfo.matrix->at<float>(1, 1) = 0;
	maskInfo.matrix->at<float>(1, 2) = 0;
	
	maskInfo.matrix->at<float>(2, 0) = -1;
	maskInfo.matrix->at<float>(2, 1) = -2;
	maskInfo.matrix->at<float>(2, 2) = -1;
	
	maskInfo.boundaryValue = getBoundaryValueOfConvolusionMask(maskInfo.matrix);
	maskInfo.scale = 1;
	return maskInfo;
}

inline ConvolutionMatInfo createEdgeGConvMaskX()
{
	ConvolutionMatInfo maskInfo;
	maskInfo.matrix = new cv::Mat(3, 3, CV_32FC1);

	maskInfo.matrix->at<float>(0, 0) = 1;
	maskInfo.matrix->at<float>(0, 1) = 0;
	maskInfo.matrix->at<float>(0, 2) = -1;

	maskInfo.matrix->at<float>(1, 0) = 2;
	maskInfo.matrix->at<float>(1, 1) = 0;
	maskInfo.matrix->at<float>(1, 2) = -2;

	maskInfo.matrix->at<float>(2, 0) = 1;
	maskInfo.matrix->at<float>(2, 1) = 0;
	maskInfo.matrix->at<float>(2, 2) = -1;
	
	maskInfo.boundaryValue = getBoundaryValueOfConvolusionMask(maskInfo.matrix);
	maskInfo.scale = 1;
	return maskInfo;
}

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

	for (int i = 0; i < boxBlurMatrxixInfo.matrix->rows; i++) {
		for (int j = 0; j < boxBlurMatrxixInfo.matrix->cols; j++) {
			boxBlurMatrxixInfo.matrix->at<uchar>(i, j) = 1;
		}
	}

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

	if (scale == 0) {
		scale = 1;
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
	_Tp sumOfProducts = 0;
	
	for (int imY = y - convolutionMask.boundaryValue, maskY = 0; maskY < convolutionMask.matrix->rows;maskY++, imY++) {
		for (int imX = x - convolutionMask.boundaryValue, maskX = 0; maskX < convolutionMask.matrix->cols; maskX++, imX++) {
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


template<typename _Tp> void doConvulsionOnImage(cv::Mat img, cv::Mat newImg, ConvolutionMatInfo convolusionMask) {
	for (int y = convolusionMask.boundaryValue; y < img.rows - convolusionMask.boundaryValue; y++) {
		for (int x = convolusionMask.boundaryValue; x < img.cols - convolusionMask.boundaryValue; x++) {
			int sumOfProducts = 0;

			newImg.at<_Tp>(y, x) = calculateConvolusionForPixel<_Tp>(x, y, convolusionMask, &img);
		}
	}
}

int main()
{
	/*ConvolutionMatInfo convolutionMaskGausian5x5, convolutionMaskGausian3x3, convolutionMaskBoxBlur;

	convolutionMaskGausian5x5 = createGaussianBlurMatrix5x5Info();
	convolutionMaskGausian3x3 = createGaussianBlurMatrix3x3Info();
	convolutionMaskBoxBlur = createBoxBlurMatrixInfo();

	cv::Mat moon_8uc1_img = cv::imread("../images/moon.jpg", CV_LOAD_IMAGE_GRAYSCALE);
	cv::Mat new_img_8uc1_img = moon_8uc1_img.clone();
	convertNarrowBandImageToFullGrayScale<uchar>(moon_8uc1_img);
	
	cv::imshow("before convulsion", moon_8uc1_img);

	doConvulsionOnImage<uchar>(moon_8uc1_img, new_img_8uc1_img, convolutionMaskGausian5x5);
	cv::imshow("after gausian 5x5 convulsion", new_img_8uc1_img);

	doConvulsionOnImage<uchar>(moon_8uc1_img, new_img_8uc1_img, convolutionMaskGausian3x3);
	cv::imshow("after gausian 3x3 convulsion", new_img_8uc1_img);

	doConvulsionOnImage<uchar>(moon_8uc1_img, new_img_8uc1_img, convolutionMaskBoxBlur);
	cv::imshow("after box blur convulsion", new_img_8uc1_img);

	doConvulsionOnImage<uchar>(moon_8uc1_img, moon_8uc1_img, convolutionMaskBoxBlur);
	cv::imshow("same matrix box blur convulsion", new_img_8uc1_img);*/

	cv::Mat sourceValveImage = cv::imread("../images/valve.png", CV_LOAD_IMAGE_GRAYSCALE);

	cv::Mat edgeDetectionOnByConvolusion = doEdgeDetectionByConvMasks(sourceValveImage);
	cv::imshow("edge detection on valve by convolution", edgeDetectionOnByConvolusion);

	cv::Mat edgeDetectionOnValveNumericDerivation = doEdgeDetectionByNumericDerivation(sourceValveImage);
	cv::imshow("edge detection on valve by numeric derivation", edgeDetectionOnValveNumericDerivation);

	cv::waitKey(0); // wait until keypressed
	return 0;
}