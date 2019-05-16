#include <memory>
#include <iostream>

#include <boost/program_options.hpp>
#include "boost/filesystem.hpp"

#include "commands.h"
#include "gitus_service.h" //TODO use if state is shared across cmds


std::unique_ptr<BaseCommand> ParseCommand(int argc, char **argv)
{
	namespace po = boost::program_options;

	// Cmd line params (e.g single dash --help vs -help
	po::command_line_style::style_t style = po::command_line_style::style_t(
		po::command_line_style::unix_style |
		po::command_line_style::case_insensitive |
		po::command_line_style::allow_long_disguise |
		po::command_line_style::allow_dash_for_short);

	po::options_description global("Global options");
	global.add_options()
		// global help
		("help,help", "Display this help message")
		// positional arguments need to be added
		("command", po::value<std::string>(), "command to execute")
		("subargs", po::value<std::vector<std::string>>(), "Arguments for command");

	po::positional_options_description pos;
	pos.add("command", 1).
		add("subargs", -1);

	po::variables_map vm;

	po::parsed_options parsed = po::command_line_parser(argc, argv).
		options(global)
		.style(style)
		.positional(pos)
		.allow_unregistered()
		.run();

	po::store(parsed, vm);

	std::string cmdName = vm["command"].as<std::string>();

	std::unique_ptr<BaseCommand> cmd;

	// Subprograms

	if (cmdName == "help")
	{
		//auto commit = new CommitCommand;
		//cmd = std::unique_ptr<BaseCommand>(commit);
	}
	else if (cmdName == "init")
	{
		po::options_description desc("init options");
		desc.add_options()("help", "Show hidden files");

		// Collects 'init args
		std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
		opts.erase(opts.begin());

		po::store(po::command_line_parser(opts)
			.options(desc)
			.style(style)
			.run(), vm);
		
		cmd = vm.count("help") ?
			std::unique_ptr<BaseCommand>(new InitCommandHelp) :
			std::unique_ptr<BaseCommand>(new InitCommand);
	}
	else if (cmdName == "add")
	{
		po::options_description desc("init options");
		desc.add_options()
			("help", "Show hidden files")
			("pathspec", "path of the file");

		// Collects 'init args
		std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
		opts.erase(opts.begin());

		po::positional_options_description pos;
		pos.add("pathspec", 1);

		po::store(po::command_line_parser(opts)
			.options(desc)
			.style(style)
			.positional(pos)
			.run(), vm);

		cmd = vm.count("help") ?
			std::unique_ptr<BaseCommand>(new AddCommandHelp) :
			std::unique_ptr<BaseCommand>(new AddCommand(vm["pathspec"].as<std::string>()));
	}
	else if (cmdName == "commit")
	{
		po::options_description desc("init options");
		desc.add_options()
			("help", "Show hidden files")
			("msg", "path of the file")
			("author", "path of the file")
			("email", "path of the file");

		po::positional_options_description pos;
		pos.add("msg", 1)
			.add("author", 1)
			.add("email", 1);
		
		// Collects 'init args
		std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
		opts.erase(opts.begin());

		po::store(po::command_line_parser(opts)
			.options(desc)
			.positional(pos)
			.style(style)
			.run(), vm);

		cmd = vm.count("help") ?
			std::unique_ptr<BaseCommand>(new CommitCommandHelp) :
			std::unique_ptr<BaseCommand>(new CommitCommand(
				vm["msg"].as<std::string>(),
				vm["author"].as<std::string>(),
				vm["email"].as<std::string>()
				));
	}

	return cmd;
}

int main(int argc, char **argv)
{
	GitusService service;
	GitusService::CreateCommitTree();
//	std::unique_ptr<BaseCommand> cmd = ParseCommand(argc, argv);
	//cmd->Execute();
}