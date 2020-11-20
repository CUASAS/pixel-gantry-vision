/* gVision.cpp
 * Routines directly related to the vision system on the gantry including
 *   - Fiducial recognition/location
 *   - Focus calculation
 *   - Focus curve fitting
 */

#include "stdafx.h"
#include "utils.h"
#include "gVision.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <set>
#include <cmath>
#include <limits>

using namespace std;


#define MAX_OBJECTS 20

__declspec(dllexport) int __cdecl calc_focus(char* imgPtr, int imgLineWidth,
                                            int imgWidth, int imgHeight,
                                            float* focus){
    cv::Mat img(imgHeight, imgWidth, CV_8U, (void*)imgPtr, imgLineWidth);

    cv::Mat lap;
    cv::Laplacian(img, lap, CV_32F, 5, .01);

    cv::Scalar mu, sigma;
    cv::meanStdDev(lap, mu, sigma);
    *focus = (float)(sigma.val[0] * sigma.val[0]);
    return 0; 
}

void set_to(cv::Mat &img, char from, char to){
    int n = img.rows * img.cols;
    for (int i = 0; i < n; i++){
        char &curr = img.at<char>(i);
        if (curr == from) curr = to;
    }
}

void set_foreground(cv::Mat &img, char fgId, char fgVal, char bgVal){
    int n = img.rows * img.cols;
    for (int i = 0; i < n; i++){
        char &curr = img.at<char>(i);
        curr = (curr == fgId) ? fgVal : bgVal;
    }
}

void do_blur(cv::Mat &img, int blurSize){
    if (blurSize % 2 == 0) blurSize++;
    cv::blur(img, img, cv::Size(blurSize, blurSize));
}

void do_kmeans(cv::Mat &img, int k){
    cv::TermCriteria tc(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 10, 1.0);
    int flags = cv::KMEANS_PP_CENTERS;
    cv::Mat kmeansIn = img.reshape(1, img.rows*img.cols);
    cv::Mat colVecD, bestLabels, centers, clustered;
    kmeansIn.convertTo(colVecD, CV_32FC3, 1.0 / 255.0);

    double compactness = kmeans(colVecD, k, bestLabels, tc, 1, flags, centers);

    bestLabels = bestLabels.reshape(1, img.rows);
    bestLabels.convertTo(bestLabels, CV_8U);
    img = bestLabels;

	// TODO: Allow for selecting *which* label to use, ie brightest, 2nd brightest, etc
    float maxVal = -1; int foreground = -1;
    for (int i = 0; i < centers.rows; i++){
        float center = centers.at<float>(i);
        if (center > maxVal){
            maxVal = center;
            foreground = i;
        }
    }
    set_foreground(img, foreground, (char)255, (char)0);
}

void do_dilate(cv::Mat &img, int size){
    cv::Size s(size, size);
    cv::Mat element = getStructuringElement(cv::MORPH_ELLIPSE, s);
    dilate(img, img, element);
}

struct ContourData{
	ContourData::ContourData(){}
	ContourData::ContourData(float area, float ar){
		this->area = area;
		this->ar = ar;
	}
	float area;
	float ar;
};

typedef pair<vector<cv::Point>,ContourData> contour_t;

vector<contour_t> get_contours(cv::Mat &img, float sizeMin, float sizeMax, float arMin, float arMax){
    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hierarchy;
    findContours(img.clone(), contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    vector<contour_t> passContours;
	stringstream ss;
    for (unsigned int i = 0; i < contours.size(); i++){
        float area = (float)contourArea(contours[i]);
        cv::RotatedRect rr = minAreaRect(contours[i]);
        float ar = float(rr.size.width) / rr.size.height;
		if(ar<1) ar = 1.0f/ar;
		ss << "area: " << area << ", ar: " << ar << endl << endl;
		log(ss);
        if ((area > sizeMin && area < sizeMax) && (ar > arMin && ar < arMax)){
			contour_t c;
			c.first = contours[i];
			c.second = ContourData(area,ar);
            passContours.push_back(c);
        }
    }
    return passContours;
}

__declspec(dllexport)
int __cdecl find_patches(
    char* imgPtr,
    int imgLineWidth,
    int imgWidth,
    int imgHeight,
    int shrinkFactor, // increase to speed up routine
    float fieldOfViewX,  // mm
    float fieldOfViewY,  // mm
    int dilateSize,
    float sizeMin,  // mm^2
    float sizeMax,  // mm^2
    float aspectRatioMin, 
    float aspectRatioMax,
    int colorGroups,
    bool debug,
    char* logFileDir,
    int* numPatches,
	float* patchXCoordinates,
	float* patchYCoordinates,
	float* patchAspectRatios,
	float* patchSizes)
{
    set_log_filedir(string(logFileDir));
    set_debug(debug);

	std::stringstream ss;
    cv::Mat imgIn(imgHeight, imgWidth, CV_8U, (void*)imgPtr, imgLineWidth);
    cv::Mat img = imgIn.clone(); //Make a local copy of image to avoid corrupting original image
    int rows = (int)(img.rows / shrinkFactor);
    int cols = (int)(img.cols / shrinkFactor);
    cv::Size s(cols, rows);
    resize(img, img, s);

    do_kmeans(img, colorGroups);
    show(img);
    do_dilate(img, dilateSize);
    show(img);

    float pixelSize = (fieldOfViewX * fieldOfViewY) / (cols * rows);
    float sizeMinPx = sizeMin / pixelSize;
    float sizeMaxPx = sizeMax / pixelSize;
    // Note that Aspect Ratio is not corrected to physical size because this code assumes square pixels

    vector<contour_t> contours = get_contours(img, sizeMinPx, sizeMaxPx, aspectRatioMin, aspectRatioMax);
    if (contours.size() > MAX_OBJECTS){
        contours.resize(MAX_OBJECTS);
    }
    *numPatches = contours.size();

    ss << "Fiducials Found: " << contours.size() << endl;
    log(ss.str());

    for (unsigned int i = 0; i < contours.size(); i++){
        vector<cv::Point> fidContour = contours[i].first;
		ContourData c = contours[i].second;

        cv::Moments mu = moments(fidContour, false);
        float x = (mu.m10 / mu.m00) * (fieldOfViewX / cols) - 0.5*fieldOfViewX;
        float y = (mu.m01 / mu.m00) * (fieldOfViewY / rows) - 0.5*fieldOfViewY;
        *(patchXCoordinates + i) = x;
        *(patchYCoordinates + i) = y;
        *(patchAspectRatios + i) = c.ar;
        *(patchSizes + i) = c.area * pixelSize;

        ss << i << ":" << endl;
        ss << "  x:" << x << ", y:" << y << endl;
		ss << "  area: " << pixelSize*c.area << ", ar: " << c.ar << endl << endl;
        log(ss);
    }
    return 0;
}


__declspec(dllexport) int __cdecl find_circles(
	char* imgPtr, 
	int imgLineWidth,
	int imgWidth,
	int imgHeight,
    int shrinkFactor, // increase to speed up routine
    float fieldOfViewX,  // mm
    float fieldOfViewY,  // mm
    float minRadius,  // mm
	float maxRadius,  // mm
	int houghGradientParam1,
	int houghGradientParam2, 
	bool debug,
	char* log_filedir,
    int* numCircles,
	float* circleXCenters,
	float* circleYCenters,
	float* circleRadii)
{
	set_log_filedir(string(log_filedir));
    set_debug(debug);

	cv::Mat imgIn(imgHeight, imgWidth, CV_8U, (void*)imgPtr, imgLineWidth);
    cv::Mat img = imgIn.clone(); //Make a local copy of image to avoid corrupting original image
    int rows = (int)(img.rows / shrinkFactor);
    int cols = (int)(img.cols / shrinkFactor);
    cv::Size s(cols, rows);
    resize(img, img, s);

    // This code assumes square pixels
    float pixelWidth = fieldOfViewX / cols;
    float minRadiusPx = minRadius / pixelWidth;
    float maxRadiusPx = maxRadius / pixelWidth;
	
	cv::GaussianBlur(img, img, cv::Size(5, 5), 0);
	vector<cv::Vec3f> circles;     
	cv::HoughCircles(img, circles, cv::HOUGH_GRADIENT, 1, rows / 16, houghGradientParam1, houghGradientParam2, 
        minRadiusPx, maxRadiusPx);

    *numCircles = circles.size();
    if (circles.size() > MAX_OBJECTS){
        circles.resize(MAX_OBJECTS);
    }
    *numCircles = circles.size();

	for (size_t i = 0; i < circles.size(); i++)
	{
		cv::Vec3i c = circles[i];

		cv::Point center = cv::Point(c[0], c[1]);
		// circle center
		cv::circle(img, center, 1, cv::Scalar(0, 0, 1), 1, cv::LINE_AA);
		// circle outline
		circle(img, center, c[2], cv::Scalar(0, 0, 1), 3, cv::LINE_AA);
	}
	show(img);

	for (int i = 0; i < circles.size(); i++) {
		*(circleXCenters + i) = circles[i][0] * (fieldOfViewX / cols) - 0.5*fieldOfViewX;
		*(circleYCenters + i) = circles[i][1] * (fieldOfViewY / rows) - 0.5*fieldOfViewY;
		*(circleRadii + i) = circles[i][2] * pixelWidth;
	}
	return 0;
}
