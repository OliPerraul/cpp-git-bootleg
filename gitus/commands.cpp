
#include <memory>
#include <iostream>
#include <ctime>

#include <boost/filesystem.hpp>

#include "utils.h"
#include "commands.h"

//-- Init


bool InitCommand::_IsArleadyInitialize() {
	return boost::filesystem::exists(GitusUtils::GitusDirectory());
}

bool InitCommand::Execute() {

	if (_IsArleadyInitialize()) {
		return false;
	}

	boost::filesystem::create_directory(GitusUtils::GitusDirectory());
	boost::filesystem::create_directory(GitusUtils::ObjectsDirectory());
	boost::filesystem::create_directory(GitusUtils::RefsDirectory());
	boost::filesystem::create_directory(GitusUtils::HeadsDirectory());
	boost::filesystem::ofstream(GitusUtils::HeadFile().string());
	boost::filesystem::ofstream(GitusUtils::MasterFile().string());
	return true;
}


//-- Add

bool AddCommand::Execute() {

	using namespace std;

	auto entries = GitusUtils::ReadIndex();

	IndexEntry entry;
	entry.path = _pathspec;
	entry.sha1 = GitusUtils::HashObject(GitusUtils::ReadFile(_pathspec), GitusUtils::Blob, true);
	if (entries->count(_pathspec) != 0) {
		auto indexedEntry = entries->at(_pathspec);
		if (indexedEntry.sha1 == entry.sha1) {
			cout << "The specified file is arleady added" << endl;
			return false;
		}
		entries->erase(_pathspec);
	}

	entries->insert(make_pair(entry.path, entry));

	GitusUtils::WriteIndex(entries);

	return true;
}


//--- Commit

bool CommitCommand::Execute() {
	std::string commitObject;

	auto treeHash = GitusUtils::CreateCommitTree();
	if (treeHash == "") {
		std::cout << "Add file[s] to staging before committing..." << std::endl;
		return false;
	}
	//If current tree exist => it's current master..
	if (GitusUtils::CheckIfGitObjectExist(treeHash)) {
		std::cout << "Add file[s] to staging before committing..." << std::endl;
		return false;
	}
	commitObject += "tree"+ ' ' + treeHash + '\n';
	// parent tree hash
	if (GitusUtils::HasParentTree()) {
		auto parentTreeHash = GitusUtils::ParentTreeHash();
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

	auto headSha1 = GitusUtils::HashObject(commitObject, GitusUtils::Commit);
	headSha1 += "\n";

	auto masterPath = GitusUtils::MasterFile();
	boost::filesystem::ofstream ofs{ masterPath };
	ofs << headSha1;
	std::cout << "committed to branch master with commit " + headSha1;

	return true;
}

