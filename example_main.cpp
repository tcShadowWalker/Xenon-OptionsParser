#include <iostream>
#include "XenonArgumentParser.h"
#include <cstring>

const char *indentationValues[] = { "tabs", "spaces", "none", 0 };
const char *numIterations[] = { "1", "2", "3", "4", 0 };
const char *mergeAlgos[] = { "resolve", "recursive", "subtree", "ours", "octopus", 0 };

XE_DECLARE_OPTIONS_GROUP(GenOptions, "General options", 0);
XE_DECLARE_OPTIONS_GROUP(OpMode, "Operation mode", Xenon::ArgumentParser::Group_Exclusive | Xenon::ArgumentParser::Group_Required);
XE_DECLARE_OPTIONS_GROUP(GroupPerformance, "Performance options", 0);

#define CREATE_MY_OPTIONLIST(DEF) \
	\
	DEF(split, bool, (OptionDesc("Split files", Options_Flag).group(OpMode)), false) \
	DEF(merge, bool, (OptionDesc("Merge files", Options_Flag).group(OpMode)), false) \
	\
	DEF(file, std::vector<std::string>, OptionDesc("Path to a file", \
			Options_Required | Options_Multiple | Options_Positional, 'f').group(GenOptions), std::vector<std::string>())  \
	DEF(path, std::string, OptionDesc("Base path to change into", Options_None).group(GenOptions), "") \
	DEF(output, std::string, OptionDesc("output filepath", Options_Flag, 'o').group(GenOptions), "test.txt") \
	DEF(log, std::string, OptionDesc("Path to logfile. If not given, log to stdout", Options_None).group(GenOptions), "") \
	DEF(verbosity, int, OptionDesc("Log verbosity level (Value between 0 and 20)", Options_None, 'v').group(GenOptions), 6) \
	\
	DEF(merge_algo, const char *, (OptionDesc("Name of algorithm to use. Only useful for merging", Options_None) \
		.setName("merge-algorithm").XE_DEPEND_ON(merge).setEnum( mergeAlgos )), "") \
	DEF(indent, std::string, (OptionDesc("Character used for indentation", Options_None).setEnum( indentationValues )), "tabs") \
	DEF(iterations, int, (OptionDesc("Number of iterations.", Options_None).setEnum( numIterations )), 1) \
	\
	DEF(secret, bool, OptionDesc("A very secret option!", Options_Hidden | Options_Flag, 's'), true) \
	DEF(del, bool, OptionDesc("Delete all files after operation is done", Options_Flag, 'd').setName("delete"), false) \
	DEF(max, float, OptionDesc("Some numeric maximum value", Options_None), 45.6f) \
	\
	DEF(print, bool, OptionDesc("Print all assigned values", Options_Flag), false) \
	DEF(lazy, bool, OptionDesc("Use lazy evaluation", Options_Flag).group(GroupPerformance), false) \
	
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
		parser.setUsage ("Usage: ExampleOptionParser [--merge | --split] [options] [--file path] filenames ...");
		if (parser.parse (opt, argc, argv) == MyOptions::Parser::PARSE_TERMINATE)
			return 0;
	}
	
	if (opt.print) {
		std::cout << "Printing all assigned values:\n";
		CREATE_MY_OPTIONLIST(PRINT_MY_OPTION)
	}
	
	if (opt.merge) {
		if (opt.has_merge_algo())
			std::cout << "Merge algorthm name: " << opt.merge_algo << "\n";
		std::cout << "Will merge the following files:\n";
	} else if (opt.split) {
		std::cout << "Will split the following files:\n";
	}
		
	for (const std::string &file : opt.file) {
		std::cout << "File: " << file << "\n";
	}
	
	
	
	if (opt.del)
		std::cout << "Deleting all source files.\n";
	
	return 0;
}
