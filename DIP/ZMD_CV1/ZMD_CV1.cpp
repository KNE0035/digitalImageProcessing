// ZMD_CV1.cpp : Tento soubor obsahuje funkci main. Provádění programu se tam zahajuje a ukončuje.
//

#include "pch.h"
#include <iostream>

int main()
{
	cv::Mat rgbInput;
	cv::Mat Y, U, V;
	cv::Mat RBGToYUVMat = cv::Mat(3, 3, CV_32FC1);



	RBGToYUVMat.at<float>(0, 0) = 0.299;
	RBGToYUVMat.at<float>(0, 1) = 0.587;
	RBGToYUVMat.at<float>(0, 2) = 0.114;

	RBGToYUVMat.at<float>(1, 0) = -0,14713;
	RBGToYUVMat.at<float>(1, 1) = -0.28886;
	RBGToYUVMat.at<float>(1, 2) = 0.436;
	
	RBGToYUVMat.at<float>(2, 0) = 0.615;
	RBGToYUVMat.at<float>(2, 1) = -0.51499;
	RBGToYUVMat.at<float>(2, 2) = -0.10001;

	rgbInput = cv::imread("../images/lena.png", CV_LOAD_IMAGE_COLOR);
	rgbInput.convertTo(rgbInput, CV_32FC3, 1.0 / 255);
	Y = cv::Mat(rgbInput.cols, rgbInput.rows, CV_32FC1);
	U = cv::Mat(rgbInput.cols, rgbInput.rows, CV_32FC1);
	V = cv::Mat(rgbInput.cols, rgbInput.rows, CV_32FC1);

	for (int y = 0; y < rgbInput.rows; y++) {
		for (int x = 0; x < rgbInput.cols; x++) {
			cv::Vec3f rgbValue = rgbInput.at<cv::Vec3f>(y, x);
			cv::Mat yuv = RBGToYUVMat * cv::Mat(rgbValue);
			
			Y.at<float>(y, x) = yuv.at<float>(0, 0);
			U.at<float>(y, x) = yuv.at<float>(1, 0);
			V.at<float>(y, x) = yuv.at<float>(2, 0);
		}
	}

	cv::imshow("Y image", Y);
	cv::imshow("U image", U);
	cv::imshow("V image", V);


	cv::imshow("Display window", rgbInput);
	cv::waitKey(0);

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
