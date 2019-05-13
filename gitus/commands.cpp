
#include <memory>
#include <iostream>


#include <boost/filesystem.hpp>

#include "gitus_service.h"
#include "commands.h"



//-- Init

bool InitCommand::_FileInStaging(std::string fileName) {
	return false;
}

bool InitCommand::_FileExist(std::string filePath) {
	bool exist = boost::filesystem::exists(filePath);
	return exist;
	
	
}

bool InitCommand::AddFileToStaging(std::string filePath) {
	//Check if actualFile exist
	if (!_FileExist(filePath)) {
		return false;

	
	}

	std::string fileName = boost::filesystem::path(filePath).filename().string();
	//Check si dans staging || local un file avec meme hash
		//yes:: do nothing return true
	//add file to staging (replace if one same name)
	return false;

}

bool InitCommand::_IsArleadyInitialize() {
	boost::filesystem::path p = "gitus/";
	return boost::filesystem::is_directory(p);
}



std::string InitCommand::GetHeadFilePath() {
	std::string path;
	path = "gitus/refs/master";
	return path;
}




bool InitCommand::Execute() {

	if (_IsArleadyInitialize()) {
		return false;
	}

	boost::filesystem::create_directory(GitusService::GitusDirectory());
	boost::filesystem::create_directory(GitusService::ObjectsDirectory());
	boost::filesystem::create_directory(GitusService::HeadsDirectory());
	boost::filesystem::ofstream(GitusService::IndexFile().string());
	boost::filesystem::ofstream(GitusService::HeadFile().string());
	boost::filesystem::ofstream(GitusService::MasterFile().string());
	return true;
}



//-- Add
/*
	"""Add all file paths to git index."""
	paths = [p.replace('\\', '/') for p in paths]
	all_entries = read_index()
	entries = [e for e in all_entries if e.path not in paths]
	for path in paths:
		sha1 = hash_object(read_file(path), 'blob')
		st = os.stat(path)
		flags = len(path.encode())
		assert flags < (1 << 12)
		entry = IndexEntry(
				int(st.st_ctime), 0, int(st.st_mtime), 0, st.st_dev,
				st.st_ino, st.st_mode, st.st_uid, st.st_gid, st.st_size,
				bytes.fromhex(sha1), flags, path)
		entries.append(entry)
	entries.sort(key=operator.attrgetter('path'))
	write_index(entries)
*/

/*
	for(auto entry = entries->begin(); entry!= entries->end; entry++)
	{

	}
*/

bool AddCommand::Execute() {

	auto entries = GitusService::ReadIndex();
	IndexEntry entry;
	entry.path = _pathspec;
	entry.sha1 = GitusService::HashObject(GitusService::ReadBytes(_pathspec), GitusService::Blob, true);
	entries->push_back(entry);

	// Sort entries
	sort(entries->begin(), entries->end(),
		[](const IndexEntry & a, const IndexEntry & b) -> bool
	{
		return a.path > b.path;
	});

	GitusService::WriteIndex(entries);

	return true;
}


//--- Commit

bool CommitCommand::Execute() {
	int i = 0;
	return true;
}

