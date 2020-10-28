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

vector<cv::Vec3f> get_circles(cv::Mat& img, int min_radius, int max_radius, int circle_detection_sensitivity, int CDparam_1, int CDblue, int CDgreen, int CDred, int line_thickness)
{
	cv::GaussianBlur(img, img, cv::Size(5, 5), 0);
	vector<cv::Vec3f> circle_vector;     
	cv::HoughCircles(img, circle_vector, cv::HOUGH_GRADIENT, 1, img.rows / 16, CDparam_1, circle_detection_sensitivity, min_radius, max_radius); //paramaters wierd??
	//Hough Cirlces Paramaters
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

param1	first method-specific parameter. In case of HOUGH_GRADIENT , it is the higher threshold of the two passed to the Canny edge detector (the lower one is twice smaller).

param2	second method-specific parameter. In case of HOUGH_GRADIENT , 
it is the accumulator threshold for the circle centers at the detection stage. 
The smaller it is, the more false circles may be detected. 
Circles, corresponding to the larger accumulator values, will be returned first.
*/
	if (!circle_vector.empty()) //otherwise draw circle
	{
		for (size_t i = 0; i < circle_vector.size(); i++)
		{
			cv::Vec3i c = circle_vector[i];
			cv::Point center = cv::Point(c[0], c[1]);
			// circle center
			cv::circle(img, center, 1, cv::Scalar(CDblue, CDgreen, CDred), 1, cv::LINE_AA);
			//img, center, radius, color(R,G,B), line thickness
			// circle outline
			int radius = c[2];
			circle(img, center, radius, cv::Scalar(CDblue, CDgreen, CDred), line_thickness, cv::LINE_AA);

		}
	}
	return circle_vector;
}
//it changes the mat so u dont have to return the image

//Point paramaters
/*(InputOutputArray 	img,
	Point 	center,
	int 	radius,
	const Scalar& color,
	int 	thickness = 1,
	int 	lineType = LINE_8,
	int 	shift = 0
	)
	*/


void error_circle(cv::Mat& img)
{
	//get width of picture(scaled)
	//set x_fov and error bound to be whatever you want
	//PARAMATERIZE
}


__declspec(dllexport) int __cdecl helloworld(char* imgPtr, int imgLineWidth, int imgWidth, int imgHeight, 
					double percent_size, int min_radius, int max_radius, int circle_detection_sensitivity, int CDparam_1, int CDblue, int CDgreen, int CDred, int line_thickness)
{
	cv::Mat img(imgHeight, imgWidth, CV_8U, (void*)imgPtr, imgLineWidth);
	
	cv::resize(img, img, cv::Size(img.cols * percent_size, img.rows * percent_size), 0, 0);

	get_circles(img, min_radius, max_radius, circle_detection_sensitivity, CDparam_1,CDblue, CDgreen, CDred, line_thickness);

	//error python outline? 
	/*
	error_bound = 10 # 10 micrometers
	 #we got width/length of picture in pixels earlier
	 x_fov = 2000 # 2mm or 2000 micrometers
	 #or if FOV is area then np.sqrt(length_field)
	 pixel_length = x_fov/xpixel #this tells you in x_fov there are this many xpixels
	 r_error = error_bound / pixel_length #error / pixel length gets you the "radius of error"
	 error_circle = cv2.circle(image, (x,y), int(r_error), 255, 1)*/

	show2(img);
	return 0;
}

//c++ code changes
//compile code (build solution)
//copy the dll's, C:\Users\hayde\pixel-gantry-vision\gVisionSource\x64\Release,just gvision.dll (c++ side)
//then go C:\Users\hayde\pixel-gantry-vision\gVisionLV\gVisionBin paste that dll from above into this one and delete old gvision.dll
//now go to labview, open helloworld.vi, right click on helloworld function, configure it, set paramaters so they are the
//same as the c++ one
//run helloworld.vi to test it then open visionlog for text, for images