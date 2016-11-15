#include <iostream>
#include "XenonArgumentParser.h"
#include <cstring>

const char *indentationValues[] = { "tabs", "spaces", "none", 0 };
const char *numIterations[] = { "1", "2", "3", "4", 0 };

XE_DECLARE_OPTIONS_GROUP(OpMode, "Operation mode", Xenon::ArgumentParser::Group_Exclusive | Xenon::ArgumentParser::Group_Required);
XE_DECLARE_OPTIONS_GROUP(GroupPerformance, "Performance options", 0);

#define CREATE_MY_OPTIONLIST(DEF) \
	\
	DEF(mode1, bool, (OptionDesc("operation mode1", Options_Flag).group(OpMode)), false) \
	DEF(mode2, bool, (OptionDesc("operation mode2", Options_Flag).group(OpMode)), false) \
	\
	DEF(filename, std::vector<std::string>, OptionDesc("some file-path", Options_Required | Options_Multiple | Options_Positional, 'f'), std::vector<std::string>())  \
	DEF(path, std::string, OptionDesc("base path", Options_None), "") \
	DEF(output, std::string, OptionDesc("output filepath. Requires path option as well", Options_Flag, 'o').XE_DEPEND_ON(path), "test.txt") \
	\
	DEF(prod_name, std::string, (OptionDesc("some name path. Only useful for operation mode 2", Options_None).setName("prod-name")), "") \
	DEF(indent, std::string, (OptionDesc("character used for indentation", Options_None).setEnum( indentationValues )), "tabs") \
	DEF(iterations, int, (OptionDesc("number of iterations. Depends on indent", Options_None).setEnum( numIterations ).XE_DEPEND_ON(indent)), 1) \
	\
	DEF(secret, bool, OptionDesc("some secret option!", Options_Hidden | Options_Flag, 's'), true) \
	DEF(flag, bool, OptionDesc("this is just a flag", Options_Flag, 'g'), false) \
	DEF(someInt, int, OptionDesc("my int", Options_None).setName("some-int"), 123) \
	DEF(myFloat, float, OptionDesc("my float", Options_None).setName("some-float"), 45.6f) \
	\
	DEF(print, bool, OptionDesc("Print all assigned values", Options_Flag), false) \
	DEF(lazy, bool, OptionDesc("Use lazy", Options_Flag).group(GroupPerformance), false) \
	
XE_DECLARE_PROGRAM_OPTIONS(MyOptions, CREATE_MY_OPTIONLIST);
XE_DEFINE_PROGRAM_OPTIONS_IMPL(MyOptions, CREATE_MY_OPTIONLIST);

template<class T> void print (std::ostream &s, const T &val) { s << "\"" << val << "\""; }
template<class T> void print (std::ostream &s, const std::vector<T> &vec) {
	int x = 0;
	for (const T &val : vec) {
		if( x++ > 0 ) s << ", ";
		print(s, val);
	}
}

#define PRINT_MY_OPTION(arg, type, desc, def) \
	if (!opt.has_##arg()) \
		std::cout << "'" << _OPTIONS_str(arg) << "' not given - assigned default value: "; \
	else \
		std::cout << "'" _OPTIONS_str(arg) << "' = "; \
	print(std::cout, opt.arg); \
	std::cout << "\n";  

int main(int argc, char **argv) {
	MyOptions opt;
	
	{
		MyOptions::Parser parser ("ExampleOptionParser", "0.1", Xenon::ArgumentParser::CompactHelp);
		parser.setHelpText("A simple program options parser example.\n");
		parser.setUsage ("Usage: ExampleOptionParser [-gs] [--filename path] filenames ...");
		if (parser.parse (opt, argc, argv) == MyOptions::Parser::PARSE_TERMINATE)
			return 0;
	}
	
	if (opt.print) {
		std::cout << "Printing all assigned values:\n";
		CREATE_MY_OPTIONLIST(PRINT_MY_OPTION)
	}
	
	for (const std::string &file : opt.filename) {
		std::cout << "File: " << file << "\n";
	}
	
	if (opt.has_prod_name())
		std::cout << "Prod name: " << opt.prod_name << "\n";
	
	return 0;
}
