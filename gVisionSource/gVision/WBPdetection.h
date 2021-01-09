#pragma once

extern "C" __declspec(dllexport) int __cdecl WBPdetection(char* imgPtr,
	int imgLineWidth,
	int imgWidth,
	int imgHeight,
	double percent_size,
	double nominalWidth,
	double nominalHeight,
	double tolerance,
	double xfov,
	double yfov);