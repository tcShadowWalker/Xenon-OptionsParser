#include "XenonArgumentParser.h"
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <cassert>
#include <memory>
#include <sstream>
#include <algorithm>

namespace Xenon {
namespace ArgumentParser {

void printEnumValues (std::ostream &out, const char * const *ev, char delim = ' ') {
	out << delim << *(ev++) << delim;
	for (; *ev; ++ev)
		out << "," << delim << *ev << delim;
}

namespace ParseFunctions {

bool check (const char *argValue, const OptionDesc &desc) {
	if (!argValue) {
		if (desc.flags & Options_Flag)
			return false; // Use default
		throw ArgumentParserError ( std::string("OptionsParser: Missing argument for parameter '") + std::string(desc.name));
	}
	if (desc.enumeration_values) {
		for (const char * const *ev = desc.enumeration_values; *ev; ++ev) {
			if ( strcmp (*ev, argValue) == 0)
				return true;
		}
		std::stringstream s;
		s << "OptionsParser: Invalid argument for parameter '" << desc.name << "'. Valid arguments are: ";
		printEnumValues (s, desc.enumeration_values);
		throw ArgumentParserError ( s.str() );
	}
	return true;
}

void parse ( std::string &p, const char *argValue, const OptionDesc &desc )
{
	if (!check (argValue, desc)) return;
	p.assign (argValue);
}

void parse ( const char * &p, const char *argValue, const OptionDesc &desc )
{
	if (!check (argValue, desc)) return;
	p = argValue;
}

void parse ( int32_t &p, const char *argValue, const OptionDesc &desc ) {
	int64_t tmp;
	parse (tmp, argValue, desc);
	p = tmp;
}

void parse ( int64_t &p, const char *argValue, const OptionDesc &desc ) {
	if (!check (argValue, desc)) return;
	char *e;
	p = strtol (argValue, &e, 10);
	if (e == argValue || *e != '\0')
		throw ArgumentParserError ( std::string("OptionsParser: Could not parse argument '") + std::string(desc.name) + "'. Not a valid number");
}

void parse ( float &p, const char *argValue, const OptionDesc &desc ) {
	if (!check (argValue, desc)) return;
	char *e;
	p = strtof (argValue, &e);
	if (e == argValue || *e != '\0')
		throw ArgumentParserError ( std::string("OptionsParser: Could not parse argument '") + std::string(desc.name) + "'. Not a valid floating-point number");
}

void parse ( bool &p, const char *argValue, const OptionDesc &desc ) {
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

// Help:

template<class T> void printHelpImpl (OHP &hp,  const OptionDesc &desc, const T &defVal, char delim = '\0')
{
	if ((desc.flags & Options_Hidden) && !hp.full)
		return;
	if (hp.lastGroup != desc.assignedGroup) {
		if (desc.assignedGroup) {
			hp.out << "\n" << desc.assignedGroup->desc << ": ";
			if (desc.assignedGroup->flags & Group_Required)
				hp.out << "*";
			hp.out << "\n";
			
		} else
			hp.out << "\n";
		hp.lastGroup = desc.assignedGroup;
	}
	const char *req = (desc.flags & Options_Required) ? "*required; " : " \0";
	hp.out << ' ' << req[0] << ' ';
	if (desc.shortOption)
		hp.out << "-" << desc.shortOption << ", ";
	const int align = 28;
	char buf[align + 1];
	memset (buf, ' ', align);
	buf[align] = '\0';
	hp.out <<  "--" << desc.name;
	
	const int nbytes = (req[0] != '\0') + 2 + (desc.shortOption ? 4 : 0) + 2 + strlen(desc.name);
	hp.out << &buf[ std::min (nbytes, align) ] << desc.description << " (" << &req[1];
	if (!(desc.flags & Options_Multiple))
		hp.out << "default: " << delim << defVal << delim;
	else
		hp.out << "multiple";
	if (desc.enumeration_values) {
		hp.out << "; values: ";
		printEnumValues (hp.out, desc.enumeration_values, delim);
	}
	hp.out << ")\n";
	if (!(hp.appInfo.programOptions & CompactHelp))
		hp.out << '\n';
	
}

void print_help (OHP &hp, const OptionDesc &desc, const std::string &, const std::string &defVal) {
	printHelpImpl (hp, desc, defVal, '"');
}
void print_help (OHP &hp, const OptionDesc &desc, const char* &, const char * &defVal) {
	printHelpImpl (hp, desc, defVal, '"');
}
void print_help (OHP &hp, const OptionDesc &desc, int, int defVal) {
	printHelpImpl (hp, desc, defVal);
}
void print_help (OHP &hp, const OptionDesc &desc, float, float defVal) {
	printHelpImpl (hp, desc, defVal);
}
void print_help (OHP &hp, const OptionDesc &desc, bool, bool defVal) {
	printHelpImpl (hp, desc, defVal);
}
}

void OptionParserBase::printHelpHead (std::ostream &out, const AppInformation &appInfos) {
	if (appInfos.usage)
		out << appInfos.usage << std::endl;
	else
		out << appInfos.programName << " " << appInfos.programVersion << std::endl;
	if (appInfos.programHelpTextHeader)
		out << appInfos.programHelpTextHeader;
	out << std::endl;
}

//

static void evalSelectedOption (const OptionDesc &selectedArg, char *nActiveGroupOptions, const OptionGroup **groups, const int maxGroups) {
	if (selectedArg.assignedGroup) {
		const OptionGroup **g = std::lower_bound (&groups[0], &groups[maxGroups], selectedArg.assignedGroup);
		assert (g < &groups[maxGroups]);
		assert (*g);
		nActiveGroupOptions[ g - &groups[0] ]++;
	}
}

OptionParserBase::ParseResult OptionParserBase::parse (int argc, char **argv, const AppInformation &appInfos)
{
	static const int maxArgLen = 63, maxPosArgs = 32, maxGroups = 32;
	char argName[maxArgLen + 1], nActiveGroupOtions[maxGroups];
	memset (&nActiveGroupOtions[0], 0, maxGroups);
	
	uint16_t positionalArgs[ maxPosArgs ];
	const OptionGroup *groups[ maxGroups ], **lastGroupPtr = &groups[0];
	this->_opt_enumerateGroups (lastGroupPtr, maxGroups);
	std::sort (&groups[0], lastGroupPtr);
	for (const OptionGroup *prev = NULL, **g = &groups[0]; g < lastGroupPtr; prev = *(g++)) {
		if (prev == *g)
			throw std::logic_error ("ArgumentParser: Multiple options within the same OptionGroup must appear in consecutive declaration order.");
	}
	
	uint32_t numPositionalArgs = 0U;
	for (int iArg = 1; iArg < argc; ++iArg)
	{
		const char *thisArg = argv[iArg];
		if (!thisArg[0] || (thisArg[0] == '-' && thisArg[1] == '\0'))
			throw ArgumentParserError("Invalid argument syntax");
		
		OptionDesc selectedArg (NULL, 0);
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
			const int pflags = (!sepPos) ? PARSE_IS_NEXT_ARG : 0;
			
			if (_opt_parseLongArgument (thisArg, argValue, &selectedArg, pflags)) {
				// Ok.
				assert (selectedArg.description != NULL);
				if ((pflags & PARSE_IS_NEXT_ARG) && !(selectedArg.flags & Options_Flag)) // Does consume additional arg
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
		} else if (thisArg[0] == '-' && thisArg[1] != '-' && thisArg[1] != '\0') // Short option
		{
			if (thisArg[2] == '\0') { // one short-hand argument. MAY Have a parameter
				const char *argValue = (argc > iArg+1) ? argv[iArg+1] : NULL;
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
					evalSelectedOption (selectedArg, nActiveGroupOtions, groups, lastGroupPtr - &groups[0]);
				}
			}
		}
		else  {
			if (iArg >= 0xFFFE || numPositionalArgs >= (maxPosArgs-1) )
				throw ArgumentParserError("Too many positional arguments given.");
			positionalArgs[numPositionalArgs++] = iArg;
		}
		if (selectedArg.description)
			evalSelectedOption (selectedArg, nActiveGroupOtions, groups, lastGroupPtr - &groups[0]);
	}
	this->_opt_checkArguments(argv, numPositionalArgs, positionalArgs, appInfos);
	for (const OptionGroup **g = &groups[0]; g < lastGroupPtr; ++g) {
		if ( (*g)->flags & Group_Required && nActiveGroupOtions[ g - &groups[0] ] < 1)
			throw ArgumentParserError(std::string("No option chosen for mandatory OptionGroup '") + (*g)->desc + "'");
		if ( (*g)->flags & Group_Exclusive && nActiveGroupOtions[ g - &groups[0] ] > 1)
			throw ArgumentParserError(std::string("Only one option may be chosen for OptionGroup '") + (*g)->desc + "'");
	}
	return PARSE_OK;
}

}
}
