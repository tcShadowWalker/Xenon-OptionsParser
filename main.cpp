#include <iostream>
#include "OptionsParser.h"
#include <cstring>

#define CREATE_MY_OPTIONLIST(DEF) \
	DEF(filename, std::string, OptionDesc("some file-path", Options_Required), "abc") \
	DEF(path, std::string, OptionDesc("base path", Options_Positional), "") \
	DEF(prod_name, std::string, OptionDesc("some name path", Options_Positional), "") \
	DEF(secret, bool, OptionDesc("some secret option!", Options_Hidden | Options_Flag, 's'), true) \
	DEF(flag, bool, OptionDesc("this is just a flag", Options_Flag, 'f'), false) \
	DEF(someInt, int, OptionDesc("my int", Options_None), 123) \
	DEF(myFloat, float, OptionDesc("my float", Options_None), 45.6f) \
	DEF(lazy, bool, OptionDesc("Use lazy", Options_Flag), false) \
	

DECLARE_PROGRAM_OPTIONS(MyOptions, CREATE_MY_OPTIONLIST);
DEFINE_PROGRAM_OPTIONS_IMPL(MyOptions, CREATE_MY_OPTIONLIST);

#define PRINT_MY_OPTION(arg, type, desc, def) \
	if (!opt.has_##arg()) \
		std::cout << "'" << _OPTIONS_str(arg) << "' not given - default value: \"" << opt.arg << "\"\n";  \
	else \
		std::cout << "'" _OPTIONS_str(arg) << "' = \"" << opt.arg << "\"\n";

int main(int argc, char **argv) {
	MyOptions opt ("OptionParserTest", "0.1");
	opt.setHelpText("A simple program options parser example.\n");
	
	opt.parse (argc, argv);
	
	CREATE_MY_OPTIONLIST(PRINT_MY_OPTION)
	
	return 0;
}
