#include "stdafx.h"
#include <iostream>
#include <utility>
#include <sstream>
#include <fstream>
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>
#include <armadillo>
#include "utils.h"

extern "C" __declspec(dllexport) 
void calc_orientation(unsigned int n,
                      double* Kx_in,    double* Ky_in,    double* Kz_in,
                      double* Dx_in,    double* Dy_in,    double* Dz_in,
                      double* offset_x, double* offset_y, double* offset_z,
                      double* pitch,    double* roll,     double* yaw,
					  double* residual, double* orthoganality);

extern "C" __declspec(dllexport)
void to_global_coords(double  local_x,  double  local_y,  double  local_z,
                      double  offset_x, double  offset_y, double  offset_z,
					  double  pitch,    double  roll,     double  yaw,
                      double* global_x, double* global_y, double* global_z);

extern "C" __declspec(dllexport)
void to_local_coords(double  global_x, double  global_y, double  global_z,
                     double  offset_x, double  offset_y, double  offset_z,
					 double  pitch,    double  roll,     double  yaw,
                     double* local_x,  double* local_y,  double* local_z);

extern "C" __declspec(dllexport) 
const char* test_geom_1(const char* fname);

typedef std::pair<vec3d, vec3d> sample;

pry get_pry_from_rot(arma::mat rot){
    pry orientation;
    orientation.yaw = atan2(rot(1,0),rot(0,0));
    orientation.pitch = atan2(-rot(2,0), sqrt(rot(2,1)*rot(2,1)+rot(2,2)*rot(2,2)));
    orientation.roll = atan2(rot(2,1),rot(2,2));
    return orientation;
}


arma::mat build_rot_mat(double pitch, double roll, double yaw){
    double theta = pitch;
    double psi = roll;
    double phi = yaw;

	arma::mat D(3,3, arma::fill::zeros);
	D(0,0) = D(1,1) = cos(phi);
	D(0,1) = sin(phi);
	D(1,0) = -D(0,1);
	D(2,2) = 1;
	arma::mat C(3,3, arma::fill::zeros);
	C(0,0) = C(2,2) = cos(theta);
	C(0,2) = -sin(theta);
	C(2,0) = -C(0,2);
	C(1,1) = 1;
	arma::mat B(3,3, arma::fill::zeros);
	B(1,1) = B(2,2) = cos(psi);
	B(1,2) = sin(psi);
	B(2,1) = -B(1,2);
	B(0,0) = 1;

    return B*C*D;
}

arma::colvec3 rotate(arma::colvec3 vec, double pitch, double roll, double yaw, bool inverse=false){
    arma::mat rot = build_rot_mat(pitch, roll, yaw);
    if(inverse)
        rot = arma::trans(rot);
    return rot*vec;
}

std::vector<sample> parse_data(std::string filename){
    std::vector<std::pair<vec3d,vec3d> > data;
    std::ifstream infile(filename.c_str());
    if (infile.is_open()){
        std::string line;
        while (std::getline(infile, line)){
            std::vector<std::string> tokens = split(line, ',');
            vec3d K;
            K.x = atof(tokens[0].c_str());
            K.y = atof(tokens[1].c_str());
            K.z = atof(tokens[2].c_str());
            vec3d D;
            D.x = atof(tokens[3].c_str());
            D.y = atof(tokens[4].c_str());
            D.z = atof(tokens[5].c_str());
            data.push_back(sample(K,D));
        }
    }
    return data;
}


std::pair<vec3d,pry> calc_orientation(arma::vec Kx, arma::vec Ky, arma::vec Kz,
                                      arma::vec Dx, arma::vec Dy, arma::vec Dz){
    arma::mat M(4,4);
    M(0, 0) =           Dx.size();
    M(0, 1) = M(1, 0) = arma::sum(Kx);
    M(0, 2) = M(2, 0) = arma::sum(Ky);
    M(0, 3) = M(3, 0) = arma::sum(Kz);
    M(1, 1) =           arma::dot(Kx, Kx);
    M(2, 2) =           arma::dot(Ky, Ky);
    M(3, 3) =           arma::dot(Kz, Kz);
    M(1, 2) = M(2, 1) = arma::dot(Kx, Ky);
    M(1, 3) = M(3, 1) = arma::dot(Kx, Kz);
    M(2, 3) = M(3, 2) = arma::dot(Ky, Kz);

    // Build RHS Vector
    arma::colvec D1(4);
    arma::colvec D2(4);
    arma::colvec D3(4);
    D1( 0) = arma::sum(Dx);
    D1( 1) = arma::dot(Kx, Dx);
    D1( 2) = arma::dot(Ky, Dx);
    D1( 3) = arma::dot(Kz, Dx);
    D2( 0) = arma::sum(Dy);
    D2( 1) = arma::dot(Kx, Dy);
    D2( 2) = arma::dot(Ky, Dy);
    D2( 3) = arma::dot(Kz, Dy);
    D3( 0) = arma::sum(Dz);
    D3( 1) = arma::dot(Kx, Dz);
    D3( 2) = arma::dot(Ky, Dz);
    D3( 3) = arma::dot(Kz, Dz);

    // Invert M
    arma::mat Y = arma::inv(M);

    //Multiply D by Y
    arma::colvec result1 = Y*D1;
    arma::colvec result2 = Y*D2;
    arma::colvec result3 = Y*D3;

    arma::mat rot(3,3);
    rot(0, 0) = result1(1); rot(0, 1) = result2(1); rot(0, 2) = result3(1);
    rot(1, 0) = result1(2); rot(1, 1) = result2(2); rot(1, 2) = result3(2);
    rot(2, 0) = result1(3); rot(2, 1) = result2(3); rot(2, 2) = result3(3);

    vec3d offset;
    offset.x = result1(0);
    offset.y = result2(0);
    offset.z = result3(0);
    pry orientation = get_pry_from_rot(rot);
    return std::pair<vec3d,pry>(offset, orientation);
}

__declspec(dllexport) 
void __cdecl calc_orientation(unsigned int n,
                              double* Kx_in, double* Ky_in, double* Kz_in,
                              double* Dx_in, double* Dy_in, double* Dz_in,
                              double* offset_x, double* offset_y, double* offset_z,
                              double* pitch,    double* roll,     double* yaw,
					          double* residual, double* orthoganality){
    arma::colvec Kx(n);
    arma::colvec Ky(n);
    arma::colvec Kz(n);
    arma::colvec Dx(n);
    arma::colvec Dy(n);
    arma::colvec Dz(n);
    for(unsigned int i=0; i<n; i++){
        Kx(i) = Kx_in[i];
        Ky(i) = Ky_in[i];
        Kz(i) = Kz_in[i];
        Dx(i) = Dx_in[i];
        Dy(i) = Dy_in[i];
        Dz(i) = Dz_in[i];
    }
    std::pair<vec3d,pry> result = calc_orientation(Kx, Ky, Kz, Dx, Dy, Dz);
    *offset_x = result.first.x;
    *offset_y = result.first.y;
    *offset_z = result.first.z;
    *pitch = result.second.pitch;
    *roll  = result.second.roll;
    *yaw   = result.second.yaw;
	*residual = 0; //TODO: Calculate Residual
	*orthoganality = 0; //TODO: Calculate rot orthoganality
}

__declspec(dllexport)
void __cdecl to_global_coords(double  local_x,  double  local_y,  double  local_z,
                              double  offset_x, double  offset_y, double  offset_z,
					          double  pitch,    double  roll,     double  yaw,
                              double* global_x, double* global_y, double* global_z){
    arma::vec3 local;
	local(0) = local_x;
	local(1) = local_y;
	local(2) = local_z;
	arma::vec3 offset;
	offset(0) = offset_x;
	offset(1) = offset_y;
	offset(2) = offset_z;
	arma::vec3 global = offset + rotate(local, pitch, roll, yaw);
	*global_x = global(0);
	*global_y = global(1);
	*global_z = global(2);
}

__declspec(dllexport)
void __cdecl to_local_coords(double  global_x, double  global_y, double  global_z,
                             double  offset_x, double  offset_y, double  offset_z,
					         double  pitch,    double  roll,     double  yaw,
                             double* local_x,  double* local_y,  double* local_z){
    arma::vec3 global;
	global(0) = global_x;
	global(1) = global_y;
	global(2) = global_z;
	arma::vec3 offset;
	offset(0) = offset_x;
	offset(1) = offset_y;
	offset(2) = offset_z;
	arma::vec3 local = rotate(global-offset,pitch, roll, yaw, true);
	*local_x = local(0);
	*local_y = local(1);
	*local_z = local(2);
}

__declspec(dllexport) 
const char* __cdecl test_geom_1(const char* fname)
{
    std::string filename(fname);
    std::vector<sample> data = parse_data(filename);
    unsigned int n = data.size();
    arma::colvec Kx(n);
    arma::colvec Ky(n);
    arma::colvec Kz(n);
    arma::colvec Dx(n);
    arma::colvec Dy(n);
    arma::colvec Dz(n);
    for(unsigned int i=0; i < n; i++){
        Kx(i) = data[i].first.x;
        Ky(i) = data[i].first.y;
        Kz(i) = data[i].first.z;
        Dx(i) = data[i].second.x;
        Dy(i) = data[i].second.y;
        Dz(i) = data[i].second.z;
    }
	std::stringstream s;
    std::pair<vec3d,pry> result = calc_orientation(Kx, Ky, Kz, Dx, Dy, Dz);
    s << "offset: " << result.first.x << ", " 
                    << result.first.y << ", "
                    << result.first.z << std::endl;
    s << "pitch: " << result.second.pitch << std::endl
      << " roll: " << result.second.roll << std::endl
      << "  yaw: " << result.second.yaw << std::endl;
	return s.str().c_str();
}
