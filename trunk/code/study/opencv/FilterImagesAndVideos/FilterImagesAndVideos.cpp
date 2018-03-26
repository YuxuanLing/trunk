// FilterImagesAndVideos.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "opencv2/opencv.hpp"
#include <iostream>

using namespace std;
using namespace cv;


int ChangeImgBrightness(char *img_name, int pix_diff)
{
	Mat img = imread(img_name);
	Mat imgBrightnessDiff;

	if (img.empty())
	{
		cout << "Could not open or find the image" << endl;
		cin.get();
		return -1;
	}

	img.convertTo(imgBrightnessDiff, -1, 1, pix_diff);

	String window_name_ori  = "Original Image";
	String window_name_diff = "Brightness Increased Image";
	
	namedWindow(window_name_ori);
	namedWindow(window_name_diff);

	imshow(window_name_ori, img);
	imshow(window_name_diff, imgBrightnessDiff);

	waitKey(0);

	destroyAllWindows();

	return 0;
}


int ChangeImgContrast(char *img_name, float contrast_high, float contrast_low)
{
	Mat img = imread(img_name);
	Mat imgH, imgL;

	if (img.empty())
	{
		cout << "Could not open or find the image" << endl;
		cin.get();
		return -1;
	}

	img.convertTo(imgH, -1, contrast_high, 0);
	img.convertTo(imgL, -1, contrast_low, 0);

	String window_name_ori = "Original Image";
	String window_name_hight = "Contrast Hight Image";
	String window_name_low = "Contrast Low Image";

	namedWindow(window_name_ori);
	namedWindow(window_name_hight);
	namedWindow(window_name_low);

	imshow(window_name_ori, img);
	imshow(window_name_hight, imgH);
	imshow(window_name_low, imgL);

	waitKey(0);

	destroyAllWindows();

	return 0;
}


int main()
{
	int ret = 0;

	cout << "Please choose the function you want to run" << endl
		<< "1 : ChangeImgBrightness increase 50" << endl
		<< "2 : ChangeImgBrightness decrease 50" << endl
		<< "3:  ChangeImgContrast 2 0.5" << endl;

	switch (cin.get())
	{
	case '1':
		ret = ChangeImgBrightness("./Eagle.jpg", 50);
		if (ret != 0)
		{
			cout << "ChangeImgBrightness increase 50 Failed" << endl;
		}
		break;
	case '2':
		ret = ChangeImgBrightness("./Eagle.jpg", -50);
		if (ret != 0)
		{
			cout << "ChangeImgBrightness decrease 50 Failed" << endl;
		}
		break;
	case '3':
		ret = ChangeImgContrast("./Eagle.jpg", 2.0, 0.5);
		if (ret != 0)
		{
			cout << "Save Video From Camera to File Failed" << endl;
		}
		break;
	default:
		break;
	}

	return ret;
}

