#include "stdafx.h"
#include "utils.h"
#include "WBPdetection.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>

using namespace std;

__declspec(dllexport) int __cdecl WBPdetection(
	char* imgPtr, 
	int imgLineWidth, 
	int imgWidth, 
	int imgHeight, 
	double percent_size, 
	double nominalWidth, 
	double nominalHeight,
	double tolerance,
	double xfov,
	double yfov,
	int* Nrects,
	float* rectCenterX,
	float* rectCenterY,
	float* rectWidths, 
	float* rectHeights,
	float* rectCenterXmm,
	float* rectCenterYmm
	)
{
	set_log_filedir(string("C:\\Users\\hayde\\OneDrive\\Desktop")); //allows user to print things to a file
	set_debug(true);
	std::stringstream ss;
	//ss << centerRects[i].x << ", " << centerRects[i].y << endl;
	//log(ss); //example logging

	cv::Mat img(imgHeight, imgWidth, CV_8U, (void*)imgPtr, imgLineWidth);
	cv::resize(img, img, cv::Size(img.cols * percent_size, img.rows * percent_size), 0, 0);

	//PREPPING IMAGE FOR DETECTION ALGORITHIM
	cv::threshold(img, img, 125, 255, cv::THRESH_OTSU);
	cv::GaussianBlur(img, img, cv::Size(5, 5), 0);
	cv::erode(img, img, cv::Mat(), cv::Point(-1, -1), 2, 1, 1); //cv::Point class to specify xy position, (-1,-1) means it is at the center
	cv::dilate(img, img, cv::Mat(), cv::Point(-1, -1), 1, 1, 1); 
	//erodeParameters(input img, output img, kernel (structing element used for dilation), point, number of iterations ,pixel extrapolation method, border value
	
	//USE FIND CONTOURS ALGORITHIM
	vector<vector<cv::Point>> contours; 
	vector<cv::Vec4i> hierarchy; 

	cv::findContours(img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	//APPROXIMATE CONTOURS TO POLYGONS AND MAKE BOUNDING RECTANGLE
	vector<vector<cv::Point> > contours_poly( contours.size() ); 
	vector<cv::Rect> boundRect( contours.size() ); //Rect is a class for rectangles, described by the coordinates of the top-left, bottom-right, width and height
	
	for( int i = 0; i < contours.size(); i++ ) 
	{	approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true ); //simplifies contours by decreasing the number of vertices, douglas-peucker algorithim
		//Parameters(input vector of a 2D point so like contours[i] stored in a vector or mat, result of the approximate curve, approximation accuracy, true for closed shape
		boundRect[i] = cv::boundingRect( cv::Mat(contours_poly[i]) ); //creates a bounding rectangle out of the approximated polynomial contours
     }

	vector<vector<double>> dimRects(contours.size()); //ex [ [w1,h1], [w2,h2], [w3,h3], ...]
	vector<cv::Point> centerRects; //ex [ [c1], [c2], [c3], ... ] ; these are center xy coordinates

	//PUTTING DIMENSIONS OF ALL RECTANGLES IN VECTORS
	for (int i = 0; i < contours.size(); i++)
	{
		cv::Point center;
		dimRects[i].push_back(boundRect[i].width * (xfov / img.cols));
		dimRects[i].push_back(boundRect[i].height * (yfov / img.rows));
		centerRects.push_back(cv::Point(ceil((boundRect[i].tl().x + boundRect[i].br().x) / 2), ceil((boundRect[i].tl().y + boundRect[i].br().y) / 2))); //x,y avg for center
		
	}

	vector<cv::Point> copy_centerRects; //have to create copy for drawing centers of rectangles
	copy_centerRects = centerRects;
	std::sort(copy_centerRects.begin(), copy_centerRects.end(), [](const cv::Point& a, const cv::Point& b) 
		{
			return (a.y < b.y);
		});


	//DEFINING minWidth, etc... FROM tolerance AND nominalWidth
	double minWidth = nominalWidth * (1 - tolerance);
	double maxWidth = nominalWidth * (1 + tolerance);
	double minHeight = nominalHeight * (1 - tolerance);
	double maxHeight = nominalHeight * (1 + tolerance);

  // DRAWING CONTOURS AND BOUNDING RECTANGLE + CENTER

	int counter = 0; //counts number of times it goes into if statement, which is number of rectangles
	for( int i = 0; i< contours.size(); i++ )
     {
       cv::Scalar color = cv::Scalar(255,255,255); //creates color
	   if ((dimRects[i][0] > minWidth && dimRects[i][0] < maxWidth) && (dimRects[i][1] > minHeight && dimRects[i][1] < maxHeight)) 
	   {
		   counter += 1;
		   drawContours(img, contours_poly, i, color, 1, 8, vector<cv::Vec4i>(), 0, cv::Point()); //takes the approximated contours as polynomails
		   rectangle(img, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0); //draws the rectangle fromthe boundRect vector
		   circle(img, centerRects[i], 1, cv::Scalar(255, 255, 255), 3, cv::LINE_8);
		   //array for labview
		   *(rectCenterX + counter) = copy_centerRects[i].x; //adds counter to the memory location for array in labview
		   *(rectCenterY + counter) = copy_centerRects[i].y;
		   *(rectWidths + counter) = boundRect[i].width;
		   *(rectHeights + counter) = boundRect[i].height;
		   *(rectCenterXmm + counter) = (copy_centerRects[i].x - img.cols/2)*(xfov / img.cols); 
		   *(rectCenterYmm + counter) = (copy_centerRects[i].y - img.rows/2)*(xfov / img.cols);
		   ss << centerRects[i].x << ", " << centerRects[i].y << endl;
		   log(ss); //example logging
		   if (counter >= 50) //for dimension size in labview, its 50
		   {
			   break;
		   }
	  }
	}
	*Nrects = counter;

	//be able to choose desirable rectangle in labview (could try to number them? idk see if easier in Labview) or choose corners

	show(img);
	return 0;
}