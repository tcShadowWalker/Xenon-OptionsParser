#include <iostream>
#include "OptionsParser.h"
#include <cstring>

#define CREATE_MY_OPTIONLIST(DEF) \
	DEF(filename, std::string, OptionDesc("some file-path", Options_Required), "abc") \
	DEF(secret, bool, OptionDesc("some secret option!", Options_Hidden), true) \
	DEF(flag, bool, OptionDesc("this is just a flag", Options_Flag), false) \
	DEF(someInt, int, OptionDesc("my int", Options_None), 123) \
	DEF(myFloat, float, OptionDesc("my float", Options_None), 45.6f) \
	

struct Options : public OptionParserBase
{
	CREATE_MY_OPTIONLIST(OPTIONS_DEF_MEMBER)
	
	enum Parameters {
		CREATE_MY_OPTIONLIST(OPTIONS_DEF_FLAG)
	};
	
	std::uint64_t _opt_setParameters;
	
	template<class F>
	void for_each_option (F &_opt_f) {
		CREATE_MY_OPTIONLIST(OPTIONS_DEF_OPERATION)
	}
	
	void printHelp (std::ostream &out) {
		printHelpHead(out);
		HelpPrinter printer {out}; for_each_option(printer);
	}
	
	Options (const char *appName, const char *version, unsigned int programOptions = 0)
		: OptionParserBase(appName, version, programOptions) 
		CREATE_MY_OPTIONLIST(OPTIONS_INIT_VAL)
		{}
protected:
	bool parseLongArgument (const char *argName, const char *argValue);
	void checkArguments ();
};

CREATE_MY_OPTIONLIST(OPTIONS_DEF_DEFAULT)

bool Options::parseLongArgument (const char *argName, const char *argValue)
{
	CREATE_MY_OPTIONLIST(OPTIONS_DEF_DO_PARSE)
	/* implicit else: */ {
		return false;
	}
	return true;
}

void Options::checkArguments () {
	CREATE_MY_OPTIONLIST(OPTIONS_CHECK_REQUIRED_GIVEN)
}

#define PRINT_MY_OPTION(arg, type, desc, def) \
	if (!opt.has_##arg()) \
		std::cout << "'" << _OPTIONS_str(arg) << "' not given - default value: \"" << opt.arg << "\"\n";  \
	else \
		std::cout << "'" _OPTIONS_str(arg) << "' = \"" << opt.arg << "\"\n";

int main(int argc, char **argv) {
	Options opt ("OptionParserTest", "0.1");
	opt.setHelpText("A simple program options parser example.\n");
	
	opt.parse (argc, argv);
	
	CREATE_MY_OPTIONLIST(PRINT_MY_OPTION)
	
	return 0;
}
