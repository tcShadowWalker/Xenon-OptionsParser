#pragma once

enum OptionsFlags {
	Options_None             = 0,
	Options_Hidden           = 1U << 0,
	Options_Positional       = 1U << 1,
	Options_Flag             = 1U << 2,
	Options_Required         = 1U << 3,
};

enum OptionParserFlags {
	/// Do not generate help text
	OptionParser_NoHelp      = 1U << 0,
	/// Do not generate behaviour for --version option
	OptionParser_NoVersion   = 1U << 1,
	/// Don't generate full-help showing hidden options
	OptionParser_HideHidden  = 1U << 1,
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

#define _OPTIONS_xstr(s) str(s)
#define _OPTIONS_str(s) #s

#define OPTIONS_DEF_MEMBER(name, type, desc, def) type name;  static const type default_##name; \
		bool has_##name () const { return _opt_setParameters & (1U << PARAM_##name); }

#define OPTIONS_DEF_FLAG(name, type, desc, def) PARAM_##name,

#define OPTIONS_DEF_OPERATION(name, type, desc, def) _opt_f( _OPTIONS_str(name), desc, this->name, def);

#define OPTIONS_DEF_DEFAULT(name, type, desc, def) const type Options::default_##name (def);

#define OPTIONS_INIT_VAL(name, type, desc, def) , name(def)

#define OPTIONS_DEF_DO_PARSE(name, type, desc, def) if ( strcmp (argName, _OPTIONS_str(name)) == 0) { \
		this->_opt_parse_arg ( this->name, argValue, desc.setName( _OPTIONS_str(name) ) ); \
	} else

#define OPTIONS_CHECK_REQUIRED_GIVEN(name, type, desc, def) if (((desc.flags) & Options_Required) && !has_##name()) {\
		throw RequiredArgumentMissing( _OPTIONS_str(name) ); }


struct RequiredArgumentMissing : public std::exception
{
	RequiredArgumentMissing (const char *arg) : arg(arg) { s = "Missing required argument '"; s+= arg; s += "'"; }
	const char *what () const noexcept { return s.c_str(); }
	
	const char *arg;
	std::string s;
};

struct OptionParserBase
{
	void _opt_parse_arg ( std::string &p, const char *argValue, const OptionDesc &desc );
	void _opt_parse_arg ( int &p, const char *argValue, const OptionDesc &desc );
	void _opt_parse_arg ( float &p, const char *argValue, const OptionDesc &desc );
	void _opt_parse_arg ( bool &p, const char *argValue, const OptionDesc &desc );
	
	struct HelpPrinter {
		void operator() (const char *argName, const OptionDesc &desc, const std::string &, const std::string &defVal);
		void operator() (const char *argName, const OptionDesc &desc, int, int defVal);
		void operator() (const char *argName, const OptionDesc &desc, float, float defVal);
		void operator() (const char *argName, const OptionDesc &desc, bool, bool defVal);
		std::ostream &out;
	};
	
	void parse (int argc, char **argv);
	virtual void printHelp (std::ostream &out) = 0;
	
	OptionParserBase (const char *appName, const char *version, unsigned int programOptions = 0)
		: programOptions(programOptions), programName(appName), programVersion(version), programHelpTextHeader(NULL) { }
	
	void setHelpText (const char *head) { programHelpTextHeader = head; }
protected:
	unsigned int programOptions;
	const char *programName, *programVersion, *programHelpTextHeader;
	
	void printHelpHead (std::ostream &out);
	
	virtual bool parseLongArgument (const char *argName, const char *argValue) = 0;
	virtual void checkArguments () = 0;
};

/**
 * @brief Main macro to define list of program options
 */
#define DECLARE_PROGRAM_OPTIONS(OptionsClassName, OPTION_LIST_MACRO_NAME)

/**
 * @brief Provide parser implementation code for program options
 */
#define DEFINE_PROGRAM_OPTIONS_IMPL(OptionsClassName, OPTION_LIST_MACRO_NAME)

