#ifndef GITUS_COMMANDS_H
#define GITUS_COMMANDS_H

#include <iostream>
#include <ctime>
#include <memory>


#include "gitus_service.h"

class BaseCommand {

protected:
	std::shared_ptr<GitusService> _gitus;

public:

	BaseCommand(const std::shared_ptr<GitusService>& gitus)
	{
		_gitus = gitus;
	}

	virtual bool Execute()
	{
		if (!_gitus->CacheCurrentGitusDirectory())
		{
			std::cout << "fatal: not a git repository (or any of the parent directories): .git" << std::endl;
			return false;
		}

		return true;
	}
};

//--- Help

class HelpCommand : public BaseCommand {
public:
	bool hidden_;
	std::string path_;

	HelpCommand(const std::shared_ptr<GitusService>& gitus) : BaseCommand(gitus) {}

	virtual bool Execute() override
	{
		return false;
	};

};

//--- Init

class InitCommandHelp : public BaseCommand {
public:
	InitCommandHelp(const std::shared_ptr<GitusService>& gitus) : BaseCommand(gitus) {}

	virtual bool Execute() override
	{
		std::cout<< "usage: git init" << std::endl;
		return true;
	};
};

class InitCommand : public BaseCommand {
private:

	bool Init();
	
public:
	InitCommand(const std::shared_ptr<GitusService>& gitus) : BaseCommand(gitus) {}

	virtual bool Execute() override;
};

//--- Add

class AddCommandHelp : public BaseCommand {
public:
	AddCommandHelp(const std::shared_ptr<GitusService>& gitus) : BaseCommand(gitus) {}

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

	AddCommand(const std::shared_ptr<GitusService>& gitus, std::string pathspec) : BaseCommand(gitus)
	{
		_pathspec = pathspec;
	};

	virtual bool Execute() override;

};


//--- Commit

class CommitCommandHelp : public BaseCommand {
public:
	CommitCommandHelp(const std::shared_ptr<GitusService>& gitus) : BaseCommand(gitus) {}

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

	CommitCommand(const std::shared_ptr<GitusService>& gitus, std::string msg, std::string author, std::string email) : BaseCommand(gitus)
	{
		_msg = msg;
		_author = author;
		_email = email;
	};

};

#endif
