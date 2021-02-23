// gVision_tests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "opencv2/opencv.hpp"
#include "../gVision/utils.h"
#include "../gVision/gVision.h"

using namespace std;
void test_find_patches(bool debug, const char* logFileDir) {
//    cv::Mat img = cv::imread("../../gVisionLV/Sample Images/HDI_fid.png");
    cv::Mat img = cv::imread("..\\..\\..\\gVisionLV\\Sample Images\\HDI_fid.png", 
        cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        cout << "failed to open image" << endl;
        return;
    }

    int numPatches;
    float patchXCoordinates[50];
    float patchYCoordinates[50];
    float patchAspectRatios[50];
    float patchSizes[50];
    find_patches(
        (char*)img.ptr(),
        img.step,
        img.cols,
        img.rows,
        4,
        1.4,
        1.048,
        5,
        0.27,  // minSize
        0.35,  // maxSize
        0.9,
        1.1,
        2,
        debug,
        logFileDir,
        &numPatches,
        patchXCoordinates,
        patchYCoordinates,
        patchAspectRatios,
        patchSizes
    );
    cout << "Found " << numPatches << " patches." << endl;
    for (int i = 0; i < numPatches; i++) {
        cout << i + 1 << ") " << patchXCoordinates[i] << ", " << patchYCoordinates[i] << endl;
        cout << "    "  << patchAspectRatios[i] << ", " << patchSizes[i] << endl;
    }
}

void test_find_rects(bool debug, const char* logFileDir) {
    cv::Mat img = cv::imread("..\\..\\..\\gVisionLV\\Sample Images\\TFPX_bond_pad.png", 
        cv::IMREAD_GRAYSCALE);
    if (img.empty()) {
        cout << "failed to open image" << endl;
        return;
    }

    int numRects;
    float rectXCenters[50];
    float rectYCenters[50];
    float rectXTLCorners[50];
	float rectYTLCorners[50];
	float rectXTRCorners[50];
	float rectYTRCorners[50];
	float rectXBLCorners[50];
	float rectYBLCorners[50];
	float rectXBRCorners[50];
	float rectYBRCorners[50];
    float rectWidths[50];
    float rectHeights[50];
    float rectAngles[50];

    find_rects(
        (char*)img.ptr(),
        img.step,
        img.cols,
        img.rows,
        1,  // shrinkFactor
        1.400,
        1.048,
        .45,  // nominalWidth
        .28, // nominalHeight
        0.1, // tolerance
        debug,
        logFileDir,
        &numRects,
        rectXCenters, rectYCenters,
	    rectXBLCorners, rectYBLCorners,
        rectXTLCorners, rectYTLCorners,
	    rectXTRCorners, rectYTRCorners,
	    rectXBRCorners, rectYBRCorners,
        rectWidths,
        rectHeights,
        rectAngles
    );
    cout << "Found " << numRects << " Rects." << endl;
    for (int i = 0; i < numRects; i++) {
        cout << i + 1 << ") @" << rectXCenters[i] << ", " << rectYCenters[i] << endl;
        cout << "    width="  << rectWidths[i] << ", height=" << rectHeights[i] << ", angle=" << rectAngles[i] << endl;
    }

}


int main()
{
    //test_find_patches(false);
    test_find_rects(true, "");
}
