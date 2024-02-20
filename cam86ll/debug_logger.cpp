#include "Debug_Logger.h"

Debug_Logger::Debug_Logger(const char* name, bool clean, int tabs, int spaces, int level_size) {
	file_name = name;
	clean_file_on_load = clean;
	tab_positions = tabs;
	tab_length = spaces;
	level_spacing = level_size;
}

Debug_Logger::~Debug_Logger() {

}

void Debug_Logger::Start_Logger() {
	if (clean_file_on_load) {
		log_file.open(file_name, std::ios::out | std::ios::trunc);
	}
	else {
		log_file.open(file_name, std::ios::out | std::ios::app);
	}

	if (!log_file.is_open()) {
		logger_failed = true;
	}
	else {
		logger_started = true;
		log_file << "---------------------\n";
		log_file << "Debug logger started.\n";
		log_file << "---------------------\n";
		log_file.close();
	}

	return;
}

bool Debug_Logger::Add_Message(const char* location, const char* message, indent_level level, line_mode new_line_mode) {
	if (!logger_started && !logger_failed) {
		Start_Logger();
	}
	
	if (logger_failed) {
		return false;
	}

	log_file.open(file_name, std::ios::out | std::ios::app);
	if (!log_file.is_open()) {
		logger_failed = true;
		return false;
	}
	else {
		if (new_line_mode == kNew_Line_Before || new_line_mode == kNew_Line_Before_After) {
			log_file << "\n";
		}

		int level_spaces = level * level_spacing;
		for (int i = 0; i < level_spaces; i++) {
			log_file << " ";
		}

		log_file << location << ":";

		int tabs = ceil((strlen(location) + 1 + level_spaces) / tab_length);

		if (tabs > tab_positions) {
			log_file << "\t";
		}
		else {
			int tabs_to_add = tab_positions - tabs;
			for (int i = 0; i < tabs_to_add; i++) {
				log_file << "\t";
			}
		}

		log_file << message << "\n";

		if (new_line_mode == kNew_Line_After || new_line_mode == kNew_Line_Before_After) {
			log_file << "\n";
		}

		log_file.close();
	}
	return true;
}

bool Debug_Logger::Add_Code(const char* location, int code, indent_level level, line_mode new_line_mode) {
	if (!logger_started && !logger_failed) {
		Start_Logger();
	}

	if (logger_failed) {
		return false;
	}

	log_file.open(file_name, std::ios::out | std::ios::app);
	if (!log_file.is_open()) {
		logger_failed = true;
		return false;
	}
	else {
		if (new_line_mode == kNew_Line_Before || new_line_mode == kNew_Line_Before_After) {
			log_file << "\n";
		}

		int level_spaces = level * level_spacing;
		for (int i = 0; i < level_spaces; i++) {
			log_file << " ";
		}

		log_file << location << ":";

		int tabs = ceil((strlen(location) + 1 + level_spaces) / tab_length);

		if (tabs > tab_positions) {
			log_file << "\t";
		}
		else {
			int tabs_to_add = tab_positions - tabs;
			for (int i = 0; i < tabs_to_add; i++) {
				log_file << "\t";
			}
		}

		log_file << "Code: " << code << "\n";

		if (new_line_mode == kNew_Line_After || new_line_mode == kNew_Line_Before_After) {
			log_file << "\n";
		}

		log_file.close();
	}
	return true;
}

void Debug_Logger::Clean() {
	if (!logger_started) {
		Start_Logger();
	}

	if (!logger_failed) {
		log_file.open(file_name, std::ios::out | std::ios::trunc);
		if (!log_file.is_open()) {
			logger_failed = true;
		}
		else {
			logger_started = true;
			log_file << "---------------------\n";
			log_file << "Logfile cleaned.\n";
			log_file << "---------------------\n";
			log_file.close();
		}
	}
}