#include "XenonArgumentParser.h"
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <cassert>
#include <memory>

namespace Xenon {
namespace ArgumentParser {

void OptionParserBase::_opt_parse_arg ( std::string &p, const char *argValue, const OptionDesc &desc )
{
	if (!argValue)
		throw ArgumentParserError ( std::string("OptionsParser: Missing argument for parameter '") + std::string(desc.name));
	p.assign (argValue);
}

void OptionParserBase::_opt_parse_arg ( int32_t &p, const char *argValue, const OptionDesc &desc ) {
	int64_t tmp;
	_opt_parse_arg (tmp, argValue, desc);
	p = tmp;
}

void OptionParserBase::_opt_parse_arg ( int64_t &p, const char *argValue, const OptionDesc &desc ) {
	if (!argValue)
		throw ArgumentParserError ( std::string("OptionsParser: Missing argument for parameter '") + std::string(desc.name));
	char *e;
	p = strtol (argValue, &e, 10);
	if (e == argValue)
		throw ArgumentParserError ( std::string("OptionsParser: Could not parse argument '") + std::string(desc.name) + "'. Not a valid number");
}

void OptionParserBase::_opt_parse_arg ( float &p, const char *argValue, const OptionDesc &desc ) {
	if (!argValue)
		throw ArgumentParserError ( std::string("OptionsParser: Missing argument for parameter '") + std::string(desc.name));
	char *e;
	p = strtof (argValue, &e);
	if (e == argValue)
		throw ArgumentParserError ( std::string("OptionsParser: Could not parse argument '") + std::string(desc.name) + "'. Not a valid floating-point number");
}

void OptionParserBase::_opt_parse_arg ( bool &p, const char *argValue, const OptionDesc &desc ) {
	if (!argValue) {
		if ( (desc.flags & Options_Flag) == 0)
			throw ArgumentParserError ( std::string("OptionsParser: Missing argument for parameter '") + std::string(desc.name));
		p = true;
		return; 
	}
	if ( argValue[1] == '\0') { // One byte
		if (argValue[0] == '1')
			p = true;
		else if ( argValue[0] == '0')
			p = false;
		else
			throw ArgumentParserError ( std::string("OptionsParser: Could not parse argument '") + std::string(desc.name) + "'. Not a valid boolean value");
	} else if (strcmp (argValue, "true") == 0) {
		p = true;
	} else if (strcmp (argValue, "false") == 0) {
		p = false;
	} else
		throw ArgumentParserError ( std::string("OptionsParser: Could not parse argument '") + std::string(desc.name) + "'. Not a valid boolean value");
}

//

template<class T> void printHelpImpl (std::ostream &out, uint32_t programOptions, bool full,
				      const char *argName, const OptionDesc &desc, const T &defVal, char delim = '\0')
{
	if ((desc.flags & Options_Hidden) && !full)
		return;
	const char *req = (desc.flags & Options_Required) ? "*required; " : " \0";
	const char *rep = (desc.flags & Options_Multiple) ? "multiple; " : "";
	out << ' ' << req[0] << ' ';
	if (desc.shortOption)
		out << "-" << desc.shortOption << ", ";
	const int align = 28;
	char buf[align + 1];
	memset (buf, ' ', align);
	buf[align] = '\0';
	out <<  "--" << argName;
	
	const int nbytes = (req[0] != '\0') + 2 + (desc.shortOption ? 4 : 0) + 2 + strlen(argName);
	out << &buf[ std::min (nbytes, align) ] << desc.description 
		<< " (" << &req[1] << rep << "default: " << delim << defVal << delim << ")\n";
	if (!(programOptions & CompactHelp))
		out << '\n';
	
}

void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, const std::string &, const std::string &defVal) {
	printHelpImpl (out, appInfo.programOptions, full, argName, desc, defVal, '"');
}
void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, int, int defVal) {
	printHelpImpl (out, appInfo.programOptions, full, argName, desc, defVal);
}
void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, float, float defVal) {
	printHelpImpl (out, appInfo.programOptions, full, argName, desc, defVal);
}
void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, bool, bool defVal) {
	printHelpImpl (out, appInfo.programOptions, full, argName, desc, defVal);
}

void OptionParserBase::printHelpHead (std::ostream &out, const AppInformation &appInfos) {
	out << appInfos.programName << " " << appInfos.programVersion << std::endl;
	if (appInfos.programHelpTextHeader)
		out << appInfos.programHelpTextHeader;
	out << std::endl;
}

//

OptionParserBase::ParseResult OptionParserBase::parse (int argc, char **argv, const AppInformation &appInfos)
{
	static const int maxArgLen = 63, maxPosArgs = 32;
	char argName[maxArgLen + 1];
	
	uint16_t positionalArgs[ maxPosArgs ];
	uint32_t numPositionalArgs = 0U;
	for (int iArg = 1; iArg < argc; ++iArg)
	{
		const char *thisArg = argv[iArg];
		if (!thisArg[0] || (thisArg[0] == '-' && thisArg[1] == '\0'))
			throw ArgumentParserError("Invalid argument syntax");
		
		if (thisArg[0] == '-' && thisArg[1] == '-') // Long option
		{
			thisArg += 2;
			const char *sepPos = strchr (thisArg, '=');
			if (sepPos && (sepPos - thisArg) < maxArgLen) {
				memcpy (argName, thisArg, sepPos - thisArg);
				argName[sepPos - thisArg] = '\0';
				thisArg = argName;
			} else
				sepPos = NULL;
			
			const char *argValue = (sepPos) ? sepPos+1 : ((argc > iArg+1) ? argv[iArg+1] : NULL);
			OptionDesc selectedArg (NULL, 0);
			
			if (_opt_parseLongArgument (thisArg, argValue, &selectedArg)) {
				// Ok.
				assert (selectedArg.description != NULL);
				if (!sepPos && !(selectedArg.flags & Options_Flag)) // Does consume additional arg
					++iArg;
			} else if ( (appInfos.programOptions & NoHelp) == 0 && strcmp (thisArg, "help") == 0) {
				printHelp (std::cout, false, appInfos);
				return PARSE_TERMINATE;
			} else if ( (appInfos.programOptions & NoHelp) == 0 && strcmp (thisArg, "full-help") == 0) {
				printHelp (std::cout, (appInfos.programOptions & HideHidden) == 0, appInfos);
				return PARSE_TERMINATE;
			} else if ( (appInfos.programOptions & NoVersion) == 0 && strcmp (thisArg, "version") == 0) {
				std::cout << appInfos.programName << " - " << appInfos.programVersion << std::endl;
				return PARSE_TERMINATE;
			} else {
				if (!(appInfos.programOptions & IgnoreUnknown))
					throw ArgumentParserError(std::string("Unknown argument: ") + thisArg);
			}
		} else if (thisArg[0] == '-' && thisArg[1] != '-') // Short option
		{
			if (thisArg[2] == '\0') { // one short-hand argument. MAY Have a parameter
				const char *argValue = (argc > iArg+1) ? argv[iArg+1] : NULL;
				OptionDesc selectedArg (NULL, 0);
				if (!_opt_parseShortArgument (thisArg[1], argValue, &selectedArg)) {
					if (!(appInfos.programOptions & IgnoreUnknown))
						throw ArgumentParserError(std::string("Unknown short-form argument: ") + thisArg[1]);
				}
				if (!(selectedArg.flags & Options_Flag) && argValue)
					++iArg;
			}
			else {
				for (const char *s = &thisArg[1]; *s; ++s) {
					OptionDesc selectedArg (NULL, 0);
					if (!_opt_parseShortArgument (*s, NULL, &selectedArg)) {
						if (!(appInfos.programOptions & IgnoreUnknown))
							throw ArgumentParserError(std::string("Unknown short-form argument: ") + *s);
					}
					assert (selectedArg.flags & Options_Flag);
				}
			}
		}
		else  {
			if (iArg >= 0xFFFE || numPositionalArgs >= (maxPosArgs-1) )
				throw ArgumentParserError("Too many positional arguments given.");
			positionalArgs[numPositionalArgs++] = iArg;
		}
	}
	this->_opt_checkArguments(argv, numPositionalArgs, positionalArgs, appInfos);
	return PARSE_OK;
}

}
}
