#include <iostream>
#include <string>
#include <fstream>

#ifndef LOGGER_H
#define LOGGER_H

#define log_filepath "log.txt"

class Logger {

public:
	Logger(bool use_file_flag);
	Logger();
	~Logger();

	void LOG_FATAL(std::string input,const char *caller);
	void LOG_ERROR(std::string input, const char *caller);
	void LOG_WARNING(std::string input, const char *caller);
	void LOG_INFO(std::string input, const char *caller);
	void LOG_DEBUG(std::string input, const char *caller);
	void LOG_TRACE(std::string input, const char *caller);

private:
	void LOG(std::string input, std::string log_level, const char *caller);
	std::ofstream log_file;
	bool use_file;
};

extern Logger __log__;

#endif