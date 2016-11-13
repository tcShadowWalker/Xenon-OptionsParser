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
	unsigned int flags;
	char shortOption;
	
	OptionDesc (const char *desc, unsigned int flags = 0, char shortOpt = 0)
		: name(NULL), description(desc), flags(flags), shortOption(shortOpt) { }
	
	OptionDesc setName (const char *name) { OptionDesc d = *this; d.name = name; return d; }
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
	void _opt_parse_arg ( std::string &p, const char *argValue, const OptionDesc &desc );
	void _opt_parse_arg ( int32_t &p, const char *argValue, const OptionDesc &desc );
	void _opt_parse_arg ( int64_t &p, const char *argValue, const OptionDesc &desc );
	void _opt_parse_arg ( float &p, const char *argValue, const OptionDesc &desc );
	void _opt_parse_arg ( bool &p, const char *argValue, const OptionDesc &desc );
	
#ifndef ARGUMENT_PARSER_NO_STL_SUPPORT
	template<class T> void _opt_parse_arg ( std::vector<T> &p, const char *argValue, const OptionDesc &desc ) {
		T val;
		_opt_parse_arg (val, argValue, desc);
		p.push_back(val);
	}
#endif
	
	struct HelpPrinter {
		void operator() (const char *argName, const OptionDesc &desc, const std::string &, const std::string &defVal);
		void operator() (const char *argName, const OptionDesc &desc, int32_t, int32_t defVal);
		void operator() (const char *argName, const OptionDesc &desc, int64_t, int64_t defVal);
		void operator() (const char *argName, const OptionDesc &desc, float, float defVal);
		void operator() (const char *argName, const OptionDesc &desc, bool, bool defVal);
		std::ostream &out;
		const AppInformation &appInfo;
		bool full;
		
#ifndef ARGUMENT_PARSER_NO_STL_SUPPORT
		template<class T> void operator() (const char *argName, const OptionDesc &desc, const std::vector<T> &, const std::vector<T> &) {
			const T val;
			(*this) (argName, desc, val, val);
		}
#endif
		HelpPrinter (std::ostream &o, const AppInformation & ai, bool f) : out(o), appInfo(ai), full(f) { }
	};
	
	enum ParseResult {
		PARSE_OK = 1,
		/// --help or --version was given; Action was taken. App should terminate.
		PARSE_TERMINATE = 2,
	};
	
	ParseResult parse (int argc, char **argv, const AppInformation &appInfos);
	virtual void printHelp (std::ostream &out, bool full, const AppInformation &appInfo) = 0;
	
	OptionParserBase () : setParameters(0) { }
protected:
	uint64_t setParameters;
	
	void printHelpHead (std::ostream &out, const AppInformation &appInfos);
	
	virtual bool _opt_parseLongArgument (const char *argName, const char *argValue, OptionDesc *selectedArg) = 0;
	virtual bool _opt_parseShortArgument (char arg, const char *argValue, OptionDesc *selectedArg) = 0;
	virtual void _opt_checkArguments (char **argv, uint32_t numPositionalArgs, uint16_t *positionalArgs, const AppInformation &appInfo) = 0;
};

/**
 * @brief Main macro to define list of program options
 */
#define XE_DECLARE_PROGRAM_OPTIONS(OPTIONS_CLASS_NAME, OPTION_LIST_MACRO_NAME)      \
struct OPTIONS_CLASS_NAME : public Xenon::ArgumentParser::OptionParserBase     \
{     \
	typedef Xenon::ArgumentParser::OptionDesc OptionDesc; \
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
	void printHelp (std::ostream &out, bool full, const Xenon::ArgumentParser::AppInformation &appInfo) {     \
		using namespace Xenon::ArgumentParser; \
		printHelpHead(out, appInfo);     \
		HelpPrinter printer (out, appInfo, full); \
		for_each_option(printer);     \
		if (appInfo.programHelpTextTail) \
			out << appInfo.programHelpTextTail; \
		out << std::endl; \
	}     \
	\
	OPTIONS_CLASS_NAME ()     \
		: OptionParserBase() OPTION_LIST_MACRO_NAME(XE_ARG_PARSE_OPTIONS_INIT_VAL)   {}     \
protected:     \
	bool _opt_parseLongArgument (const char *argName, const char *argValue, OptionDesc *selectedArg);     \
	bool _opt_parseShortArgument (char arg, const char *argValue, OptionDesc *selectedArg);     \
	void _opt_checkArguments (char **argv, uint32_t numPositionalArgs, uint16_t *positionalArgs, const Xenon::ArgumentParser::AppInformation &);     \
};

/**
 * @brief Provide parser implementation code for program options
 */
#define XE_DEFINE_PROGRAM_OPTIONS_IMPL(OPTIONS_CLASS_NAME, OPTION_LIST_MACRO_NAME)     \
bool OPTIONS_CLASS_NAME::_opt_parseLongArgument (const char *argName, const char *argValue, OptionDesc *selectedArg) {      \
	using namespace Xenon::ArgumentParser; \
	OPTION_LIST_MACRO_NAME(XE_ARG_PARSE_OPTIONS_DEF_DO_PARSE)      \
	/* implicit else: */ {      \
		return false;      \
	}      \
	return true;      \
}      \
bool OPTIONS_CLASS_NAME::_opt_parseShortArgument (char arg, const char *argValue, OptionDesc *selectedArg) {      \
	using namespace Xenon::ArgumentParser; \
	OPTION_LIST_MACRO_NAME(XE_ARG_PARSE_OPTIONS_DEF_DO_PARSE_SHORT)      \
	/* implicit else: */ {      \
		return false;      \
	}      \
	return true;      \
}      \
void OPTIONS_CLASS_NAME::_opt_checkArguments (char **_opt_argv, uint32_t _opt_numPositionalArgs, \
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

#define XE_ARG_PARSE_OPTIONS_DEF_OPERATION(name, type, desc, def) _opt_f( _OPTIONS_str(name), desc, this->name, def);

#define XE_ARG_PARSE_OPTIONS_INIT_VAL(name, type, desc, def) , name(def)

#define XE_ARG_PARSE_OPTIONS_DEF_DO_PARSE(name, type, desc, def) if ( strcmp (argName, _OPTIONS_str(name)) == 0) { \
		this->_opt_parse_arg ( this->name, argValue, desc.setName( _OPTIONS_str(name) ) ); \
		this->setParameters |= (1U << PARAM_##name); \
		*selectedArg = desc; \
	} else

#define XE_ARG_PARSE_OPTIONS_DEF_DO_PARSE_SHORT(name, type, desc, def) if ( arg == desc.shortOption ) { \
		this->_opt_parse_arg ( this->name, argValue, desc.setName( _OPTIONS_str(name) ) ); \
		this->setParameters |= (1U << PARAM_##name); \
		*selectedArg = desc; \
	} else

#define XE_ARG_PARSE_OPTIONS_CHECK_ARGUMENTS(name, type, desc, def) \
	if (((desc.flags) & Options_Positional) && (!has_##name() || (desc.flags & Options_Multiple)) && _opt_nextPositionalArg < _opt_numPositionalArgs) { \
		do { \
			this->_opt_parse_arg ( this->name, _opt_argv[_opt_positionalArgs[_opt_nextPositionalArg++]], desc.setName( _OPTIONS_str(name) ) ); \
		} while ((_opt_nextPositionalArg < _opt_numPositionalArgs) && (desc.flags & Options_Multiple)); \
		this->setParameters |= (1U << PARAM_##name); \
	} \
	if (((desc.flags) & Options_Required) && !has_##name()) { \
		throw RequiredArgumentMissing( _OPTIONS_str(name) ); }


}
}
