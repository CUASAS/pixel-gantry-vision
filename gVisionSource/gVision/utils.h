#include <vector>
#include <string>
#include <sstream>

#define GVISION_LOG_FILE "C:\\Users\\Husker\\Desktop\\out.txt"
//#define GVISION_ENABLE_LOGGING

std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems);
std::vector<std::string> split(const std::string &s, char delim);

void log(std::string &data);
void log(std::stringstream &data);

struct vec3d{
    double x;
    double y;
    double z;
};
struct pry{
    double pitch;
    double roll;
    double yaw;
};