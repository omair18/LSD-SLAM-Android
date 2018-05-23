#include "misc.h"
#include "settings.h"
#include "logger.h"



#define PC_DIR "/sdcard/LSD/pc"
#define DUMP_DIR "/sdcard/LSD/dump"
using namespace lsd_slam;

std::string &ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        return s;
}
std::string &rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
        return s;
}
std::string &trim(std::string &s) {
        return ltrim(rtrim(s));
}
int getdir (std::string dir, std::vector<std::string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if((dp  = opendir(dir.c_str())) == NULL)
    {
        return -1;
    }

    while ((dirp = readdir(dp)) != NULL) {
    	std::string name = std::string(dirp->d_name);

    	if(name != "." && name != "..")
    		files.push_back(name);
    }
    closedir(dp);


    std::sort(files.begin(), files.end());

    if(dir.at( dir.length() - 1 ) != '/') dir = dir+"/";
	for(unsigned int i=0;i<files.size();i++)
	{
		if(files[i].at(0) != '/')
			files[i] = dir + files[i];
	}

    return files.size();
}

int getFile (std::string source, std::vector<std::string> &files)
{
	std::ifstream f(source.c_str());

	if(f.good() && f.is_open())
	{
		while(!f.eof())
		{
			std::string l;
			std::getline(f,l);

			l = trim(l);

			if(l == "" || l[0] == '#')
				continue;

			files.push_back(l);
		}

		f.close();

		size_t sp = source.find_last_of('/');
		std::string prefix;
		if(sp == std::string::npos)
			prefix = "";
		else
			prefix = source.substr(0,sp);

		for(unsigned int i=0;i<files.size();i++)
		{
			if(files[i].at(0) != '/')
				files[i] = prefix + "/" + files[i];
		}

		return (int)files.size();
	}
	else
	{
		f.close();
		return -1;
	}
}

float computeDist(float* a, float* b, int size) {
    float dist = 0.0f;
    for (int i=0; i<size; ++i) {
        dist += std::pow(a[i] - b[i], 2);
    }
    return std::sqrt(dist);
}

void dumpCloudPoint(std::map<int, Keyframe*>& keyframes, int format) {
    if (format == 0) {
        std::string targetPathTmp = std::string(PC_DIR) + "/pc_tmp.ply";
        LOGD("Flushing Pointcloud to %s!\n", targetPathTmp.c_str());
    	std::ofstream f(targetPathTmp.c_str());
    	int numpts = 0;
    	for(std::map<int, Keyframe *>::iterator i = keyframes.begin(); i != keyframes.end(); ++i) {
    	    if (i->second->initId >= cutFirstNKf) {
    	        numpts += i->second->flushPC(&f, 0);
    	    }
    	}
    	f.flush();
    	f.close();

        std::string targetPath = std::string(PC_DIR) + "/pc.ply";
    	std::ofstream f2(targetPath.c_str());
    	f2 << std::string("ply\n");
    	f2 << std::string("format binary_little_endian 1.0\n");
    	f2 << std::string("element vertex ") << numpts << std::string("\n");
    	f2 << std::string("property float x\n");
    	f2 << std::string("property float y\n");
    	f2 << std::string("property float z\n");
    	f2 << std::string("property float intensity\n");
    	f2 << std::string("end_header\n");

    	std::ifstream f3(targetPathTmp.c_str());
    	while(!f3.eof()) f2.put(f3.get());

    	f2.close();
    	f3.close();

    	system((std::string("rm ") + targetPathTmp).c_str());
    	LOGD("Done Flushing Pointcloud with %d points!\n", numpts);
	} else {
	    std::string targetPath = std::string(PC_DIR) + "/pc.txt";
        LOGD("Flushing Pointcloud to %s!\n", targetPath.c_str());
    	std::ofstream f(targetPath.c_str());
    	int numpts = 0;
    	for(std::map<int, Keyframe *>::iterator i = keyframes.begin(); i != keyframes.end(); ++i) {
    	    if (i->second->initId >= cutFirstNKf) {
    	        numpts += i->second->flushPC(&f, 1);
    	    }
    	}
    	f.flush();
    	f.close();
    	LOGD("Done Flushing Pointcloud with %d points!\n", numpts);
	}
}

void dumpFile(unsigned char* data, int length) {
    std::string targetPath = std::string(DUMP_DIR) + "/aaa.dump";
    LOGD("dumpFile to %s!\n", targetPath.c_str());

    std::ofstream f(targetPath.c_str());
    for (int i=0; i<length; ++i) {
        f.put(data[i]);
    }
    f.flush();
    f.close();

    LOGD("Done dumpFile, size=%d!\n", length);
}

