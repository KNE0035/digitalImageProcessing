// Tomograph.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>

cv::Mat rotateImage(const cv::Mat & inImg, const double angle);
cv::Mat createImage();
double* countPixels(const cv::Mat & inImg);

cv::Mat sumImages(cv::Mat img1, cv::Mat img2);

cv::Mat rotateImage(const cv::Mat & inImg, const double angle) {
	cv::Point center = cv::Point(inImg.cols / 2, inImg.rows / 2);
	double scale = 1.0;

	cv::Mat rotMat(2, 3, CV_32FC1);
	cv::Mat rotImg;

	rotMat = cv::getRotationMatrix2D(center, angle, scale);
	cv::warpAffine(inImg, rotImg, rotMat, inImg.size());

	return rotImg;
}

cv::Mat sumImages(cv::Mat img1, cv::Mat img2) {
	for (int y = 0; y < img1.rows; y++)
	{
		for (int x = 0; x < img1.cols; x++) {
			img1.at<double>(y, x) = img1.at<double>(y, x) + img2.at<double>(y, x);
		}
	}

	return img1;
}

double* countPixels(const cv::Mat & inImg){
	double* pixelTable = new double[inImg.rows];

	for (int i = 0; i < inImg.rows; i++) 
	{
		pixelTable[i] = 0;
	}
	
	for (int y = 0; y < inImg.rows; y++) 
	{
		for (int x = 0; x < inImg.cols; x++) {
			pixelTable[y] += inImg.at<double>(y, x);
		}
	}
	return pixelTable;
}

cv::Mat createImage() {
	cv::Mat img = cv::Mat::zeros(150, 150, CV_64FC1);
	cv::circle(img, cv::Point{ img.cols / 2, img.rows / 2 }, 50, 1);
	cv::rectangle(img, cv::Point{80, 80}, cv::Point{145, 145}, 0.4, -1);
	
	cv::imshow("input imaga", img);

	cv::waitKey();
	return img;
}


int main()
{
	cv::Mat originalImage = createImage();
	cv::Mat finalImg = cv::Mat::zeros(originalImage.rows, originalImage.cols, CV_64FC1);

	for (int i = 0; i < 360; i++) {
		cv::Mat rotatedImg = rotateImage(originalImage, i);
		double* pixelCount = countPixels(rotatedImg);



	}
    std::cout << "Hello World!\n"; 
}

// Spuštění programu: Ctrl+F5 nebo nabídka Ladit > Spustit bez ladění
// Ladění programu: F5 nebo nabídka Ladit > Spustit ladění

// Tipy pro zahájení práce:
//   1. K přidání nebo správě souborů použijte okno Průzkumník řešení.
//   2. Pro připojení ke správě zdrojového kódu použijte okno Team Explorer.
//   3. K zobrazení výstupu sestavení a dalších zpráv použijte okno Výstup.
//   4. K zobrazení chyb použijte okno Seznam chyb.
//   5. Pokud chcete vytvořit nové soubory kódu, přejděte na Projekt > Přidat novou položku. Pokud chcete přidat do projektu existující soubory kódu, přejděte na Projekt > Přidat existující položku.
//   6. Pokud budete chtít v budoucnu znovu otevřít tento projekt, přejděte na Soubor > Otevřít > Projekt a vyberte příslušný soubor .sln.
