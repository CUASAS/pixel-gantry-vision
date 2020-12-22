#include "stdafx.h"
#include "utils.h"
#include "testthing.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

using namespace std;

void show3(cv::Mat img)
{
	cv::namedWindow("MyWindow", cv::WINDOW_AUTOSIZE);
	cv::imshow("MyWindow", img);
	cv::waitKey(0);
	cv::destroyWindow("MyWindow");
}

/*
void size_filter(cv::Mat img, std::vector<std::vector<cv::Point>> contours, double sizeMin, double sizeMax)
{
	int pixels = img.cols * img.rows;
	for (int i = 0; i < contours.size(); i++)
	{
		if(cv::contourArea(contours[i])/pixels < sizeMin)
		{
			contours.erase(contours.begin() + i);
		}
	}
}//return the contours between sizeMin and sizeMax
//instead of eraseing only draw contours with that sizes, erasing is better bec you can use multiple filters easily
*/

__declspec(dllexport) int __cdecl WBPdetection(char* imgPtr, int imgLineWidth, int imgWidth, int imgHeight, double percent_size, double sizeMin, double sizeMax)
{
	cv::Mat img(imgHeight, imgWidth, CV_8U, (void*)imgPtr, imgLineWidth);
	cv::resize(img, img, cv::Size(img.cols * percent_size, img.rows * percent_size), 0, 0);
	// if you dont want img to get overwritten -> cv::Mat img_clone = image.clone();

	cv::threshold(img, img, 125, 255, cv::THRESH_OTSU);
	cv::GaussianBlur(img, img, cv::Size(5, 5), 0);
	cv::erode(img, img, cv::Mat(), cv::Point(-1, -1), 2, 1, 1); //idk what Point does
	cv::dilate(img, img, cv::Mat(), cv::Point(-1, -1), 1, 1, 1); //after point is number of iterations
	//cv::Point is a class to specify x,y postion, it acts as an anchor point, (-1,-1) means it as the center for erode/dilate
	//Parameters(input img, output img, kernel (structing element used for dilation), point, number of iterations
	//,pixel extrapolation method, border value in case of a constant border (this might mean width of line)
	//border value goes from 0-16 each mean a different thing for example 0 is constant and 1 is replicate
	//it keeps the border value for 1 so it can replicate the color it erodes or you can do 

	vector<vector<cv::Point>> contours; //contours is this double vector where the whole thing is a vector and in that vector are smaller point vectors
	//study what cv::Point means
	//comment line structure of double vector
	//vectors are containers representing arrays that can change size, so it could grow in size as you add elements
	//so this is a vector of a vector of points which are xy values representing where the contours are. Each element is a different contour


	vector<cv::Vec4i> hierarchy; //I believe this is a place holder, except RETR_Tree creates heirarchy
	cv::findContours(img, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE); //RETR_List says forget about the hierarchy, tree makes heirarchy
	//Finds all contours by algorithim and makes them this double vector

	//size_filter(img, contours, sizeMin, sizeMax); //size filter but doesnt work

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
  /// Draw polygonal contour + bonding rects
	int pixels = img.cols * img.rows;
	for( int i = 0; i< contours.size(); i++ )
     {
       cv::Scalar color = cv::Scalar(255,255,255); //creates color
	   if (cv::contourArea(contours_poly[i])/pixels > sizeMin && cv::contourArea(contours_poly[i])/pixels < sizeMax) //if statement for size filter
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
	   }
	}
 //filter this by a given size of
	//be able to output the center and then be able to choose the corners in labview
	//rect center, heights and widths
	//center is distance in mm from center of image
	//rectangle width in mm and height in mm
	//also return number of rectangles found and be able to select in labview
	show3(img);
	return 0;
}