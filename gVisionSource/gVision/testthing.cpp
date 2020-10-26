
#include "stdafx.h"
#include "utils.h"
#include "testthing.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>

using namespace std;

void show2(cv::Mat img) 
{
	cv::namedWindow("MyWindow", cv::WINDOW_AUTOSIZE);
	cv::imshow("MyWindow", img);
	cv::waitKey(0);
	cv::destroyWindow("MyWindow");
}

vector<cv::Vec3f> get_circles(cv::Mat& img, int min_radius, int max_radius)
{

	cv::GaussianBlur(img, img, cv::Size(5, 5), 0);
	vector<cv::Vec3f> circle_vector;     //idk if this works hope so, also might be bad code
	cv::HoughCircles(img, circle_vector, cv::HOUGH_GRADIENT, 1, img.rows / 16, 100, 30, min_radius, max_radius); //paramaters wierd??
	/*
cv::HoughCircles(img, circles, cv::HOUGH_GRADIENT, 1, img.rows / 16, 100, 30, 1, 30);
gray: Input image(grayscale).
circles : A vector that stores sets of 3 values : xc, yc, r for each detected circle.
HOUGH_GRADIENT : Define the detection method.Currently this is the only one available in OpenCV.
dp = 1 : The inverse ratio of resolution.
min_dist = gray.rows / 16 : Minimum distance between detected centers.
param_1 = 200 : Upper threshold for the internal Canny edge detector.
param_2 = 100 * : Threshold for center detection.
min_radius = 0 : Minimum radius to be detected.If unknown, put zero as default.
max_radius = 0 : Maximum radius to be detected.If unknown, put zero as default.
*/
	if (!circle_vector.empty()) //otherwise draw circle
	{
		for (size_t i = 0; i < circle_vector.size(); i++)
		{
			cv::Vec3i c = circle_vector[i];
			cv::Point center = cv::Point(c[0], c[1]);
			// circle center
			cv::circle(img, center, 1, cv::Scalar(0, 100, 100), 3, cv::LINE_AA);
			// circle outline
			int radius = c[2];
			circle(img, center, radius, cv::Scalar(255, 0, 255), 3, cv::LINE_AA);
		}
	}
	return circle_vector;
}
//it changes the mat so u dont have to return 

__declspec(dllexport) int __cdecl helloworld(char* imgPtr, int imgLineWidth, int imgWidth, int imgHeight, int min_radius, int max_radius)
{
	double percent_size = .4;
	cv::Mat img(imgHeight, imgWidth, CV_8U, (void*)imgPtr, imgLineWidth);
	cv::resize(img, img, cv::Size(img.cols * percent_size, img.rows * percent_size), 0, 0);

	get_circles(img, min_radius, max_radius);

	//Houghcirlces
	//use houghcircle info to draw houghcircles
	//return that info (circle,x,y)

	show2(img);
	return 0;
}

/*
cv::HoughCircles(img, circles, cv::HOUGH_GRADIENT, 1, img.rows / 16, 100, 30, 1, 30);
gray: Input image(grayscale).
circles : A vector that stores sets of 3 values : xc, yc, r for each detected circle.
HOUGH_GRADIENT : Define the detection method.Currently this is the only one available in OpenCV.
dp = 1 : The inverse ratio of resolution.
min_dist = gray.rows / 16 : Minimum distance between detected centers.
param_1 = 200 : Upper threshold for the internal Canny edge detector.
param_2 = 100 * : Threshold for center detection.
min_radius = 0 : Minimum radius to be detected.If unknown, put zero as default.
max_radius = 0 : Maximum radius to be detected.If unknown, put zero as default.
*/

//c++ code changes
//compile code (build solution)
//copy the dll's, C:\Users\hayde\pixel-gantry-vision\gVisionSource\x64\Release,just gvision.dll (c++ side)
//then go C:\Users\hayde\pixel-gantry-vision\gVisionLV\gVisionBin paste that dll from above into this one and delete old gvision.dll
//now go to labview, open helloworld.vi, right click on helloworld function, configure it, set paramaters so they are the
//same as the c++ one
//run helloworld.vi to test it then open visionlog for text, for images