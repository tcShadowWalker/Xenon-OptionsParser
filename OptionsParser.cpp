#include <stdlib.h>
#include <string>
#include "OptionsParser.h"
#include <stdexcept>
#include <iostream>
#include <cstring>
#include <cassert>
#include <memory>

void OptionParserBase::_opt_parse_arg ( std::string &p, const char *argValue, const OptionDesc &desc )
{
	if (!argValue)
		throw std::runtime_error ( std::string("OptionsParser: Missing argument for parameter '") + std::string(desc.name));
	p.assign (argValue);
}

void OptionParserBase::_opt_parse_arg ( std::int32_t &p, const char *argValue, const OptionDesc &desc ) {
	std::int64_t tmp;
	_opt_parse_arg (tmp, argValue, desc);
	p = tmp;
}

void OptionParserBase::_opt_parse_arg ( std::int64_t &p, const char *argValue, const OptionDesc &desc ) {
	if (!argValue)
		throw std::runtime_error ( std::string("OptionsParser: Missing argument for parameter '") + std::string(desc.name));
	char *e;
	p = strtol (argValue, &e, 10);
	if (e == argValue)
		throw std::runtime_error ( std::string("OptionsParser: Could not parse argument '") + std::string(desc.name) + "'. Not a valid number");
}

void OptionParserBase::_opt_parse_arg ( float &p, const char *argValue, const OptionDesc &desc ) {
	if (!argValue)
		throw std::runtime_error ( std::string("OptionsParser: Missing argument for parameter '") + std::string(desc.name));
	char *e;
	p = strtof (argValue, &e);
	if (e == argValue)
		throw std::runtime_error ( std::string("OptionsParser: Could not parse argument '") + std::string(desc.name) + "'. Not a valid floating-point number");
}

void OptionParserBase::_opt_parse_arg ( bool &p, const char *argValue, const OptionDesc &desc ) {
	if (!argValue) {
		if ( (desc.flags & Options_Flag) == 0)
			throw std::runtime_error ( std::string("OptionsParser: Missing argument for parameter '") + std::string(desc.name));
		p = true;
		return; 
	}
	if ( argValue[1] == '\0') { // One byte
		if (argValue[0] == '1')
			p = true;
		else if ( argValue[0] == '0')
			p = false;
		else
			throw std::runtime_error ( std::string("OptionsParser: Could not parse argument '") + std::string(desc.name) + "'. Not a valid boolean value");
	} else if (strcmp (argValue, "true") == 0) {
		p = true;
	} else if (strcmp (argValue, "false") == 0) {
		p = false;
	} else
		throw std::runtime_error ( std::string("OptionsParser: Could not parse argument '") + std::string(desc.name) + "'. Not a valid boolean value");
}

//

void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, const std::string &, const std::string &defVal) {
	if (desc.flags & Options_Hidden)
		return;
	const char *req = (desc.flags & Options_Required) ? "*required; " : "\0\0";
	out << "\t" << req[0] << argName << "\t\t" << desc.description << " (" << &req[1] << "default: \"" << defVal << "\")\n" << std::endl;
}
void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, int, int defVal) {
	if (desc.flags & Options_Hidden)
		return;
	const char *req = (desc.flags & Options_Required) ? "*required; " : "\0\0";
	out << "\t" << req[0] << argName << "\t\t" << desc.description << " (" << &req[1] << "default: " << defVal << ")\n" << std::endl;
}
void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, float, float defVal) {
	if (desc.flags & Options_Hidden)
		return;
	const char *req = (desc.flags & Options_Required) ? "*required; " : "\0\0";
	out << "\t" << req[0] << argName << "\t\t" << desc.description << " (" << &req[1] << "default: " << defVal << ")\n" << std::endl;
}
void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, bool, bool defVal) {
	if (desc.flags & Options_Hidden)
		return;
	const char *req = (desc.flags & Options_Required) ? "*required; " : "\0\0";
	out << "\t" << req[0] << argName << "\t\t" << desc.description << " (" << &req[1] << "default: " << defVal << ")\n" << std::endl;
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
	this->setParameters = 0;
	static const int maxArgLen = 63, maxPosArgs = 32;
	char argName[maxArgLen + 1];
	
	std::uint16_t positionalArgs[ maxPosArgs ];
	std::uint32_t numPositionalArgs = 0U;
	for (int iArg = 1; iArg < argc; ++iArg)
	{
		const char *thisArg = argv[iArg];
		if (!thisArg[0] || (thisArg[0] == '-' && thisArg[1] == '\0'))
			throw std::runtime_error("Invalid argument syntax");
		
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
			
			if (parseLongArgument (thisArg, argValue, &selectedArg)) {
				// Ok.
				assert (selectedArg.description != NULL);
				if (!sepPos && !(selectedArg.flags & Options_Flag)) // Does consume additional arg
					++iArg;
			} else if ( (programOptions & OptionParser_NoHelp) == 0 && strcmp (thisArg, "help") == 0) {
				printHelp (std::cout);
			} else if ( (programOptions & OptionParser_NoVersion) == 0 && strcmp (thisArg, "version") == 0) {
				std::cout << programName << " - " << programVersion << std::endl;
			} else {
				if (!(programOptions & OptionParser_IgnoreUnknown))
					throw std::runtime_error(std::string("Unknown argument: ") + thisArg);
			}
		} else if (thisArg[0] == '-' && thisArg[1] != '-') // Short option
		{
			if (thisArg[2] == '\0') { // one short-hand argument. MAY Have a parameter
				const char *argValue = (argc > iArg+1) ? argv[iArg+1] : NULL;
				OptionDesc selectedArg (NULL, 0);
				if (!parseShortArgument (thisArg[1], argValue, &selectedArg)) {
					if (!(programOptions & OptionParser_IgnoreUnknown))
						throw std::runtime_error(std::string("Unknown short-form argument: ") + thisArg[1]);
				}
				if (!(selectedArg.flags & Options_Flag) && argValue)
					++iArg;
			}
			else {
				for (const char *s = &thisArg[1]; *s; ++s) {
					OptionDesc selectedArg (NULL, 0);
					if (!parseShortArgument (*s, NULL, &selectedArg)) {
						if (!(programOptions & OptionParser_IgnoreUnknown))
							throw std::runtime_error(std::string("Unknown short-form argument: ") + *s);
					}
					assert (selectedArg.flags & Options_Flag);
				}
			}
		}
		else  {
			if (iArg >= 0xFFFE || numPositionalArgs >= (maxPosArgs-1) )
				throw std::runtime_error("Too many positional arguments given.");
			positionalArgs[numPositionalArgs++] = iArg;
		}
	}
	this->checkArguments(argv, numPositionalArgs, positionalArgs);
}
