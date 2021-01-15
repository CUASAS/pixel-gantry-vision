#pragma once

/*
#define GVISION_LOG_FILENAME "visionlog.txt"

//C:\Users\hayde\OneDrive\Desktop
void set_log_filedir("C:\Users\hayde\OneDrive\Desktop");

void log(std::string& data);
*/

extern "C" __declspec(dllexport) int __cdecl WBPdetection(
	char* imgPtr,
	int imgLineWidth,
	int imgWidth,
	int imgHeight,
	double percent_size,
	double nominalWidth,
	double nominalHeight,
	double tolerance,
	double xfov,
	double yfov);