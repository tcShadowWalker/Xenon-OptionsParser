#pragma once
#ifndef ARGUMENT_PARSER_NO_STL_SUPPORT
	#include <vector>
#endif
#include <stdexcept>
#include <string>
#include <stdint.h>

namespace Xenon {
namespace ArgumentParser {

enum OptionsFlags {
	Options_None             = 0,
	/// Do not print this parameter when --help is given.
	Options_Hidden           = 1U << 0,
	/// Positional parameter can be given without the argument name or short-flag
	Options_Positional       = 1U << 1,
	/// This parameter requires no argument value, but serves only as a flag. Must be of type bool.
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

struct OptionDesc {
	const char *name;
	const char *description;
	const char * const *enumeration_values;
	const char *depend_on;
	unsigned int flags;
	char shortOption;
	
	OptionDesc (const char *desc, unsigned int flags = 0, char shortOpt = 0)
		: name(NULL), description(desc), enumeration_values(NULL), depend_on(NULL), flags(flags), shortOption(shortOpt) { }
	
	OptionDesc &setName (const char *name) { if (!this->name) { this->name = name; } return *this; }
	OptionDesc &setEnum (const char * const * const enum_values) {enumeration_values = enum_values; return *this; }
	OptionDesc &dependOn (const char *name) { depend_on = name; return *this; }
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
	const char *programName, *programVersion, *programHelpTextHeader, *programHelpTextTail;
	std::ostream *helpOutputStream; ///< If NULL; default is std::cout
	
	AppInformation (const char *appName, const char *version, unsigned int programOptions = 0)
		: programOptions(programOptions), programName(appName), programVersion(version), programHelpTextHeader(NULL), helpOutputStream(NULL) { }
	
	void setHelpText (const char *head, const char *tail = NULL) { programHelpTextHeader = head; programHelpTextTail = tail; }
};

struct OptionParserBase
{
	struct HelpPrinter {
		template<class T> void operator() (const char *argName, const OptionDesc &desc, const T &v, const T &d);
		
		HelpPrinter (std::ostream &o, const AppInformation & ai, bool f) : out(o), appInfo(ai), full(f) { }
		std::ostream &out;
		const AppInformation &appInfo;
		bool full;
	};
	
	enum ParseResult {
		PARSE_OK = 1,
		/// --help or --version was given; Action was taken. App should terminate.
		PARSE_TERMINATE = 2,
	};
	
	virtual void printHelp (std::ostream &out, bool full, const AppInformation &appInfo) = 0;
protected:
	void printHelpHead (std::ostream &out, const AppInformation &appInfos);
	
	ParseResult parse (int argc, char **argv, const AppInformation &appInfos);
	virtual bool _opt_parseLongArgument (const char *argName, const char *argValue, OptionDesc *selectedArg) = 0;
	virtual bool _opt_parseShortArgument (char arg, const char *argValue, OptionDesc *selectedArg) = 0;
	virtual void _opt_checkArguments (char **argv, uint32_t numPositionalArgs, uint16_t *positionalArgs, const AppInformation &appInfo) = 0;
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

	void print_help (const OHP &, const char *argName, const OptionDesc &desc, const std::string &, const std::string &defVal);
	void print_help (const OHP &, const char *argName, const OptionDesc &desc, const char* &, const char* &defVal);
	void print_help (const OHP &, const char *argName, const OptionDesc &desc, int32_t, int32_t defVal);
	void print_help (const OHP &, const char *argName, const OptionDesc &desc, int64_t, int64_t defVal);
	void print_help (const OHP &, const char *argName, const OptionDesc &desc, float, float defVal);
	void print_help (const OHP &, const char *argName, const OptionDesc &desc, bool, bool defVal);
	
#ifndef ARGUMENT_PARSER_NO_STL_SUPPORT
	template<class T, class Alloc>
	void print_help (const OHP &hp, const char *argName, const OptionDesc &desc, const std::vector<T, Alloc> &, const std::vector<T, Alloc> &) {
		const T val;
		print_help (hp, argName, desc, val, val);
	}
#endif

};

template<class T>
void OptionParserBase::HelpPrinter::operator() (const char *argName, const OptionDesc &desc, const T &v, const T &d) {
	ParseFunctions::print_help (*this, argName, desc, v, d);
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
}; \
\
struct OPTIONS_CLASS_NAME##_Parser : public Xenon::ArgumentParser::OptionParserBase, public Xenon::ArgumentParser::AppInformation { \
	\
	typedef Xenon::ArgumentParser::OptionDesc OptionDesc; \
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
	bool _opt_parseLongArgument (const char *argName, const char *argValue, OptionDesc *selectedArg);     \
	bool _opt_parseShortArgument (char arg, const char *argValue, OptionDesc *selectedArg);     \
	void _opt_checkArguments (char **argv, uint32_t numPositionalArgs, uint16_t *positionalArgs, const Xenon::ArgumentParser::AppInformation &);     \
};

/**
 * @brief Provide parser implementation code for program options
 */
#define XE_DEFINE_PROGRAM_OPTIONS_IMPL(OPTIONS_CLASS_NAME, OPTION_LIST_MACRO_NAME)     \
bool OPTIONS_CLASS_NAME##_Parser::_opt_parseLongArgument (const char *argName, const char *argValue, OptionDesc *selectedArg) {      \
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
	OPTION_LIST_MACRO_NAME(XE_ARG_PARSE_OPTIONS_CHECK_ARGUMENTS)      \
	if ((_opt_nextPositionalArg < _opt_numPositionalArgs) && !(appInfo.programOptions & Xenon::ArgumentParser::IgnoreUnknown))  \
		throw ArgumentParserError(std::string("Too many positional arguments"));    \
}

#define _OPTIONS_xstr(s) str(s)
#define _OPTIONS_str(s) #s

/// PUBLIC:
#define OPTIONS_DEF_GROUP(group_name)

/// PRIVATE:
#define XE_ARG_PARSE_OPTIONS_DEF_MEMBER(name, type, desc, def) type name; \
		bool has_##name () const { return setParameters & (1U << PARAM_##name); }

#define XE_ARG_PARSE_OPTIONS_DEF_FLAG(name, type, desc, def) PARAM_##name,

#define XE_ARG_PARSE_OPTIONS_DEF_OPERATION(name, type, desc, def) _opt_f( _OPTIONS_str(name), desc, this->name, (type const &) (def));

#define XE_ARG_PARSE_OPTIONS_INIT_VAL(name, type, desc, def) , name(def)

#define XE_ARG_PARSE_OPTIONS_DEF_DO_PARSE(name, type, desc, def) if ( strcmp (argName, _OPTIONS_str(name)) == 0) { \
		ParseFunctions::parse ( this->data->name, argValue, (desc).setName( _OPTIONS_str(name) ) ); \
		this->data->setParameters |= (1U << this->data->PARAM_##name); \
		*selectedArg = desc; \
	} else

#define XE_ARG_PARSE_OPTIONS_DEF_DO_PARSE_SHORT(name, type, desc, def) if ( arg == (desc).shortOption ) { \
		ParseFunctions::parse ( this->data->name, argValue, (desc).setName( _OPTIONS_str(name) ) ); \
		this->data->setParameters |= (1U << this->data->PARAM_##name); \
		*selectedArg = desc; \
	} else

#define XE_ARG_PARSE_OPTIONS_CHECK_ARGUMENTS(name, type, desc, def) \
	if ((((desc).flags) & Options_Positional) && (!data->has_##name() || ((desc).flags & Options_Multiple)) && _opt_nextPositionalArg < _opt_numPositionalArgs) { \
		do { \
			ParseFunctions::parse ( this->data->name, _opt_argv[_opt_positionalArgs[_opt_nextPositionalArg++]], (desc).setName( _OPTIONS_str(name) ) ); \
		} while ((_opt_nextPositionalArg < _opt_numPositionalArgs) && ((desc).flags & Options_Multiple)); \
		this->data->setParameters |= (1U << this->data->PARAM_##name); \
	} \
	if ((((desc).flags) & Options_Required) && !data->has_##name()) { \
		throw RequiredArgumentMissing( _OPTIONS_str(name) ); }


}
}
