// HistogramOfOrientedGradients.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>

struct GradientInfo {
	cv::Mat fx;
	cv::Mat fy;
};

int main()
{
	cv::Mat image = cv::imread("../images/hog_test.png", CV_LOAD_IMAGE_GRAYSCALE);
	Gra 

	/*for (int y = 1; y < image.rows - 1; y++) {
		for (int x = 1; x < image.cols -1; x++) {
			gradientInfoMat.at<gradientInfo>(y, x).fx = (double)image.at<uchar>(y, x + 1) - image.at<uchar>(y, x);
			gradientInfoMat.at<gradientInfo>(y, x).fy = (double)image.at<uchar>(y + 1, x) - image.at<uchar>(y, x);
		}
	}

	for (int y = 0; y < image.rows; y++) {
		for (int x = 0; x < image.cols; x++) {
			double fx = gradientInfoMat.at<gradientInfo>(y, x).fx;
			double fy = gradientInfoMat.at<gradientInfo>(y, x).fy;

			printf("%f, %f\n", fx, fy);
		}
	}*/

	cv::imshow("image", image);
	cv::waitKey(0);
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
