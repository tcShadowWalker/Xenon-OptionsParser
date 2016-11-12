#include <stdlib.h>
#include <string>
#include "OptionsParser.h"
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <memory>

void OptionParserBase::_opt_parse_arg ( std::string &p, const char *argValue, const OptionDesc &desc )
{
	p.assign (argValue);
}

void OptionParserBase::_opt_parse_arg ( int &p, const char *argValue, const OptionDesc &desc ) {
	char *e;
	p = strtol (argValue, &e, 10);
	if (e == argValue)
		throw std::runtime_error ( std::string("OptionsParser: Could not parse argument '") + std::string(desc.name) + "'. Not a valid number");
}

void OptionParserBase::_opt_parse_arg ( float &p, const char *argValue, const OptionDesc &desc ) {
	
}

void OptionParserBase::_opt_parse_arg ( bool &p, const char *argValue, const OptionDesc &desc ) {
	
}

//

void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, const std::string &, const std::string &defVal) {
	if (desc.flags & Options_Hidden)
		return;
	out << "\t" << argName << "\t\t" << desc.description << " (default " << defVal << ")\n" << std::endl;
}
void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, int, int defVal) {
	if (desc.flags & Options_Hidden)
		return;
	out << "\t" << argName << "\t\t" << desc.description << " (default " << defVal << ")\n" << std::endl;
}
void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, float, float defVal) {
	if (desc.flags & Options_Hidden)
		return;
	out << "\t" << argName << "\t\t" << desc.description << " (default " << defVal << ")\n" << std::endl;
}
void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, bool, bool defVal) {
	if (desc.flags & Options_Hidden)
		return;
	out << "\t" << argName << "\t\t" << desc.description << " (default " << defVal << ")\n" << std::endl;
}

void OptionParserBase::printHelpHead (std::ostream &out) {
	out << programName << " " << programVersion << std::endl;
	if (programHelpTextHeader)
		out << programHelpTextHeader;
	out << std::endl;
}

//

void OptionParserBase::parse (int argc, char **argv)
{
	static const int _max_arg_name_len = 60;
	char _opt_arg_name[_max_arg_name_len + 1];
	
	int havePositionalArgs = ~0U;
	for (int iArg = 1; iArg < argc; ++iArg)
	{
		const char *_this_arg = argv[iArg];
		if (!_this_arg[0])
			throw std::runtime_error("Invalid argument syntax");
		
		if (_this_arg[0] == '-' && _this_arg[1] == '-') // Long option
		{ 
			const char *_opt_sepPos = (const char*)memchr (_this_arg, '=', _max_arg_name_len);
			if (_opt_sepPos) {
				memcpy (_opt_arg_name, _this_arg, _opt_sepPos - _this_arg);
				_opt_arg_name[_opt_sepPos - _this_arg] = '\0';
			} else {
				++iArg;
			}
			const char *argValue = (_opt_sepPos) ? _opt_sepPos+1 : argv[iArg];
			if (parseLongArgument (_opt_arg_name, argValue)) {
				// Ok.
			} else if ( (programOptions & OptionParser_NoHelp) == 0 && strcmp (_opt_arg_name, "--help") == 0) {
				printHelp (std::cout);
			} else if ( (programOptions & OptionParser_NoVersion) == 0 && strcmp (_opt_arg_name, "--version") == 0) {
				std::cout << programName << " - " << programVersion << std::endl;
			} else {
				std::cerr << "Invalid parameter: " << _opt_arg_name << std::endl;
			}
		} else if (_this_arg[0] == '-' && _this_arg[1] != '-') // Short option
		{
			// TODO
		} else  {
			if (havePositionalArgs == ~0U)
				havePositionalArgs = iArg;
		}
	}
	if (~havePositionalArgs) {
		for (int iArg = havePositionalArgs; iArg < argc; ++iArg)
		{
			
		}
	}
	this->checkArguments();
}
