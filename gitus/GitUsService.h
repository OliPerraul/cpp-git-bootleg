#pragma once
#include <iostream>
#include "boost/filesystem.hpp"

class GitusService {
private:
	const int InitMaxArgs = 0;
	bool IsArleadyInitialize();
	bool FileExist(std::string filePath);
	bool FileInStaging(std::string fileName);
public:
	bool InitRepo();
	bool AddFileToStaging(std::string filePath);
	std::string GetHeadFilePath();


};

bool GitusService::FileInStaging(std::string fileName){
	
}

bool GitusService::FileExist(std::string filePath) {
	bool exist = boost::filesystem::exist(filePath);
	return exist;
}

bool GitusService::AddFileToStaging(std::string filePath) {
	//Check if actualFile exist
	if (!FileExist(filePath)) {
		return false;
	}

	std::string fileName = boost::filesystem::path(filePath).filename();
	//Check si dans staging || local un file avec meme hash
		//yes:: do nothing return true
	//add file to staging (replace if one same name)

}

bool GitusService::InitRepo() {
	if (IsArleadyInitialize()) {
		return false;
	}
	boost::filesystem::path p = boost::filesystem::current_path();
	boost::filesystem::create_directory("gitus/");
	boost::filesystem::create_directory(p / "gitus" / "staging" );
	boost::filesystem::create_directory(p / "gitus" / "local");
	boost::filesystem::create_directory(p / "gitus" / "refs");
	boost::filesystem::ofstream(p / GetHeadFilePath());

	return true;
}

bool GitusService::IsArleadyInitialize() {
	boost::filesystem::path p = "gitus/";
	return boost::filesystem::is_directory(p);
}

std::string GitusService::GetHeadFilePath() {
	std::string path;
	path = "gitus/refs/master";
	return path;
}
