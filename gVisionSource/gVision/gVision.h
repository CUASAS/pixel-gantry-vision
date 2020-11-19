#pragma once


extern "C" __declspec(dllexport) int __cdecl calc_focus(
	char* img, 
	int imgLineWidth,
	int imgWidth, 
	int imgHeight,
	float* focus,
	char* log_filedir);

extern "C" __declspec(dllexport) int __cdecl find_fiducial(
	char* img, 
	int imgLineWidth,
	int imgWidth,
	int imgHeight,
	float shrinkFactor,
	int dilateSize,
	float sizeMin, 
	float sizeMax,
	float arMin, 
	float arMax,
	int colorGroups,
	int maxAttempts,
	int interactive,
	int* numFiducials,
	float* coords,
	char* log_filedir);

