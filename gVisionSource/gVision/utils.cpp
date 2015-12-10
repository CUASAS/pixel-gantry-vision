#include "stdafx.h"
#include "utils.h"
#include <sstream>
#include <fstream>
std::vector<std::string> &split(const std::string &s, char delim,
    std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    std::vector<std::string> tokens;
    for(unsigned int i=0; i<elems.size(); i++){
        if(elems[i].size() > 0){
            tokens.push_back(elems[i]);
        }
    }
    return tokens;
}


void log(std::string &data){
#ifdef GVISION_ENABLE_LOGGING
    std::ofstream myfile;
    myfile.open(GVISION_LOG_FILE, std::ios::out | std::ios::app);

    myfile << data << std::endl;
    myfile.close();
#endif
}
void log(std::stringstream &data){
	log(data.str());
	data.str("");
}