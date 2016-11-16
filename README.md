# Xenon-OptionsParser
Simple C++ ProgramOptions parser; without a separate pre-compilation code-generation step.

This library is similar to GNU Gengetopt or boost::program_options.
However, unlike gengetopt, no dedicated pre-processing tool is required to run before your compilation step.
Instead, the C preprocessor is used to generate the struct holding the options data, as well as the parser to fill it.

Unlike boost::program_options, a real C++ structure is generated that provides type-safe access to options,
whereas boost uses runtime strings to access options. If an argument name is misspelled, compilation would succeed, but result in a runtime error.

```c++
const char *IndentationValues[] = { "tabs", "spaces", "none", 0 };
/// Define a macro with a list of all your options: To define an option, use the provided macro argument
/// DEF( attribute_name, type, description, default_value)   the description object can hold additional information (flags, a short flag, grouping and dependency information)
#define CREATE_MY_OPTIONLIST(DEF) \
	\
	DEF(file, std::string, OptionDesc("Path to a file", Options_Required, 'f'), "")  \
	DEF(long_option, std::string, (OptionDesc("Description for long option", Options_None).setName("long-option")), "") \
	\
	DEF(indent, const char *, (OptionDesc("Character used for indentation", Options_None).setEnum( IndentationValues )), "tabs") \
	DEF(verbosity, int, OptionDesc("Log verbosity level (Value between 0 and 20)", Options_None, 'v'), 6) \
	DEF(delete_files, bool, (OptionDesc("Delete files", Options_Flag, 'd')), false) \
	DEF(print, bool, OptionDesc("Print all assigned values", Options_Flag, 'p'), false) 

/// This macro generates the structure definition 'MyOptions' that will hold all specified attributes
XE_DECLARE_PROGRAM_OPTIONS(MyOptions, CREATE_MY_OPTIONLIST);
/// Generate the parser implementation to fill a 'MyOptions' struct from an array of command-line options.
XE_DEFINE_PROGRAM_OPTIONS_IMPL(MyOptions, CREATE_MY_OPTIONLIST);

int main(int argc, char **argv) {
	MyOptions opt;
	MyOptions::Parser parser ("ExampleOptionParser", "0.1");
	parser.parse (opt, argc, argv)

	if (opt.print) { ... }

	if (opt.verbosity > 10) { ... }

	if (opt.has_long_option())
		std::cout << "Given long option is: " << opt.long_option << std::endl;
}
```
The generated structure definition would look like this:
```c++
struct MyOptions {
	std::string file;
	std::string long_option;
	const char *indent;
	int verbosity;
	bool delete_files;
	bool print;
	
	bool has_long_option () const;
	bool has_indent () const;
	bool has_verbosity () const;
	bool has_delete_files () const;
	bool has_print () const;
	/* std::uint64_t   used as a bitmask about which options are set, used by the has_* methods */;
};
```
See [a simple example](example_main.cpp) as a short introduction.

Some features this OptionParser provides:
- Generating a standard help page.
- Parsing of positional arguments
- Support for default arguments
- Short-forms for options. Multiple short options can be set with one command-line argument (e.g.: '-xvz')
- Required options (that must be given, or parsing will fail)
- Enumerations: Options that can only be set to one object from a predefined set of values.
- Dependencies between options: If A is given, B must be given as well.
- Multiple occurences of a single option (for example, multiple filenames, aggregated into an std::vector)
- OptionGroups: Provide sub-headings and logical grouping for the help page
- OptionGroups can be set to be mandatory
- All options within an OptionGroup may be set to be mutually exclusive: Then only one of those options can be given.
- Toggle switches (Options_Flag) with optional arguments
- Hidden options, only visible with --full-help (or not at all, if generation of --full-help is suppressed)
- Integrated support for options of these types: std::string, const char *, int, float, bool and vectors of any defined type.
- Extensibility for custom types: If you need custom times (for example, dates or timestamps)
  you can simple extend the namespace containing the parsing routines, and then define your options with your own, new types.

Example of a generated help page:
```
Usage: ExampleOptionParser - A simple program options parser example.

Main options:
   --long_option            Description for long option (default: "")

Extended options:
   --print                  Print all assigned values (default: 0)
   -v, --verbosity          Log verbosity level (default: 6)
   --indent                 Character used for indentation 
                            (default: "tabs"; values: "tabs", "spaces", "none")
   -d, --delete             Delete all files after operation is done (default: 0)
```

The headings ("Extended options", "Main options") are generated through OptionGroups.


  
