#include <vector>
#include <string>
#include <sstream>
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#define GVISION_LOG_FILENAME "gVisionlog.txt"

void set_debug(bool new_debug);

void show(cv::Mat img);

void set_log_filedir(std::string& new_log_filedir);

void log(std::string &data);
void log(std::stringstream &data);