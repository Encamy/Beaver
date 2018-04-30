#include "Logger.h"
#include <cstdio>
#include "boost/date_time.hpp"
#include "Windows.h"

Logger __log__;

Logger::Logger(bool use_file_flag)
{
	use_file = use_file_flag;
	if (use_file_flag)
	{
		log_file.open(log_filepath);
		log_file.close();
	}
}

Logger::Logger()
{
	log_file.open(log_filepath);
	log_file.close();
	use_file = true;
}

Logger::~Logger()
{
}

void Logger::LOG(std::string input, std::string log_level, const char* caller)
{
	boost::posix_time::ptime timeLocal = boost::posix_time::second_clock::local_time();
	int hh = timeLocal.time_of_day().hours();
	int mm = timeLocal.time_of_day().minutes();
	int ss = timeLocal.time_of_day().seconds();
	std::string seconds = std::to_string(ss);
	if (ss < 10)
	{
		seconds = "0" + seconds;
	}
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if (log_level == "ERROR")
	{
		printf("%i:%i:%s [", hh, mm, seconds.c_str());
		SetConsoleTextAttribute(hConsole, 12);
		printf("%s", log_level.c_str());
		SetConsoleTextAttribute(hConsole, 7);
		printf("][%s] % s\n", caller, input.c_str());
	}
	else 
	if (log_level == "FATAL")
	{
		printf("%i:%i:%s [", hh, mm, seconds.c_str());
		SetConsoleTextAttribute(hConsole, 79);
		printf("%s", log_level.c_str());
		SetConsoleTextAttribute(hConsole, 7);
		printf("][%s] % s\n", caller, input.c_str());
	}
	else
	{
		printf("%i:%i:%s [%s][%s] %s\n", hh, mm, seconds.c_str(), log_level.c_str(), caller, input.c_str());
	}
	if (use_file)
	{
		log_file.open(log_filepath, std::ios::app);
		std::string result_str;
		log_file << hh << ":" << mm << ":" << seconds << "[" << log_level << "][" << caller << "]" << input << "\n";
		log_file.close();
	}
}


void Logger::LOG_FATAL(std::string input, const char *caller)
{
	LOG(input, "FATAL", caller);
}

void Logger::LOG_ERROR(std::string input, const char *caller)
{
	LOG(input, "ERROR", caller);
}

void Logger::LOG_WARNING(std::string input, const char *caller)
{
	LOG(input, "WARN ", caller);
}

void Logger::LOG_INFO(std::string input, const char *caller)
{
	LOG(input, "INFO ", caller);
}

void Logger::LOG_DEBUG(std::string input, const char *caller)
{
	LOG(input, "DEBUG", caller);
}

void Logger::LOG_TRACE(std::string input, const char *caller)
{
	LOG(input, "TRACE", caller);
}
