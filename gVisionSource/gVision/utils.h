#include <vector>
#include <string>
#include <sstream>

#define GVISION_LOG_FILE "C:/Users/cfang/Desktop/gVisionlog.txt"
#define GVISION_ENABLE_LOGGING

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

void log(std::string &data);
void log(std::stringstream &data);