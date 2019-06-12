#pragma once


extern "C" __declspec(dllexport) int __cdecl calc_focus(char* img, int imgLineWidth,
	int imgWidth, int imgHeight,
	float* focus);

extern "C" __declspec(dllexport) int __cdecl find_fiducial(char* img, int imgLineWidth,
	int imgWidth, int imgHeight,
	float shrinkFactor,
	int dilateSize,
	float sizeMin, float sizeMax,
	float arMin, float arMax,
	int colorGroups,
	int maxAttempts,
	int interactive,
	int* numFiducials,
	float* coords);

// DEPRECIATED
extern "C" __declspec(dllexport) void __cdecl fit_focus(unsigned int num_measurements,
	double* focus_values,
	double* heights,
	double* mean,
	double* stdev);


extern "C" __declspec(dllexport)
int __cdecl get_safe_path(const char* filename,
	const char* start_label,
	const char* end_label,
	int* num_points,
	float* coordinate_list);