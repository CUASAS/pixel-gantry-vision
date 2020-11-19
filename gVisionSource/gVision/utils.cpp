#include "stdafx.h"
#include "utils.h"
#include <sstream>
#include <fstream>


std::string log_filedir = "";

void set_log_filedir(std::string& new_log_filedir) {
    log_filedir = new_log_filedir;
}

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
    if (!log_filedir.empty()) {
        std::ofstream myfile;
        std::string filename;
        filename.append(log_filedir);
        filename.append("\\");
        filename.append(GVISION_LOG_FILENAME);
        myfile.open(filename, std::ios::out | std::ios::app);

        myfile << data << std::endl;
        myfile.close();
    }
}
void log(std::stringstream &data){
	log(data.str());
	data.str("");
}