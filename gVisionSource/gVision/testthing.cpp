#include "stdafx.h"
#include "utils.h"
#include "testthing.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"


using namespace std;

void show(cv::Mat img) 
{
	cv::namedWindow("MyWindow", cv::WINDOW_AUTOSIZE);
	cv::imshow("MyWindow", img);
	cv::waitKey(0);
	cv::destroyWindow("MyWindow");
}

__declspec(dllexport) int __cdecl helloworld(char* imgPtr, int imgLineWidth, int imgWidth, int imgHeight)
{
	double percent_size = .4;
	cv::Mat img(imgHeight, imgWidth, CV_8U, (void*)imgPtr, imgLineWidth);
	cv::resize(img, img, cv::Size(img.cols * percent_size, img.rows * percent_size), 0, 0);
	
	//put code to do to resized here using void functions? hopefully if not declspec everything and create IV for each :(
	//blur
	//Houghcirlces
	//use houghcircle info to draw houghcircles
	//return that info (circle,x,y)
	
	show(img);
}


//get used to modifying function in c++ code and knowing how to modify the labview set up accordingly

//c++ code changes
//compile code (build solution)
//copy the dll's,C:\Users\hayde\pixel-gantry-vision\gVisionSource\x64\Release,just gvision.dll (c++ side)
//then go C:\Users\hayde\pixel-gantry-vision\gVisionLV\gVisionBin paste that dll from above into this one and delete old gvision.dll
//now go to labview, open helloworld.vi, right click on helloworld function, configure it, set paramaters so they are the
//same as the c++ one
//run helloworld.vi to test it then open visionlog for text, for images