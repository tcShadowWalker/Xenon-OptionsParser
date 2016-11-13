#include <iostream>
#include "XenonArgumentParser.h"
#include <cstring>

// TODO: Groups!
// TODO: Modes

#define CREATE_MY_OPTIONLIST(DEF) \
	DEF(filename, std::vector<std::string>, OptionDesc("some file-path", Options_Required | Options_Multiple | Options_Positional, 'f'), std::vector<std::string>()) \
	DEF(path, std::string, OptionDesc("base path", Options_None), "") \
	DEF(output, std::string, OptionDesc("output filepath", Options_None, 'o'), "test.txt") \
	\
	DEF(prod_name, std::string, OptionDesc("some name path", Options_None), "") \
	DEF(secret, bool, OptionDesc("some secret option!", Options_Hidden | Options_Flag, 's'), true) \
	DEF(flag, bool, OptionDesc("this is just a flag", Options_Flag, 'f'), false) \
	DEF(someInt, int, OptionDesc("my int", Options_None), 123) \
	DEF(myFloat, float, OptionDesc("my float", Options_None), 45.6f) \
	\
	OPTIONS_DEF_GROUP("Performance optionss") \
	DEF(lazy, bool, OptionDesc("Use lazy", Options_Flag), false) \
	

XE_DECLARE_PROGRAM_OPTIONS(MyOptions, CREATE_MY_OPTIONLIST);
XE_DEFINE_PROGRAM_OPTIONS_IMPL(MyOptions, CREATE_MY_OPTIONLIST);

#define PRINT_MY_OPTION(arg, type, desc, def) \
	if (!opt.has_##arg()) \
		std::cout << "'" << _OPTIONS_str(arg) << "' not given - default value: \"" << opt.arg << "\"\n";  \
	else \
		std::cout << "'" _OPTIONS_str(arg) << "' = \"" << opt.arg << "\"\n";

int main(int argc, char **argv) {
	MyOptions opt;
	
	{
		MyOptions::Parser parser ("ExampleOptionParser", "0.1", Xenon::ArgumentParser::CompactHelp);
		parser.setHelpText("A simple program options parser example.\n");
		if (parser.parse (opt, argc, argv) == MyOptions::Parser::PARSE_TERMINATE)
			return 0;
	}
	
	//CREATE_MY_OPTIONLIST(PRINT_MY_OPTION)
	for (const std::string &file : opt.filename) {
		std::cout << "File: " << file << "\n";
	}
	
	return 0;
}
