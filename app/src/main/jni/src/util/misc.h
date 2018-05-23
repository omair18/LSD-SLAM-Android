#ifndef MISC_H_
#define MISC_H_

#include <cmath>
#include <string>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <algorithm>
#include <map>
#include "IOWrapper/Android/Keyframe.h"


std::string &ltrim(std::string &s);
std::string &rtrim(std::string &s);
std::string &trim(std::string &s);
int getdir (std::string dir, std::vector<std::string> &files);
int getFile (std::string source, std::vector<std::string> &files);
float computeDist(float* a, float* b, int size);
void dumpCloudPoint(std::map<int, Keyframe*>& keyframes, int format);
void dumpFile(unsigned char* data, int length);




#endif // MISC_H_
