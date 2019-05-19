#include <memory>
#include <iostream>

#include <boost/program_options.hpp>
#include "boost/filesystem.hpp"

#include "commands.h"
#include "gitus_service.h"
#include "utils.h"


std::shared_ptr<BaseCommand> CreateCommand(const std::shared_ptr<GitusService>& gitus, int argc, char **argv)
{
	namespace po = boost::program_options;
	using namespace std;

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
		("command", po::value<string>(), "command to execute")
		("subargs", po::value<vector<string>>(), "Arguments for command");

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

	string cmdName = vm["command"].as<string>();

	std::shared_ptr<BaseCommand> cmd;
	// Subprograms
	if (cmdName == "help")
	{
		return shared_ptr<BaseCommand>(new HelpCommand(gitus));
	}
	else if (cmdName == "init")
	{
		po::options_description desc("init options");
		desc.add_options()("help", "");

		// Collects 'init args
		vector<string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
		opts.erase(opts.begin());

		// Create help command
		cmd = shared_ptr<BaseCommand>(new InitCommandHelp(gitus));

		// Check if too many arguments
		if (opts.size() > 1)
		{
			cout << "Wrong number of positional arguments." << endl;
			return cmd;
		}

		po::store(po::command_line_parser(opts)
			.options(desc)
			.style(style)
			.run(), vm);

		// Check if help argument
		if (vm.count("help"))
		{
			return cmd;
		}
		else if (opts.size() != 0)
		{
			cout << "Wrong number of positional arguments." << endl;
			return cmd;
		}
		else
		{
			return shared_ptr<BaseCommand>(new InitCommand(gitus));
		}
	}
	else if (cmdName == "add")
	{
		po::options_description desc("init options");
		desc.add_options()
			("help", "")
			("pathspec", "");

		// Collects 'init args
		std::vector<string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
		opts.erase(opts.begin());
		
		// Create help command
		cmd = shared_ptr<BaseCommand>(new AddCommandHelp(gitus));

		// Check if too many arguments
		if (opts.size() > 1)
		{
			cout << "Wrong number of positional arguments." << endl;
			return cmd;
		}

		po::positional_options_description pos;
		pos.add("pathspec", 1);

		po::store(po::command_line_parser(opts)
			.options(desc)
			.style(style)
			.positional(pos)
			.run(), vm);

		if (vm.count("help"))
		{
			return cmd;
		}
		else if (opts.size() != 1)
		{
			cout << "Wrong number of positional arguments." << endl;
			return cmd; // return the help command
		}
		else
		{
			return shared_ptr<BaseCommand>(new AddCommand(gitus, vm["pathspec"].as<std::string>()));
		}
	}
	else if (cmdName == "commit")
	{
		po::options_description desc("init options");
		desc.add_options()
			("help", "")
			("msg", "")
			("author", "")
			("email", "");

		po::positional_options_description pos;
		pos.add("msg", 1)
			.add("author", 1)
			.add("email", 1);
		
		// Collects 'init args
		vector<string> opts = po::collect_unrecognized(parsed.options, po::include_positional);
		opts.erase(opts.begin());

		// Create help command
		cmd = shared_ptr<BaseCommand>(new CommitCommandHelp(gitus));

		// Check if too many arguments
		if (opts.size() > 3)
		{
			cout << "Wrong number of positional arguments." << endl;
			return cmd;
		}

		po::store(po::command_line_parser(opts)
			.options(desc)
			.positional(pos)
			.style(style)
			.run(), vm);

		if (vm.count("help"))
		{
			return cmd;
		}
		else if (opts.size() != 3)
		{
			cout << "Wrong number of positional arguments." << endl;
			return cmd;
		}
		else
		{
			return shared_ptr<BaseCommand>(new CommitCommand(
				gitus,
				vm["msg"].as<string>(),
				vm["author"].as<string>(),
				vm["email"].as<string>()
			));
		}
	}

	return false;
}

int main(int argc, char **argv)
{
	auto gitus = std::shared_ptr<GitusService>(new GitusService);
	auto cmd = CreateCommand(gitus, argc, argv);
	
	if (!cmd->Execute())
	{
		int x;
		std::cin >> x;
		return 1;
	}

	int x;
	std::cin >> x;
	return 0;
}