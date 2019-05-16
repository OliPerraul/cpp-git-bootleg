#ifndef GITUS_COMMANDS_H
#define GITUS_COMMANDS_H

#include <iostream>
#include <ctime>

class BaseCommand {
	bool debug_;

public:
	virtual bool Execute() = 0;
};

//--- Help

class HelpCommand : public BaseCommand {
public:
	bool hidden_;
	std::string path_;

	virtual bool Execute() override
	{
		return false;
	};

};

//--- Init

class InitCommandHelp : public BaseCommand {
public:
	virtual bool Execute() override
	{
		std::cout<< "usage: git init" << std::endl;
		return true;
	};
};

class InitCommand : public BaseCommand {
private:
	bool _IsArleadyInitialize();
	bool _FileExist(std::string filePath);
public:
	
	virtual bool Execute() override;
};

//--- Add

class AddCommandHelp : public BaseCommand {
public:
	virtual bool Execute() override
	{
		std::cout<< "usage: gitus add <pathspec>" << std::endl;
		return true;
	};
};

class AddCommand : public BaseCommand {
private:
	std::string _pathspec;

public:

	AddCommand(std::string pathspec)
	{
		_pathspec = pathspec;
	};

	virtual bool Execute() override;

};


//--- Commit
class CommitCommandHelp : public BaseCommand {
public:
	virtual bool Execute() override
	{
		std::cout<< "usage: gitus commit <msg> <author> <email>" << std::endl;
		return true;
	};
};


class CommitCommand : public BaseCommand {
private:
	std::string _msg;
	std::string _author;
	std::string _email;
	
	std::string GenerateAuthorCommit(std::time_t posxTime) {
		return GenerateCommitMessage("author", posxTime);
	}

	std::string GenerateCommiterCommit(std::time_t posxTime) {
		return GenerateCommitMessage("committer", posxTime);

	}
	std::string GenerateCommitMessage(std::string type, std::time_t posxTime) {
		auto commitMessage = type + " " + _author + " " + "<" + _email + ">" + " " + std::to_string(posxTime);
		return commitMessage;
	}

public:
	virtual bool Execute() override;

	CommitCommand(std::string msg, std::string author, std::string email)
	{
		_msg = msg;
		_author = author;
		_email = email;
	};

};

#endif
