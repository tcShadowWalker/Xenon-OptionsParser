#pragma once
#ifndef ARGUMENT_PARSER_NO_STL_SUPPORT
	#include <vector>
#endif
#include <stdexcept>
#include <string>
#include <stdint.h>
#include <iostream>

namespace Xenon {
namespace ArgumentParser {

enum OptionsFlags {
	Options_None             = 0,
	/// Do not print this parameter when --help is given.
	Options_Hidden           = 1U << 0,
	/// Positional parameter can be given without the argument name or short-flag
	Options_Positional       = 1U << 1,
	/// This parameter requires no argument value, but serves as a flag. If no argument is given, it's default value is assigned.
	Options_Flag             = 1U << 2,
	/// This parameter is mandatory; If not given, evaluation will fail.
	Options_Required         = 1U << 3,
	/// Parameter can be given more than once. Use vectors for these arguments.
	/// If combined with @link Options_Positional, will consume all positional arguments.
	Options_Multiple         = 1U << 4,
};

enum OptionParserFlags {
	/// Do not generate help text. Can be used to provide --help generation manually.
	NoHelp         = 1U << 0,
	/// Do not generate behaviour for --version option
	NoVersion      = 1U << 1,
	/// Don't generate full-help showing hidden options
	HideHidden     = 1U << 2,
	/// Don't abort parsing when encountering unknown arguments
	IgnoreUnknown  = 1U << 3,
	/// When printing the help-page, use only one line per argument
	CompactHelp    = 1U << 4,
};

enum OptionGroupFlags {
	/// At most one option can be active
	Group_Exclusive      = 1U << 0,
	/// At least one option from this group must be active
	Group_Required       = 1U << 1,
};

struct OptionGroup {
	const char *desc;
	std::uint32_t flags;
	
	OptionGroup (const char *desc, unsigned int pFlags = 0) : desc(desc), flags(pFlags) { }
};

struct OptionDesc {
	const char *name, *description;
	const char * const *enumeration_values;
	const OptionGroup *assignedGroup;
	uint64_t depends_on;
	unsigned int flags;
	char shortOption;
	
	OptionDesc (const char *desc, unsigned int flags = 0, char shortOpt = 0)
	  : name(NULL), description(desc), enumeration_values(NULL), assignedGroup(NULL),
	    depends_on(0), flags(flags), shortOption(shortOpt) { }
	
	OptionDesc &setName (const char *name) { if (!this->name) { this->name = name; } return *this; }
	OptionDesc &setEnum (const char * const * const enum_values) {enumeration_values = enum_values; return *this; }
	OptionDesc &dependOn (int64_t option_bit) { depends_on |= option_bit; return *this; }
	OptionDesc &group (const OptionGroup &grp) { this->assignedGroup = &grp; return *this; }
};

struct RequiredArgumentMissing : public std::exception
{
	RequiredArgumentMissing (const char *arg) : arg(arg) { s = "Missing required argument '"; s+= arg; s += "'"; }
	~RequiredArgumentMissing () throw() { }
	const char *what () const throw() { return s.c_str(); }
	
	const char *arg;
	std::string s;
};

struct ArgumentParserError : public std::runtime_error
{
	ArgumentParserError (const std::string &s) : runtime_error(s) { }
};

struct AppInformation
{
	unsigned int programOptions;
	const char *programName, *programVersion, *programHelpTextHeader, *programHelpTextTail, *usage;
	std::ostream *helpOutputStream; ///< If NULL; default is std::cout
	
	AppInformation (const char *appName, const char *version, unsigned int programOptions = 0)
		: programOptions(programOptions), programName(appName), programVersion(version),
		programHelpTextHeader(NULL), programHelpTextTail(NULL), usage(NULL), helpOutputStream(NULL) { }
	
	AppInformation &setHelpText (const char *head, const char *tail = NULL) { programHelpTextHeader = head; programHelpTextTail = tail; return *this; }
	AppInformation &setUsage (const char *txt) { usage = txt; return *this; }
};

struct OptionParserBase
{
	struct HelpPrinter {
		template<class T> void operator() (const OptionDesc &desc, const T &v, const T &d);
		
		HelpPrinter (std::ostream &o, const AppInformation & ai, bool f) : out(o), appInfo(ai), full(f), lastGroup(NULL) { }
		std::ostream &out;
		const AppInformation &appInfo;
		bool full;
		const OptionGroup *lastGroup;
	};
	
	enum ParseResult {
		PARSE_OK = 1,
		/// --help or --version was given; Action was taken. App should terminate.
		PARSE_TERMINATE = 2,
	};
	
	virtual void printHelp (std::ostream &out, bool full, const AppInformation &appInfo) = 0;
protected:
	void printHelpHead (std::ostream &out, const AppInformation &appInfos);
	
	enum ParseFlags {
		PARSE_IS_NEXT_ARG = 1,
	};
	ParseResult parse (int argc, char **argv, const AppInformation &appInfos);
	virtual bool _opt_parseLongArgument (const char *argName, const char *argValue, OptionDesc *selectedArg, int parseFlags = 0) = 0;
	virtual bool _opt_parseShortArgument (char arg, const char *argValue, OptionDesc *selectedArg) = 0;
	virtual void _opt_checkArguments (char **argv, uint32_t numPositionalArgs, uint16_t *positionalArgs, const AppInformation &appInfo) = 0;
	virtual void _opt_enumerateGroups (const OptionGroup **&groups, unsigned int maxGroups) = 0;
};

/// @brief Contains all the parsing functions for each type. Can be extended by the user.
namespace ParseFunctions {
	typedef OptionParserBase::HelpPrinter OHP;
	
	void parse ( std::string &p, const char *argValue, const OptionDesc &desc );
	void parse ( const char * &p, const char *argValue, const OptionDesc &desc );
	void parse ( int32_t &p, const char *argValue, const OptionDesc &desc );
	void parse ( int64_t &p, const char *argValue, const OptionDesc &desc );
	void parse ( float &p, const char *argValue, const OptionDesc &desc );
	void parse ( bool &p, const char *argValue, const OptionDesc &desc );
	
#ifndef ARGUMENT_PARSER_NO_STL_SUPPORT
	template<class T, class Alloc>
	void parse ( std::vector<T, Alloc> &p, const char *argValue, const OptionDesc &desc ) {
		T val;
		parse (val, argValue, desc);
		p.push_back(val);
	}
#endif

	void print_help (OHP &, const OptionDesc &desc, const std::string &, const std::string &defVal);
	void print_help (OHP &, const OptionDesc &desc, const char* &, const char* &defVal);
	void print_help (OHP &, const OptionDesc &desc, int32_t, int32_t defVal);
	void print_help (OHP &, const OptionDesc &desc, int64_t, int64_t defVal);
	void print_help (OHP &, const OptionDesc &desc, float, float defVal);
	void print_help (OHP &, const OptionDesc &desc, bool, bool defVal);
	
#ifndef ARGUMENT_PARSER_NO_STL_SUPPORT
	template<class T, class Alloc>
	void print_help (OHP &hp, const OptionDesc &desc, const std::vector<T, Alloc> &, const std::vector<T, Alloc> &) {
		const T val;
		print_help (hp, desc, val, val);
	}
#endif

};

template<class T>
void OptionParserBase::HelpPrinter::operator() (const OptionDesc &desc, const T &v, const T &d) {
	ParseFunctions::print_help (*this, desc, v, d);
}

/**
 * @brief Main macro to define list of program options
 */
#define XE_DECLARE_PROGRAM_OPTIONS(OPTIONS_CLASS_NAME, OPTION_LIST_MACRO_NAME)      \
struct OPTIONS_CLASS_NAME##_Parser; \
struct OPTIONS_CLASS_NAME \
{     \
	typedef Xenon::ArgumentParser::OptionDesc OptionDesc; \
	uint64_t setParameters; \
	OPTION_LIST_MACRO_NAME(XE_ARG_PARSE_OPTIONS_DEF_MEMBER)     \
	\
	enum _opt_Parameters {     \
		OPTION_LIST_MACRO_NAME(XE_ARG_PARSE_OPTIONS_DEF_FLAG)     \
	};     \
	\
	template<class F>     \
	void for_each_option (F &_opt_f) {     \
		using namespace Xenon::ArgumentParser; \
		OPTION_LIST_MACRO_NAME(XE_ARG_PARSE_OPTIONS_DEF_OPERATION)     \
	}     \
	\
	OPTIONS_CLASS_NAME ()     \
		: setParameters(0) OPTION_LIST_MACRO_NAME(XE_ARG_PARSE_OPTIONS_INIT_VAL)   {}     \
	typedef OPTIONS_CLASS_NAME##_Parser Parser; \
	typedef OPTIONS_CLASS_NAME _XE_OPT_DATA; \
}; \
\
struct OPTIONS_CLASS_NAME##_Parser : public Xenon::ArgumentParser::OptionParserBase, public Xenon::ArgumentParser::AppInformation { \
	\
	typedef Xenon::ArgumentParser::OptionDesc OptionDesc; \
	typedef OPTIONS_CLASS_NAME _XE_OPT_DATA; \
	void printHelp (std::ostream &out, bool full, const Xenon::ArgumentParser::AppInformation &appInfo) {     \
		using namespace Xenon::ArgumentParser; \
		printHelpHead(out, appInfo);     \
		HelpPrinter printer (out, appInfo, full); \
		data->for_each_option(printer);     \
		if (appInfo.programHelpTextTail) \
			out << appInfo.programHelpTextTail; \
		out << std::endl; \
	}     \
	\
	ParseResult parse (OPTIONS_CLASS_NAME &opts, int argc, char **argv) { \
		this->data = &opts; \
		return this->OptionParserBase::parse (argc, argv, *this); \
	} \
	OPTIONS_CLASS_NAME##_Parser (const char *appName, const char *version, unsigned int programOptions = 0) \
		: AppInformation(appName, version, programOptions) { } \
protected:     \
	OPTIONS_CLASS_NAME *data; \
	\
	bool _opt_parseLongArgument (const char *argName, const char *argValue, OptionDesc *selectedArg, int parseFlags);     \
	bool _opt_parseShortArgument (char arg, const char *argValue, OptionDesc *selectedArg);     \
	void _opt_checkArguments (char **argv, uint32_t numPositionalArgs, uint16_t *positionalArgs, const Xenon::ArgumentParser::AppInformation &);     \
	void _opt_enumerateGroups (const Xenon::ArgumentParser::OptionGroup **&groups, unsigned int maxGroups); \
};

/**
 * @brief Provide parser implementation code for program options
 */
#define XE_DEFINE_PROGRAM_OPTIONS_IMPL(OPTIONS_CLASS_NAME, OPTION_LIST_MACRO_NAME)     \
bool OPTIONS_CLASS_NAME##_Parser::_opt_parseLongArgument (const char *argName, const char *argValue, OptionDesc *selectedArg, const int parseFlags) {      \
	using namespace Xenon::ArgumentParser; \
	OPTION_LIST_MACRO_NAME(XE_ARG_PARSE_OPTIONS_DEF_DO_PARSE)      \
	/* implicit else: */ {      \
		return false;      \
	}      \
	return true;      \
}      \
bool OPTIONS_CLASS_NAME##_Parser::_opt_parseShortArgument (char arg, const char *argValue, OptionDesc *selectedArg) {      \
	using namespace Xenon::ArgumentParser; \
	OPTION_LIST_MACRO_NAME(XE_ARG_PARSE_OPTIONS_DEF_DO_PARSE_SHORT)      \
	/* implicit else: */ {      \
		return false;      \
	}      \
	return true;      \
}      \
void OPTIONS_CLASS_NAME##_Parser::_opt_checkArguments (char **_opt_argv, uint32_t _opt_numPositionalArgs, \
		uint16_t *_opt_positionalArgs, const Xenon::ArgumentParser::AppInformation &appInfo) \
{      \
	using namespace Xenon::ArgumentParser; \
	unsigned int _opt_nextPositionalArg = 0; \
	OPTION_LIST_MACRO_NAME(XE_ARG_PARSE_OPTIONS_POSITIONAL_ARGUMENTS)      \
	if ((_opt_nextPositionalArg < _opt_numPositionalArgs) && !(appInfo.programOptions & Xenon::ArgumentParser::IgnoreUnknown))  \
		throw ArgumentParserError(std::string("Too many positional arguments"));    \
	OPTION_LIST_MACRO_NAME(XE_ARG_PARSE_OPTIONS_CHECK_ARGUMENTS)      \
}\
void OPTIONS_CLASS_NAME##_Parser::_opt_enumerateGroups (const Xenon::ArgumentParser::OptionGroup **&_opt_group, unsigned int _opt_maxGroups) {\
	using namespace Xenon::ArgumentParser; \
	unsigned int _opt_nGroups = 0; \
	*_opt_group = NULL; \
	const OptionGroup *_opt_prev = NULL; \
	OPTION_LIST_MACRO_NAME(XE_ARG_VISIT_GROUPS)      \
}

#define XE_DECLARE_OPTIONS_GROUP(GROUP_NAME, GROUP_DESC, GROUP_FLAGS) const Xenon::ArgumentParser::OptionGroup GROUP_NAME (GROUP_DESC, GROUP_FLAGS);

#define XE_DEPEND_ON(OPTION_NAME) dependOn ( (1U << _XE_OPT_DATA::PARAM_##OPTION_NAME) )

#define _OPTIONS_xstr(s) str(s)
#define _OPTIONS_str(s) #s

/// PRIVATE:
#define XE_ARG_PARSE_OPTIONS_DEF_MEMBER(var_name, type, desc, def) type var_name; \
		bool has_##var_name () const { return setParameters & (1U << PARAM_##var_name); }

#define XE_ARG_PARSE_OPTIONS_DEF_FLAG(var_name, type, desc, def) PARAM_##var_name,

#define XE_ARG_PARSE_OPTIONS_DEF_OPERATION(var_name, type, desc, def) _opt_f( (desc).setName(_OPTIONS_str(var_name)), this->var_name, (type const &) (def));

#define XE_ARG_PARSE_OPTIONS_INIT_VAL(var_name, type, desc, def) , var_name(def)

#define XE_ARG_PARSE_OPTIONS_DEF_DO_PARSE(var_name, type, desc, def) \
	if ( strcmp (argName, (desc).setName( _OPTIONS_str(var_name)).name) == 0) { \
		if ( (parseFlags & PARSE_IS_NEXT_ARG) && ((desc).flags & Options_Flag)) \
			argValue = NULL; \
		ParseFunctions::parse ( this->data->var_name, argValue, (desc).setName( _OPTIONS_str(var_name))); \
		this->data->setParameters |= (1U << this->data->PARAM_##var_name); \
		*selectedArg = desc; \
	} else

#define XE_ARG_PARSE_OPTIONS_DEF_DO_PARSE_SHORT(var_name, type, desc, def) \
	if ( arg == (desc).shortOption ) { \
		if (((desc).flags & Options_Flag)) \
			argValue = NULL; \
		ParseFunctions::parse ( this->data->var_name, argValue, (desc).setName( _OPTIONS_str(var_name) ) ); \
		this->data->setParameters |= (1U << this->data->PARAM_##var_name); \
		*selectedArg = desc; \
	} else

#define XE_ARG_PARSE_OPTIONS_POSITIONAL_ARGUMENTS(var_name, type, desc, def) \
	if ((((desc).flags) & Options_Positional) && (!data->has_##var_name() || ((desc).flags & Options_Multiple)) && _opt_nextPositionalArg < _opt_numPositionalArgs) { \
		do { \
			ParseFunctions::parse ( this->data->var_name, _opt_argv[_opt_positionalArgs[_opt_nextPositionalArg++]], (desc).setName( _OPTIONS_str(var_name) ) ); \
		} while ((_opt_nextPositionalArg < _opt_numPositionalArgs) && ((desc).flags & Options_Multiple)); \
		this->data->setParameters |= (1U << this->data->PARAM_##var_name); \
	} \

#define XE_ARG_PARSE_OPTIONS_CHECK_ARGUMENTS(var_name, type, macro_desc, def) \
	{ \
		OptionDesc &odesc = (macro_desc).setName( _OPTIONS_str(var_name) ); \
		if (((odesc.flags) & Options_Required) && !data->has_##var_name()) { \
			throw RequiredArgumentMissing( _OPTIONS_str(var_name) ); \
		} \
		if ( data->has_##var_name() && (odesc.depends_on & ~this->data->setParameters )) { \
			throw ArgumentParserError ( std::string("OptionsParser: Option '") + std::string(odesc.name) + "' depends on options that are not given"); \
		} \
	}
	
#define XE_ARG_VISIT_GROUPS(var_name, type, macro_desc, def) \
	{ \
		const OptionGroup *g = (macro_desc).assignedGroup; \
		if (g && g != _opt_prev) {\
			if (++_opt_nGroups > _opt_maxGroups)\
				throw std::logic_error ("ArgumentParser: Too many OptionGroups. Only 32 are allowed"); \
			*(_opt_group++) = _opt_prev = g; \
		} \
	}

}
}
