#pragma once

extern "C" __declspec(dllexport) int __cdecl helloworld(char* imgPtr, int imgLineWidth, int imgWidth, int imgHeight,
	double percent_size, int min_radius, int max_radius, int circle_detection_sensitivity, int CDparam_1, int CDblue, int CDgreen, int CDred, int line_thickness);


//if ur func has arguments pass those too