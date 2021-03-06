#include "OldMovie.h"
#include <opencv2/opencv.hpp>
using namespace cv;

#include <iostream>
double OldMovie_filter::generateGaussianNoise(double mu, double sigma)
{
	static double V1, V2, S;
	static int phase = 0;
	double X;
	double U1, U2;
	if (phase == 0) {
		do {
			U1 = (double)rand() / RAND_MAX;
			U2 = (double)rand() / RAND_MAX;

			V1 = 2 * U1 - 1;
			V2 = 2 * U2 - 1;
			S = V1 * V1 + V2 * V2;
		} while (S >= 1 || S == 0);

		X = V1 * sqrt(-2 * log(S) / S);
	}
	else {
		X = V2 * sqrt(-2 * log(S) / S);
	}
	phase = 1 - phase;
	return mu + sigma * X;
}

Mat OldMovie_filter::AddNoise(Mat img, double mu, double sigma, int k) {
	Mat outImage;
	outImage.create( img.rows , img.cols ,img.type() );
	std::cout << outImage.rows << outImage.cols;
	for (int y = 0; y < img.rows; y  ++ ) {
		for (int x = 0; x < img.cols*3 ; x ++ ) {
			int temp = img.ptr<uchar>(y)[x] + k * generateGaussianNoise(mu, sigma);
			if (temp > 255) {
				temp = 255;
			}
			else if (temp < 0) {
				temp = 0;
			}
			outImage.ptr<uchar>(y)[x] = (uchar)temp;
			//std::cout << x << std::endl;
		}
	}
	//imshow("formal", img);
	return outImage;
}

Mat OldMovie_filter::ColorChage(Mat img) {
	int width = img.cols;
	int heigh = img.rows;
	RNG rng;
	Mat finalImage(img.size(), CV_8UC3);
	for (int y = 0; y < heigh; y++)
	{
		uchar* P0 = img.ptr<uchar>(y);
		uchar* P1 = finalImage.ptr<uchar>(y);
		for (int x = 0; x < width; x++)
		{
			float B = P0[3 * x];
			float G = P0[3 * x + 1];
			float R = P0[3 * x + 2];
			float newB = 0.272*R + 0.534*G + 0.131*B;
			float newG = 0.349*R + 0.686*G + 0.168*B;
			float newR = 0.393*R + 0.769*G + 0.189*B;
			if (newB<0)newB = 0;
			if (newB>255)newB = 255;
			if (newG<0)newG = 0;
			if (newG>255)newG = 255;
			if (newR<0)newR = 0;
			if (newR>255)newR = 255;
			P1[3 * x] = (uchar)newB;
			P1[3 * x + 1] = (uchar)newG;
			P1[3 * x + 2] = (uchar)newR;
		}
	}
	return finalImage;
}

Mat OldMovie_filter::OldImage(Mat img) {
	Mat img2 = AddNoise(img, 0, 0.5, 64);
	img2 = ColorChage(img2);
	return img2;
}

Mat OldMovie_filter::FixedVignetting(Mat img,int color) {
	Mat VigImg;
	int cols = img.cols;
	int rows = img.rows;
	int lenth = 20;
	if( color == BLACK_BACK ){
		lenth = 5;
	}
	Vec3b *p;
	VigImg.create(rows, cols, img.type());
	Mat change;
	bitwise_not(img, change);
	for (int i = 0; i < rows; i++)
	{
		p = VigImg.ptr<Vec3b>(i);
				for (int j = 0; j < cols; j++) {
				if ((rows - i) <= lenth || i <= lenth || (cols - j) <= lenth || j <= lenth) {//白框
					p[j] = { 255,255,255 };
				}
				else
					p[j] = { 0,0,0 };
			}
	}
	if (color == BLACK_BACK ) {
		blur(VigImg, VigImg, Size(50, 50));
		VigImg = (VigImg + change);
		bitwise_not(VigImg,VigImg);
	}

	else if (color == WHITE_BACK) {
		blur(VigImg, VigImg, Size(100, 100));
		VigImg = (VigImg + img);
	}

	return VigImg;
}

Mat OldMovie_filter::OldMovieImage(Mat img) {
	Mat Old = OldImage(img);
	Old = FixedVignetting(Old,BLACK_BACK);
	return Old;
}

Mat OldMovie_filter::OldPaperImage(Mat img) {
	Mat Old = OldImage(img);
	Old = FixedVignetting(Old, WHITE_BACK);
	return Old;
}

Mat OldMovie_filter::FragmentLomoImage(Mat img) {
	Mat Old = FragmentImage(img,5);
	Old = FixedVignetting(Old, BLACK_BACK);
	return Old;
}

Mat OldMovie_filter::FragmentImage(Mat imge, int Offset) {
	Mat imgeW = shift(imge, Up, Offset);
	Mat imgeS = shift(imge, Down, Offset);
	Mat imgeA = shift(imge, Left, Offset);
	Mat imgeD = shift(imge, Right, Offset);
	Mat finalImage = (imgeW + imgeA + imgeD + imgeS) / 4;
	return finalImage;
}

Mat OldMovie_filter::shift(Mat imge, int dir, int offset) {
	Mat finalImage;
	const int rows = imge.rows;
	const int cols = imge.cols;
	finalImage.create(rows, cols, imge.type());

	Vec3b *p;
	for (int i = 0; i < rows; i++) {
		p = finalImage.ptr<Vec3b>(i);
		for (int j = 0; j < cols; j++) {
			int x = j;
			int y = i;
			switch (dir) {
			case Up:
				y += offset;
				break;
			case Down:
				y -= offset;
				break;
			case Left:
				x -= offset;
				break;
			case Right:
				x += offset;
				break;
			default:
				break;
			}
			if (x >= 0 && y >= 0 && x < cols && y < rows) {
				p[j] = imge.ptr<Vec3b>(y)[x];
			}
		}
	}
	return finalImage;
}

Mat OldMovie_filter::FragmentImage(Mat img) {
	Mat Old = FragmentImage(img, 5);
	return Old;
}
