#include "stdafx.h"
#include "utils.h"
#include "WBPdetection.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <vector>

using namespace std;

void show3(cv::Mat img)
{
	cv::namedWindow("MyWindow", cv::WINDOW_AUTOSIZE);
	cv::imshow("MyWindow", img);
	cv::waitKey(0);
	cv::destroyWindow("MyWindow");
}

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
	double yfov)
{
	cv::Mat img(imgHeight, imgWidth, CV_8U, (void*)imgPtr, imgLineWidth);
	cv::resize(img, img, cv::Size(img.cols * percent_size, img.rows * percent_size), 0, 0);
	// if you dont want img to get overwritten -> cv::Mat img_clone = image.clone();

	//PREPPING IMAGE FOR DETECTION ALGORITHIM
	cv::threshold(img, img, 125, 255, cv::THRESH_OTSU);
	cv::GaussianBlur(img, img, cv::Size(5, 5), 0);
	cv::erode(img, img, cv::Mat(), cv::Point(-1, -1), 2, 1, 1); //idk what Point does
	cv::dilate(img, img, cv::Mat(), cv::Point(-1, -1), 1, 1, 1); //after point is number of iterations
	//cv::Point is a class to specify x,y postion, it acts as an anchor point, (-1,-1) means it as the center for erode/dilate
	//Parameters(input img, output img, kernel (structing element used for dilation), point, number of iterations
	//,pixel extrapolation method, border value in case of a constant border (this might mean width of line)
	//border value goes from 0-16 each mean a different thing for example 0 is constant and 1 is replicate
	//it keeps the border value for 1 so it can replicate the color it erodes or you can do 
	
	//USE FIND CONTOURS ALGORITHIM
	vector<vector<cv::Point>> contours; //contours is this double vector where the whole thing is a vector and in that vector are smaller point vectors
	//study what cv::Point means
	//comment line structure of double vector
	//vectors are containers representing arrays that can change size, so it could grow in size as you add elements
	//so this is a vector of a vector of points which are xy values representing where the contours are. Each element is a different contour
	vector<cv::Vec4i> hierarchy; //I believe this is a place holder, except RETR_Tree creates heirarchy
	cv::findContours(img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE); //RETR_List says forget about the hierarchy, tree makes heirarchy
	//Finds all contours by algorithim and makes them this double vector

	// could prob combine most of these loops for efficiency but would lose some readability could test how much more efficient
	//APPROXIMATE CONTOURS TO POLYGONS AND MAKE BOUNDING RECTANGLE
	vector<vector<cv::Point> > contours_poly( contours.size() ); //creats another double vector but for approximate polynomial
	//its called contours_poly, not sure what the parenthesise are doing
	vector<cv::Rect> boundRect( contours.size() ); //same idea for boundRect as for contours_poly
	//comment line for why it is like boundRect( contours.size() ) with the parenthesis
	//Rect is a class for rectangles, described by the coordinates of the top-left and the width and height
	//so i think its a vector/array of class objects that are rectangles
	for( int i = 0; i < contours.size(); i++ ) //loops through the whole contour vector
	{	approxPolyDP( cv::Mat(contours[i]), contours_poly[i], 3, true ); //for each vector in the contour vector it approximates -> next line
		//it as a polynomial and then has a contour vector that is assigned to contours_poly[i] corresponding to each contours[i]
	//comment line for documentation on approxPolyDP
	//Mat represents an n-dim numerical array and can be used to store images
	//approxPolyDP approximates a curve or a polygon with another curve/polygon such that theres less vertices or the distance is smaller
	//uses the douglas-peucker algorithim
	//Parameters(input vecotr of a 2D point so like contours[i] stored ina vector or mat, result of the approximate curve,
	//epsilon which specifys the approximation accuricy, last is if it is true the the approximation is closed
		boundRect[i] = cv::boundingRect( cv::Mat(contours_poly[i]) ); //creates a bounding rectangle out of the approximated polynomial contours
		//how is it a cv::Mat?? Maybe that is what a mat is?
		//commentline for bounding rect which is not min area rect so it doesnt consider rotation
		//STILL DONT KNOW FOR THIS ONE
		//
     }

	vector<vector<double>> dimRects; //ex [ [w1,h1], [w2,h2], [w3,h3], ...]
	vector<cv::Point> centerRects; //ex [ [c1], [c2], [c3], ... ]
	//these are center xy coordinates

	//PUTTING DIMENSIONS OF ALL RECTANGLES IN VECTORS
	for (int i = 0; i < contours.size(); i++)
	{
		cv::Point center = ((boundRect[i].tl().x + boundRect[i].br().x) / 2, (boundRect[i].tl().y + boundRect[i].br().y) / 2); //what about even pixels
		double rectWidth = (boundRect[i].br().x - boundRect[i].tl().x) * (xfov / img.cols); //might not matter tbh
		double rectHeight = (boundRect[i].tl().y - boundRect[i].br().y) * (yfov / img.rows);
		dimRects[i].push_back(rectWidth);
		dimRects[i].push_back(rectHeight);
		centerRects.push_back(center);
	}

	//DEFINING minWidth, etc... FROM tolerance AND nominalWidth
	double minWidth = nominalWidth * (1 - tolerance);
	double maxWidth = nominalWidth * (1 + tolerance);
	double minHeight = nominalHeight * (1 - tolerance);
	double maxHeight = nominalHeight * (1 + tolerance);

  // DRAWING CONTOURS AND BOUNDING RECTANGLE + CENTER
	for( int i = 0; i< contours.size(); i++ )
     {
       cv::Scalar color = cv::Scalar(255,255,255); //creates color
	   if (dimRects[i][0] > minWidth && dimRects[i][0] < maxWidth && dimRects[i][1] > minHeight && dimRects[i][1] < maxHeight) 
	   {
		   drawContours(img, contours_poly, i, color, 1, 8, vector<cv::Vec4i>(), 0, cv::Point()); //takes the approximated contours as polynomails
		   // (from the approxPolyDP) dont know what anything past i is doing except for color
		   //drawContours documaentaion
		   //Paramaters(image, all input contours (each is stored as a vector),contourIdz whcih indicates what contour to draw, color, 
		   //thickness, line type which is line connectivity, heiarchy stuff which is optional and only used if you want to draw some of the contours,
		   //maximal level for contours and offset are the last ones but dont seem super important and are niche)
		   rectangle(img, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0); //draws the rectangle fromthe boundRect vector
		   //found the boundingRect using cv::boundingRect algorithim, .tl and .br are functions used to pic out tl and br as points to draw the rectangle
		//rectangle documentation line
		//
		   circle(img, centerRects[i], 1, cv::Scalar(0, 0, 255), 1, cv::LINE_8);
	   }
	}

	
	//make arguments as similiar as possible to find circles except for rectangle parts
	//find center mm relative to the center of image
	//labview show an array for the rectangle found
	//array for the widths and heights and centers(x,y)
	//choose rectangles so number them in labview prob better



 //filter this by a given size of
	//be able to output the center and then be able to choose the corners in labview
	//rect center, heights and widths
	//center is distance in mm from center of image
	//rectangle width in mm and height in mm
	//also return number of rectangles found and be able to select in labview
	show3(img);
	return 0;
}