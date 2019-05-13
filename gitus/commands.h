#ifndef COMMANDS_H
#define COMMANDS_H

#include <iostream>

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
	const int InitMaxArgs = 0;
	bool _IsArleadyInitialize();
	bool _FileExist(std::string filePath);
	bool _FileInStaging(std::string fileName);
	bool AddFileToStaging(std::string filePath);
	std::string GetHeadFilePath();

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
