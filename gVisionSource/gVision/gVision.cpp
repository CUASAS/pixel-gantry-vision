/* gVision.cpp
 * Routines directly related to the vision system on the gantry including
 *   - Fiducial recognition/location
 *   - Focus calculation
 *   - Focus curve fitting
 */

#include "stdafx.h"
#include "utils.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <cmath>
#include <limits>

extern "C" __declspec(dllexport) int __cdecl calc_focus(char* img,   int imgLineWidth,
                                                      int imgWidth, int imgHeight,
                                                      int interactive,
                                                      float* focus);
extern "C" __declspec(dllexport) int __cdecl find_fiducial(char* img, int imgLineWidth,
                                                           int imgWidth, int imgHeight,
                                                           float shrinkFactor,
                                                           int dilateSize,
                                                           float sizeMin, float sizeMax,
                                                           float arMin, float arMax,
                                                           int colorGroups,
                                                           int interactive,
                                                           int* numFiducials,
                                                           float* coords);


extern "C" __declspec(dllexport) void __cdecl fit_focus(unsigned int num_measurements,
                                                        double* focus_values,
                                                        double* heights,
                                                        double* mean,
                                                        double* stdev);


void show(cv::Mat img, int interactive){
    if (interactive==0) return;
    cv::namedWindow("MyWindow", cv::WINDOW_AUTOSIZE);
    cv::imshow("MyWindow", img);
    cv::waitKey(0);
    cv::destroyWindow("MyWindow");
}

//#define INTERACTIVE

__declspec(dllexport) int __cdecl get_focus(char* imgPtr, int imgLineWidth,
                                            int imgWidth, int imgHeight,
                                            int interactive,
                                            float* focus){
    cv::Mat img(imgHeight, imgWidth, CV_8U, (void*)imgPtr, imgLineWidth);
    show(img, interactive);

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

std::vector<std::vector<cv::Point>> get_contours(cv::Mat &img, float sizeMin, float sizeMax, float arMin, float arMax){
    float pixels = (float)img.rows * img.cols;
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    findContours(img.clone(), contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point>> passContours;
    for (unsigned int i = 0; i < contours.size(); i++){
        float size = (float)(contourArea(contours[i]) / pixels);
        cv::RotatedRect rr = minAreaRect(contours[i]);
        float ar = float(rr.size.width) / rr.size.height;
        if ((size > sizeMin && size < sizeMax) && (ar > arMin && ar < arMax)){
            passContours.push_back(contours[i]);
        }
    }
    return passContours;
}

#define NUM_FIDS 10
__declspec(dllexport)
int __cdecl get_fiducial(char* imgPtr, int imgLineWidth,
                         int imgWidth, int imgHeight,
                         float shrinkFactor,
                         int dilateSize,
                         float sizeMin, float sizeMax,
                         float arMin, float arMax,
                         int colorGroups,
                         int interactive,
                         int* numFiducials,
                         float* coords){ //coords must be a 2*NUM_FIDS element float array
    cv::Mat imgIn(imgHeight, imgWidth, CV_8U, (void*)imgPtr, imgLineWidth);
    cv::Mat img = imgIn.clone(); //Make a local copy of image to avoid corrupting original image
    int rows = (int)(img.rows / shrinkFactor);
    int cols = (int)(img.cols / shrinkFactor);
    cv::Size s(cols, rows);
    show(img, interactive);
    resize(img, img, s);
    //show(img, interactive);
    //doBlur(img, dilateSize);
    show(img, interactive);
    do_kmeans(img, colorGroups);

    do_dilate(img, dilateSize);
    show(img, interactive);

    std::vector<std::vector<cv::Point>> contours = get_contours(img, sizeMin, sizeMax, arMin, arMax);
    if (contours.size() > NUM_FIDS){
        contours.resize(NUM_FIDS);
    }
    *numFiducials = contours.size();


    std::stringstream ss;
    ss << "Fiducials Found: " << contours.size() << std::endl;
    log(ss.str());

    for (unsigned int i = 0; i < contours.size(); i++){
        std::vector<cv::Point> fidContour = contours[i];

        cv::Moments mu = moments(fidContour, false);
        //cv::Point2f centroid(mu.m10 / mu.m00, mu.m01 / mu.m00);
        float x = (float)(mu.m10 / mu.m00 / cols);
        float y = (float)(mu.m01 / mu.m00 / rows);
        //circle(img, centroid, 3, cv::Scalar(255), -1);
        *(coords + 2 * i) = x;
        *(coords + 2 * i + 1) = y;

        ss << "x:" << x << ", y:" << y << std::endl;
        log(ss);
    }

    return 0;
}

__declspec(dllexport)
void __cdecl fit_focus(unsigned int num_measurements,
                       double* focus_values,
                       double* heights,
                       double* mean,
                       double* stdev){
    std::stringstream ss;
    ss << "Fitting Focus of " << num_measurements << " datapoints\n";
    ss << "height\tfocus\n";
    for(unsigned int i=0; i<num_measurements; i++){
        ss << heights[i] << "\t" << focus_values[i] << std::endl;
    }
    log(ss);
    if (num_measurements==0){
            *mean  = std::numeric_limits<double>::quiet_NaN();
            *stdev = std::numeric_limits<double>::quiet_NaN();
            return;
    }
    if (num_measurements==1){
            *mean  = heights[0];
            *stdev = 0;
            return;
    }
    std::vector<double> means;
    std::vector<double> vals_adj;

    double sum = 0;
    for(unsigned int i=0; i<num_measurements; i++){
        vals_adj.push_back(log(focus_values[i]));
        sum += vals_adj[i];
    }
    //Calculate leave-one-out means
    for(unsigned int j=0; j<num_measurements; j++){
        means.push_back(0);
        for(unsigned int i=0; i<num_measurements; i++){
            if(i!=j) means[j] += heights[i]*vals_adj[i];
        }
        means[j] = means[j] / (sum-vals_adj[j]);
    }

    //Calculate mean and variance of means
    double sumSquares =0;
    sum = 0;
    for(unsigned int i=0; i< num_measurements; i++){
        sumSquares += means[i]*means[i];
        sum += means[i];
    }
    *mean  = sum/num_measurements;
    *stdev = sqrt(sumSquares - sum*sum)/num_measurements; 
    ss << "\tfound mean  =" << *mean << std::endl;
    ss << "\tfound stdev =" << *stdev << std::endl;
    log(ss);
}
