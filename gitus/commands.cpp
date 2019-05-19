
#include <memory>
#include <iostream>

#include <boost/filesystem.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/posix_time/conversion.hpp"

#include "commands.h"
#include "utils.h"


//-- Init

bool InitCommand::Init()
{
	boost::filesystem::create_directory(GitusService::NewGitusDirectory());
	_gitus->CacheCurrentGitusDirectory();

	boost::filesystem::create_directory(_gitus->ObjectsDirectory());
	boost::filesystem::create_directory(_gitus->RefsDirectory());
	boost::filesystem::create_directory(_gitus->HeadsDirectory());
	
	auto masterFilePath = _gitus->MasterFile().string();
	boost::filesystem::ofstream{ masterFilePath};
	boost::filesystem::ofstream headFile{ _gitus->HeadFile().string()};
	headFile << "ref: refs / heads / master";

	return true;
}

bool InitCommand::Execute() {

	using namespace std;
	using namespace boost;

	string msg;
	// Reinitialize the repository
	if (filesystem::exists(GitusService::NewGitusDirectory()))
	{
		boost::filesystem::remove(_gitus->IndexFile());
		boost::filesystem::remove(_gitus->ObjectsDirectory());
		Init();
		msg = "Reinitialized existing Git repository in ";
	}
	// Create new repository
	else
	{
		Init();
		msg = "Initialized empty Git repository in ";
	}

	_gitus->CacheCurrentGitusDirectory();
	msg += _gitus->RepoDirectory().string();

	cout << msg << endl;
	return true;
}


//-- Add

bool AddCommand::Execute() {

	using namespace std;
	using namespace boost;

	if (!BaseCommand::Execute())
		return false;

	auto fullPath = _gitus->RepoDirectory() / _pathspec;

	if (!filesystem::exists(fullPath))
	{
		cout << "fatal: pathspec '" << _pathspec <<"' did not match any files" << endl;
		return false;
	}

	auto entries = map<string, IndexEntry>();
	if (!_gitus->ReadIndex(entries))
	{
		// Error while reading, return.
		return false;
	}

	IndexEntry entry;

	_gitus->HashObject(Utils::ReadBytes(fullPath.string()), GitusService::Blob, true, entry.sha1);

	entry.path = _pathspec;

	if (entries.count(_pathspec) != 0) {

		auto indexedEntry = entries.at(_pathspec);
		if (indexedEntry.sha1 == entry.sha1) {
			cout << "The file '" << _pathspec << "' is arleady inside the index." << endl;
			return false;
		}
	}

	entries.insert(make_pair(entry.path, entry));

	_gitus->WriteIndex(entries);

	cout << "File '" << _pathspec << "' added to the index." << std::endl;
	return true;
}


//--- Commit

bool CommitCommand::Execute() {

	using namespace std;
	using namespace boost::posix_time;

	if (!BaseCommand::Execute())
		return false;
	
	
	auto currentTree = _gitus->HashCommitTree();
	if (currentTree.empty())
	{
		// Error occured, return
		return false;
	}

	//Check to see if parent hash obj is same as current index
	auto treeContent = _gitus->CreateContentData(currentTree, GitusService::Tree);
	string treeStringHash;
	Utils::Sha1String(treeContent, treeStringHash);
	if(_gitus->ObjectExists(treeStringHash)) 
	{
		std::cout << "nothing to commit, working tree clean" << std::endl;
		return false;
	}

	RawData directoryTreeObject;
	_gitus->HashObject(currentTree, GitusService::Tree, true, directoryTreeObject);

	stringstream content;
	// Add tree information
	content << "tree ";
	content.write(
		reinterpret_cast<char*>(directoryTreeObject.data()),
		directoryTreeObject.size() * sizeof(unsigned char));

	// Add parent commit object information
	// If has local master and local master is not empty
	if (_gitus->HasParentTree()) {
		RawData localMaster;
		_gitus->LocalMasterHash(localMaster);

		if (localMaster.size() > 0)
		{
			content << '\n';
			content << "parent ";
			content.write(
				reinterpret_cast<char*>(localMaster.data()),
				directoryTreeObject.size() * sizeof(unsigned char));
		}
	}

	// Add author information
	content << '\n';
	time_t utcTime = to_time_t(second_clock::universal_time());
	string timezone = "+0000";	
	content << "author " << _author << utcTime << timezone;

	// Add commiter information (Simplified: same as author)
	content << '\n';
	content << "author " << _author << _email << utcTime << timezone;

	// Add linebreak as per the specification
	content << '\n';

	// Add the message
	content << '\n';
	content << _msg;

	RawData commitObject(
		(std::istreambuf_iterator<char>(content)),
		(std::istreambuf_iterator<char>()));

	// Get hash representation
	RawData commitHash;
	_gitus->HashObject(commitObject, GitusService::Commit, true, commitHash);
	commitHash.push_back('\n');

	// Write commit representation to master file
	boost::filesystem::ofstream ofs{ _gitus->MasterFile() };
	ofs.write(reinterpret_cast<char*>(commitHash.data()), commitHash.size()*sizeof(unsigned char));
	
	string commitHexString;
	Utils::Sha1String(commitObject, commitHexString);
	std::cout << "committed to branch master with commit " + commitHexString.substr(0, 7) << std::endl;
	return true;
}

