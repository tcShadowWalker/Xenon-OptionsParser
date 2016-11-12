struct Wanted {
	std::string val;
	static const std::string default_val;
	bool has_val () const { return setParameters & (1U << PARAM_VAL); }
	
	int someInt;
	float myFloat;
	
	template<class F>
	void for_each (F &f) {
		f (val, "val", "my string");
		f (someInt, "someInt", "my int");
		f (myFloat, "myFloat", "my float");
	}
	
	void parse (int argc, const char **argv);
	
	enum Parameters {
		PARAM_VAL,
	};
	std::uint64_t setParameters;
};
