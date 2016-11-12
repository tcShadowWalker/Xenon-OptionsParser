#include <iostream>
#include <vector>
#include <cstring>
#include <memory>
#include "OptionsParser.h"

#define CREATE_MY_OPTIONLIST(DEF) \
	DEF(filename, std::string, "some file-path", "abc", Options_Required) \
	DEF(secret, bool, "some secret option!", true, Options_Hidden) \
	DEF(flag, bool, "this is just a flag", false, Options_Flag) \
	DEF(someInt, int, "my int", 123, Options_None) \
	DEF(myFloat, float, "my float", 45.6f, Options_None) \
	

struct Options : public OptionParserBase
{
	CREATE_MY_OPTIONLIST(OPTIONS_DEF_MEMBER)
	
	void parse (int argc, char **argv);
	
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
};

CREATE_MY_OPTIONLIST(OPTIONS_DEF_DEFAULT)

void Options::parse (int argc, char **argv)
{
	static const int _max_arg_name_len = 60;
	char _opt_arg_name[_max_arg_name_len + 1];
	
	for (int iArg = 1; iArg < argc; ++iArg)
	{
		const char *_this_arg = argv[iArg];
		const char *_opt_sepPos = (const char*)memchr (_this_arg, '=', _max_arg_name_len);
		if (_opt_sepPos) {
			memcpy (_opt_arg_name, _this_arg, _opt_sepPos - _this_arg);
			_opt_arg_name[_opt_sepPos - _this_arg] = '\0';
		} else {
			++iArg;
		}
		const char *argValue = (_opt_sepPos) ? _opt_sepPos+1 : argv[iArg];
		CREATE_MY_OPTIONLIST(OPTIONS_DEF_DO_PARSE)
		
		if ( (programOptions & OptionParser_NoHelp) == 0 && strcmp (_opt_arg_name, "--help") == 0) {
			printHelp (std::cout);
		} else if ( (programOptions & OptionParser_NoVersion) == 0 && strcmp (_opt_arg_name, "--version") == 0) {
			std::cout << programName << " - " << programVersion << std::endl;
		} else {
			std::cerr << "Invalid parameter: " << _opt_arg_name << std::endl;
		}
	}
	CREATE_MY_OPTIONLIST(OPTIONS_CHECK_REQUIRED_GIVEN)
}

/*struct MyOptionsPrinter {
	void operator() (const char *argName, const char *, const std::string &val, const std::string &, unsigned int flags) {
		std::cout << argName << " = \"" << val << "\"\n";
	}
	void operator() (const char *argName, const char *, int val, int, unsigned int flags) {
		std::cout << argName << " = " << val << "\n";
	}
	void operator() (const char *argName, const char *, float val, float, unsigned int flags) {
		std::cout << argName << " = " << val << "\n";
	}
	void operator() (const char *argName, const char *, bool val, bool, unsigned int flags) {
		std::cout << argName << " = " << val << "\n";
	}
};*/

#define PRINT_MY_OPTION(arg, type, desc, def, flags) \
	if (!opt.has_##arg()) \
		std::cout << "'" << _OPTIONS_str(arg) << "' not given - default value: \"" << opt.arg << "\"\n";  \
	else \
		std::cout << "'" _OPTIONS_str(arg) << "' = \"" << opt.arg << "\"\n";

int main(int argc, char **argv) {
	Options opt ("OptionParserTest", "0.1");
	opt.setHelpText("A simple program options parser example.\n");
	
	opt.parse (argc, argv);
	
	//MyOptionsPrinter printer;
	//opt.for_each_option (printer);
	CREATE_MY_OPTIONLIST(PRINT_MY_OPTION)
	
	return 0;
}
