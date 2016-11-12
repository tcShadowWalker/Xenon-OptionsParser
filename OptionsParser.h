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
	
	OptionDesc (const char *name, const char *desc, unsigned int flags = 0, char shortOpt = 0)
		: name(name), description(desc), flags(flags), shortOption(shortOpt) { }
};

#define _OPTIONS_xstr(s) str(s)
#define _OPTIONS_str(s) #s

#define OPTIONS_DEF_MEMBER(name, type, desc, def, flags) type name;  static const type default_##name; \
		bool has_##name () const { return _opt_setParameters & (1U << PARAM_##name); }

#define OPTIONS_DEF_FLAG(name, type, desc, def, flags) PARAM_##name,

#define OPTIONS_DEF_OPERATION(name, type, desc, def, flags) _opt_f( _OPTIONS_str(name), desc, this->name, def, flags);

#define OPTIONS_DEF_DEFAULT(name, type, desc, def, flags) const type Options::default_##name (def);

#define OPTIONS_INIT_VAL(name, type, desc, def, flags) , name(def)

#define OPTIONS_DEF_DO_PARSE(name, type, desc, def, flags) if ( strcmp (_opt_arg_name, _OPTIONS_str(name)) == 0) { \
		_opt_parse_arg ( this->name, argValue, flags, _OPTIONS_str(name) ); \
	} else

#define OPTIONS_CHECK_REQUIRED_GIVEN(name, type, desc, def, flags) if (((flags) & Options_Required) && !has_##name()) {\
		throw RequiredArgumentMissing( _OPTIONS_str(name) ); }

struct RequiredArgumentMissing : public std::exception
{
	RequiredArgumentMissing (const char *arg) : arg(arg) { s = "Missing required argument for '"; s+= arg; s += "'"; }
	const char *what () const noexcept { return s.c_str(); }
	
	const char *arg;
	std::string s;
};

struct OptionParserBase
{
	void _opt_parse_arg ( std::string &p, const char *argValue, unsigned int flags, const char *argName );
	void _opt_parse_arg ( int &p, const char *argValue, unsigned int flags, const char *argName );
	void _opt_parse_arg ( float &p, const char *argValue, unsigned int flags, const char *argName );
	void _opt_parse_arg ( bool &p, const char *argValue, unsigned int flags, const char *argName );
	
	struct HelpPrinter {
		void operator() (const char *argName, const char *desc, const std::string &, const std::string &defVal, unsigned int flags);
		void operator() (const char *argName, const char *desc, int, int defVal, unsigned int flags);
		void operator() (const char *argName, const char *desc, float, float defVal, unsigned int flags);
		void operator() (const char *argName, const char *desc, bool, bool defVal, unsigned int flags);
		std::ostream &out;
	};
	
	OptionParserBase (const char *appName, const char *version, unsigned int programOptions = 0)
		: programOptions(programOptions), programName(appName), programVersion(version), programHelpTextHeader(NULL) { }
	
	void setHelpText (const char *head) { programHelpTextHeader = head; }
protected:
	unsigned int programOptions;
	const char *programName, *programVersion, *programHelpTextHeader;
	
	void printHelpHead (std::ostream &out);
};

