
#include <memory>
#include <iostream>
#include <ctime>

#include <boost/filesystem.hpp>

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

	if (filesystem::exists(GitusService::NewGitusDirectory()))
	{
		boost::filesystem::remove(_gitus->IndexFile());
		boost::filesystem::remove(_gitus->ObjectsDirectory());
		Init();
		cout << "Reinitialized existing Git repository in " << GitusService::NewGitusDirectory() << endl;
	}
	else
	{
		Init();
		cout << "Initialized empty Git repository in " << GitusService::NewGitusDirectory() << endl;
	}

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

	if (!BaseCommand::Execute())
		return false;

	RawData commitObject;
	if(_gitus->HashCommitTree(commitObject))
	if (treeHash == "") {
		std::cout << "Add file[s] to staging before committing..." << std::endl;
		return false;
	}


	//If current tree exist => it's current master..
	if (_gitus->CheckIfGitObjectExist(treeHash)) {
		std::cout << "Add file[s] to staging before committing..." << std::endl;
		return false;
	}
	commitObject += "tree"+ ' ' + treeHash + '\n';
	// parent tree hash
	if (_gitus->HasParentTree()) {
		auto parentTreeHash = _gitus->ParentTreeHash();
		commitObject += "parent" + ' '+ commitObject + '\n';
	}
	auto posxTime = std::time(0);

	auto author = this->GenerateAuthorCommit(posxTime);
	commitObject += author + "\n";

	auto committer = this->GenerateCommiterCommit(posxTime);
	commitObject += committer + "\n";
	
	//double \n before msg, git spec
	commitObject += "\n";
	commitObject += this->_msg + "\n";

	auto headSha1 = _gitus->HashObject(commitObject, GitusService::Commit);
	headSha1 += "\n";

	auto masterPath = _gitus->MasterFile();
	boost::filesystem::ofstream ofs{ masterPath };
	ofs << headSha1;
	std::cout << "committed to branch master with commit " + headSha1;

	return true;
}

