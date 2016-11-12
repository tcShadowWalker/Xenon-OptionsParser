#include <stdlib.h>
#include <string>
#include "OptionsParser.h"
#include <stdexcept>
#include <iostream>

void OptionParserBase::_opt_parse_arg ( std::string &p, const char *argValue, unsigned int flags, const char *argName )
{
	p.assign (argValue);
}

void OptionParserBase::_opt_parse_arg ( int &p, const char *argValue, unsigned int flags, const char *argName ) {
	char *e;
	p = strtol (argValue, &e, 10);
	if (e == argValue)
		throw std::runtime_error ( std::string("OptionsParser: Could not parse argument '") + std::string(argName) + "'. Not a valid number");
}

void OptionParserBase::_opt_parse_arg ( float &p, const char *argValue, unsigned int flags, const char *argName ) {
	
}

void OptionParserBase::_opt_parse_arg ( bool &p, const char *argValue, unsigned int flags, const char *argName ) {
	
}

//

void OptionParserBase::HelpPrinter::operator() (const char *argName, const char *desc, const std::string &, const std::string &defVal, unsigned int flags) {
	if (flags & Options_Hidden)
		return;
	out << "\t" << argName << "\t\t" << desc << " (default " << defVal << ")\n" << std::endl;
}
void OptionParserBase::HelpPrinter::operator() (const char *argName, const char *desc, int, int defVal, unsigned int flags) {
	if (flags & Options_Hidden)
		return;
	out << "\t" << argName << "\t\t" << desc << " (default " << defVal << ")\n" << std::endl;
}
void OptionParserBase::HelpPrinter::operator() (const char *argName, const char *desc, float, float defVal, unsigned int flags) {
	if (flags & Options_Hidden)
		return;
	out << "\t" << argName << "\t\t" << desc << " (default " << defVal << ")\n" << std::endl;
}
void OptionParserBase::HelpPrinter::operator() (const char *argName, const char *desc, bool, bool defVal, unsigned int flags) {
	if (flags & Options_Hidden)
		return;
	out << "\t" << argName << "\t\t" << desc << " (default " << defVal << ")\n" << std::endl;
}

void OptionParserBase::printHelpHead (std::ostream &out) {
	out << programName << " " << programVersion << std::endl;
	if (programHelpTextHeader)
		out << programHelpTextHeader;
	out << std::endl;
}
