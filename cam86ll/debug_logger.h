#pragma once
#include <iostream>
#include <fstream>


class Debug_Logger
{
public:
	std::ofstream log_file;
	std::string file_name;
	bool clean_file_on_load;
	int tab_positions;
	int tab_length;
	int level_spacing;

	bool logger_started = false;
	bool logger_failed = false;

	enum indent_level {
		kLevel_0 = 0,
		kLevel_1 = 1,
		kLevel_2 = 2,
		kLevel_3 = 3
	};

	enum line_mode {
		kNew_Line_Disable = 0,
		kNew_Line_Before = 1,
		kNew_Line_After = 2,
		kNew_Line_Before_After = 3
	};

	Debug_Logger(const char* file_name, bool clean, int tabs, int spaces, int indent_spacing);
	~Debug_Logger();
	void Start_Logger();
	bool Add_Message(const char* location, const char* message, indent_level level, line_mode new_line_mode);
	bool Add_Code(const char* location, int code, indent_level level, line_mode new_line_mode);
	void Clean();
};