#include <iostream>

#include <dummy.h>

#include <iostream>
#include <boost/program_options.hpp>

#include <dummy.h>

// TODO: use boost::program_options::value<int>() for args
void init(bool res)
{
	std::cout << "INIT" << std::endl;
	int x;
	std::cin >> x;
}

void commit(bool res)
{
	std::cout << "DO IT" << std::endl;
	int x;
	std::cin >> x;
}

void add(bool res)
{
	std::cout << "ADD" << std::endl;
	int x;
	std::cin >> x;
}


int main(int argc, char **argv)
{
	using namespace boost::program_options;

	try
	{
		std::cout << argv[1] << std::endl;

		options_description description("Allowed options");
		variables_map options;
		store(parse_command_line(argc, argv, description), options);

		if (options.count("init"))
		{
			option_description d;
			///*description*/.add("init", boost::program_options::bool_switch()->notifier(&init));
		}

		if (description.options().empty())
		{
			description.add_options()
				("help,h", "Help screen")
				("init", boost::program_options::bool_switch()->notifier(&init))
				("commit", boost::program_options::bool_switch()->notifier(&commit));
			//("add", boost::program_options::bool_switch()->notifier(&add));
		}


		options.notify();
		//boost::program_options::notify(options);

	}
	catch (std::exception & ex)
	{
		std::cerr << "Error: " << ex.what() << std::endl;
		return 1;
	}
	
	return 0;
}
