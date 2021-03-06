#include "pch.h"
#include <math.h>
#include <algorithm> 

struct ConvolutionMatInfo {
	cv::Mat* matrix;
	int scale;
	int boundaryValue;
};

cv::Mat doEdgeDetectionByConvMasks(cv::Mat sourceImage);
cv::Mat doCannyEdgeDetection(cv::Mat sourceImage);
cv::Mat getEdgeGradientOfImageAndDirection(cv::Mat sourceImage, cv::Mat& edgeDirectionImage);
cv::Mat applyEdgeThining(cv::Mat edgeGradientImage, cv::Mat edgeDirectionImage);
cv::Mat createGImage(cv::Mat & gMaskX, cv::Mat & gMaskY);
inline ConvolutionMatInfo createEdgeGConvMaskY();
inline ConvolutionMatInfo createEdgeGConvMaskX();
inline ConvolutionMatInfo createGaussianBlurMatrix3x3Info();
inline ConvolutionMatInfo createBoxBlurMatrixInfo();
inline ConvolutionMatInfo createGaussianBlurMatrix5x5Info();
template<typename _Tp> int getScaleOfConvolusionMask(cv::Mat* convolutionMask);
int getBoundaryValueOfConvolusionMask(cv::Mat* convolutionMask);
bool isConvolutionMaskInRightFormat(cv::Mat* convolutionMask);
template<typename _Tp> float calculateConvolusionForPixel(int x, int y, ConvolutionMatInfo convolutionMask, cv::Mat* image);
template<typename _Tp> void convertNarrowBandImageToFullGrayScale(cv::Mat img);
template<typename _Tp> void doConvulsionOnImage(cv::Mat img, cv::Mat newImg, ConvolutionMatInfo convolusionMask);
cv::Mat normalize(cv::Mat matrix);
cv::Mat applyDoubleTresholdingOnEdgeImage(cv::Mat edgeImage, float t1 = 0.01, float t2 = 0.12);
float resolveMiddleTresholdForPoint(cv::Mat tresholdImage, int y,int x);

cv::Mat normalize(cv::Mat matrix)
{
	double min, max;
	cv::minMaxLoc(matrix, &min, &max);

	for (int y = 0; y < matrix.rows; y++)
	{
		for (int x = 0; x < matrix.cols; x++)
		{
			matrix.at<float>(y, x) = (matrix.at<float>(y, x) - (float)min) * 1.0 / (max - min);
		}
	}
	return matrix;
}


float resolveMiddleTresholdForPoint(cv::Mat tresholdImage, int y, int x) {
	if (y == 0 || x == 0 || y == tresholdImage.rows - 1 || x == tresholdImage.cols -1) {
		return 0.0f;
	}
	
	float leftUp = tresholdImage.at<float>(y - 1, x - 1);
	float rightUp = tresholdImage.at<float>(y - 1, x + 1);
	float leftDown = tresholdImage.at<float>(y + 1, x - 1);
	float rightDown = tresholdImage.at<float>(y + 1, x + 1);
	float middleUp = tresholdImage.at<float>(y - 1, x);
	float middleDown = tresholdImage.at<float>(y + 1, x);
	float middleLeft = tresholdImage.at<float>(y, x - 1);
	float middleRight = tresholdImage.at<float>(y, x + 1);

	float pointsArray[] = {leftUp , rightUp, leftDown, rightDown, middleUp, middleDown, middleLeft, middleRight};

	float maxOfPoints = *std::max_element(pointsArray, pointsArray + 8);

	if (maxOfPoints == 1.0f) {
		return 1.0f;
	}
	
	if (leftUp == 0.5) {
		return resolveMiddleTresholdForPoint(tresholdImage, y - 1, x - 1);
	}

	if (rightUp == 0.5) {
		return resolveMiddleTresholdForPoint(tresholdImage, y - 1, x + 1);
	}

	if (leftDown == 0.5) {
		return resolveMiddleTresholdForPoint(tresholdImage, y + 1, x - 1);
	}

	if (rightDown == 0.5) {
		return resolveMiddleTresholdForPoint(tresholdImage, y + 1, x + 1);
	}

	if (middleUp == 0.5) {
		return resolveMiddleTresholdForPoint(tresholdImage, y - 1, x);
	}

	if (middleDown == 0.5) {
		return resolveMiddleTresholdForPoint(tresholdImage, y + 1, x);
	}

	if (middleLeft == 0.5) {
		return resolveMiddleTresholdForPoint(tresholdImage, y, x - 1);
	}

	if (middleRight == 0.5) {
		return resolveMiddleTresholdForPoint(tresholdImage, y, x + 1);
	}
	
	return 0.0f;
}


cv::Mat applyDoubleTresholdingOnEdgeImage(cv::Mat edgeImage, float t1, float t2) {
	cv::Mat resultImage = cv::Mat::zeros(edgeImage.rows, edgeImage.cols, CV_32FC1);

	for (int y = 0; y < resultImage.rows; y++)
	{
		for (int x = 0; x < resultImage.cols; x++)
		{
		    if (edgeImage.at<float>(y, x) > t2) {
				resultImage.at<float>(y, x) = 1.0f;
			} 
			else if (edgeImage.at<float>(y, x) > t1) {
				resultImage.at<float>(y, x) = 0.5f;
			}
		}
	}

	for (int y = 1; y < resultImage.rows; y++)
	{
		for (int x = 1; x < resultImage.cols; x++)
		{
			if (resultImage.at<float>(y, x) == 0.5f) {
				resultImage.at<float>(y, x) = resolveMiddleTresholdForPoint(edgeImage, y, x);
			}
		}
	}

	return resultImage;
}

cv::Mat applyEdgeThining(cv::Mat edgeGradientImage, cv::Mat edgeDirectionImage) {
	cv::Mat resultImage = cv::Mat::zeros(edgeGradientImage.rows, edgeGradientImage.cols, CV_32FC1);

	for (int y = 1; y < resultImage.rows - 1; y++) {
		for (int x = 1; x < resultImage.cols - 1; x++) {
			cv::Point octantEdgePoints[] = { cv::Point(x + 1, y), cv::Point(x + 1, y - 1), cv::Point(x, y - 1), cv::Point(x - 1, y - 1),
								cv::Point(x - 1, y), cv::Point(x - 1, y + 1), cv::Point(x, y + 1), cv::Point(x + 1, y + 1) };

			float octant1 = M_PI / 4.0;
			float octant2 = M_PI / 2.0;
			float octant3 = (3.0 / 4.0) * M_PI;
			float octant4 = M_PI;

			float angle = edgeDirectionImage.at<float>(y, x);
			float a = tan(angle);
			float interpolatedEdgePointPlus, interpolatedEdgePointMinus;

			if (angle <= octant1) {
				interpolatedEdgePointPlus = a * edgeGradientImage.at<float>(octantEdgePoints[1]) + (1 - a) * edgeGradientImage.at<float>(octantEdgePoints[0]);
				interpolatedEdgePointMinus = a * edgeGradientImage.at<float>(octantEdgePoints[5]) + (1 - a) * edgeGradientImage.at<float>(octantEdgePoints[4]);
			}
			if (angle <= octant2 && angle > octant1) {
				interpolatedEdgePointPlus = a * edgeGradientImage.at<float>(octantEdgePoints[2]) + (1 - a) * edgeGradientImage.at<float>(octantEdgePoints[1]);
				interpolatedEdgePointMinus = a * edgeGradientImage.at<float>(octantEdgePoints[6]) + (1 - a) * edgeGradientImage.at<float>(octantEdgePoints[5]);
			}
			if (angle <= octant3 && angle > octant2) {
				interpolatedEdgePointPlus = a * edgeGradientImage.at<float>(octantEdgePoints[3]) + (1 - a) * edgeGradientImage.at<float>(octantEdgePoints[2]);
				interpolatedEdgePointMinus = a * edgeGradientImage.at<float>(octantEdgePoints[7]) + (1 - a) * edgeGradientImage.at<float>(octantEdgePoints[6]);
			}
			if (angle <= octant4 && angle > octant3) {
				interpolatedEdgePointPlus = a * edgeGradientImage.at<float>(octantEdgePoints[4]) + (1 - a) * edgeGradientImage.at<float>(octantEdgePoints[3]);
				interpolatedEdgePointMinus = a * edgeGradientImage.at<float>(octantEdgePoints[0]) + (1 - a) * edgeGradientImage.at<float>(octantEdgePoints[7]);
			}
			if (edgeGradientImage.at<float>(y, x) > interpolatedEdgePointPlus && edgeGradientImage.at<float>(y, x) > interpolatedEdgePointMinus) {
				resultImage.at<float>(y, x) = edgeGradientImage.at<float>(y, x);
			}
		}
	}
	resultImage = normalize(resultImage);
	return resultImage;
}

cv::Mat doCannyEdgeDetection(cv::Mat sourceImage) {
	cv::Mat floatImage, edgedirectionImage, edgeGradientImage, resultImage;
	sourceImage.convertTo(floatImage, CV_32FC1, 1.0 / 255.0);


	edgeGradientImage = getEdgeGradientOfImageAndDirection(floatImage, edgedirectionImage);
	cv::imshow("Edge gradient image", edgeGradientImage);

	resultImage = applyEdgeThining(edgeGradientImage, edgedirectionImage);
	return resultImage;
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

cv::Mat getEdgeGradientOfImageAndDirection(cv::Mat sourceImage, cv::Mat& edgeDirectionImage)
{
	cv::Mat edgeGradientImage = sourceImage.clone();
	edgeDirectionImage = sourceImage.clone();
	float fx, fy;

	for (int y = 1; y < edgeGradientImage.rows - 1; y++) {
		for (int x = 1; x < edgeGradientImage.cols - 1; x++) {
			fy = (sourceImage.at<float>(y - 1, x) - sourceImage.at<float>(y + 1, x)) / 2.0;
			fx = (sourceImage.at<float>(y, x - 1) - sourceImage.at<float>(y, x + 1)) / 2.0;
			edgeGradientImage.at<float>(y, x) = sqrt(pow(fy, 2) + pow(fx, 2));
			edgeDirectionImage.at<float>(y, x) = atan2(fy, fx);
		}
	}

	return edgeGradientImage;
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

template<typename _Tp> float calculateConvolusionForPixel(int x, int y, ConvolutionMatInfo convolutionMask, cv::Mat* image) {
	float sumOfProducts = 0;
	
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
	ConvolutionMatInfo convolutionMaskGausian5x5, convolutionMaskGausian3x3, convolutionMaskBoxBlur;

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
	cv::imshow("same matrix box blur convulsion", new_img_8uc1_img);

	cv::Mat sourceValveImage = cv::imread("../images/valve.png", CV_LOAD_IMAGE_GRAYSCALE);

	cv::Mat edgeDetectionOnByConvolusion = doEdgeDetectionByConvMasks(sourceValveImage);
	cv::imshow("edge detection on valve by convolution", edgeDetectionOnByConvolusion);

	cv::Mat edgeDetectionOnValveNumericDerivation = doCannyEdgeDetection(sourceValveImage);
	cv::imshow("edge detection on valve by canny edge detection", edgeDetectionOnValveNumericDerivation);

	cv::Mat doubleTresholdedImage = applyDoubleTresholdingOnEdgeImage(edgeDetectionOnValveNumericDerivation);
	cv::imshow("double tresholded image", doubleTresholdedImage);

	cv::waitKey(0); // wait until keypressed
	return 0;
}