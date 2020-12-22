#pragma once


extern "C" __declspec(dllexport) int __cdecl calc_focus(
	char* img, 
	int imgLineWidth,
	int imgWidth, 
	int imgHeight,
	float* focus,
	char* log_filedir);

extern "C" __declspec(dllexport) int __cdecl find_patches(
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
    float* patchSizes);



extern "C" __declspec(dllexport) int __cdecl find_circles(
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
    float* circleRadii);
