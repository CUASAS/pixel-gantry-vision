// gVision_tests.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "opencv2/opencv.hpp"
#include "../gVision/utils.h"
#include "../gVision/gVision.h"

using namespace std;
void test_find_patches() {
//    cv::Mat img = cv::imread("../../gVisionLV/Sample Images/HDI_fid.png");
    cv::Mat img = cv::imread("C:\\Users\\cfang\\Desktop\\nebraska-silicon-lab\\pixel-gantry-vision\\gVisionLV\\Sample Images\\HDI_fid.png", 
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
        true,
        "",
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


int main()
{
    set_debug(true);
    test_find_patches();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
