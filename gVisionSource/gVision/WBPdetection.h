#pragma once

extern "C" __declspec(dllexport) int __cdecl WBPdetection(char* imgPtr, int imgLineWidth, int imgWidth, int imgHeight, double percent_size, double sizeMin, double sizeMax);
