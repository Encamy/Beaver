#include "Logger.h"
#include <cstdio>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
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
	auto now = std::chrono::system_clock::now();
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
	auto timer = std::chrono::system_clock::to_time_t(now);
	struct std::tm bt;
	localtime_s(&bt, &timer);

	std::ostringstream ss;

	ss << std::put_time(&bt, "%T");
	ss << '.' << std::setfill('0') << std::setw(3) << ms.count();

	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	if (log_level == "ERROR")
	{
		printf("%s [", ss.str().c_str());
		SetConsoleTextAttribute(hConsole, 12);
		printf("%s", log_level.c_str());
		SetConsoleTextAttribute(hConsole, 7);
		printf("][%s] % s\n", caller, input.c_str());
	}
	else 
	if (log_level == "FATAL")
	{
		printf("%s [", ss.str().c_str());
		SetConsoleTextAttribute(hConsole, 79);
		printf("%s", log_level.c_str());
		SetConsoleTextAttribute(hConsole, 7);
		printf("][%s] % s\n", caller, input.c_str());
	}
	else
	{
		printf("%s [%s][%s] %s\n", ss.str().c_str(), log_level.c_str(), caller, input.c_str());
	}
	if (use_file)
	{
		log_file.open(log_filepath, std::ios::app);
		std::string result_str;
		log_file << ss.str() << "[" << log_level << "][" << caller << "]" << input << "\n";
		log_file.close();
	}
}


void Logger::LOG_FATAL(std::string input, const char *caller)
{
	__log__.LOG(input, "FATAL", caller);
}

void Logger::LOG_ERROR(std::string input, const char *caller)
{
	__log__.LOG(input, "ERROR", caller);
}

void Logger::LOG_WARNING(std::string input, const char *caller)
{
	__log__.LOG(input, "WARN ", caller);
}

void Logger::LOG_INFO(std::string input, const char *caller)
{
	__log__.LOG(input, "INFO ", caller);
}

void Logger::LOG_DEBUG(std::string input, const char *caller)
{
	__log__.LOG(input, "DEBUG", caller);
}

void Logger::LOG_TRACE(std::string input, const char *caller)
{
	__log__.LOG(input, "TRACE", caller);
}
