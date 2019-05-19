
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
	boost::filesystem::create_directory(_gitus->ObjectsDirectory());
	boost::filesystem::create_directory(_gitus->RefsDirectory());
	boost::filesystem::create_directory(_gitus->HeadsDirectory());
	boost::filesystem::ofstream(_gitus->HeadFile().string());
	boost::filesystem::ofstream(_gitus->MasterFile().string());
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
	
	entry.path = _pathspec;
	
	_gitus->HashObject(Utils::ReadBytes(fullPath.string()), GitusService::Blob, true, entry.sha1);
	
	if (entries.count(_pathspec) != 0) {
		auto indexedEntry = entries.at(_pathspec);
		if (indexedEntry.sha1 == entry.sha1) {
			cout << "The specified file is arleady added" << endl;
			return false;
		}

		entries.erase(_pathspec);
	}

	entries.insert(make_pair(entry.path, entry));

	_gitus->WriteIndex(entries);

	return true;
}


//--- Commit

bool CommitCommand::Execute() {

	using namespace std;
	using namespace boost::posix_time;

	if (!BaseCommand::Execute())
		return false;

	RawData directoryTreeObject;
	if(_gitus->HashCommitTree(directoryTreeObject))
	{
		// Error occured, return
		return false;
	}

	stringstream content;

	// Add tree information
	content << "tree ";
	content.write(
		reinterpret_cast<char*>(directoryTreeObject.data()), 
		directoryTreeObject.size() * sizeof(unsigned char));

	// Add parent commit object information
	if (_gitus->HasParentTree()) {
		RawData localMaster;
		_gitus->LocalMasterHash(localMaster);
		content << '\n';
		content << "parent ";
		content.write(
			reinterpret_cast<char*>(localMaster.data()),
			directoryTreeObject.size() * sizeof(unsigned char));
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
	std::cout << "committed to branch master with commit " + commitHexString.substr(0, 7);
	return true;
}

