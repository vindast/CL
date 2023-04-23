#pragma once 
#include <CLObjects/CLCriticalSection.h>
#include <stdio.h>
#include <fstream> 
#include <chrono>
#include <ctime>  
#include <string>
#include <stdarg.h>

#ifndef CL_LOG_FOLDER
#define CL_LOG_FOLDER "Logs"
#endif // !CL_LOG_FOLDER

namespace CL
{
	class Logger
	{
	public:
		static void write(const std::string& message);
		static void writeSuccess(const std::string& message);
		static void writeTry(const std::string& message);
		static void writeCriticalError(const std::string& message);
		static std::string getDate();
		static void write(const char* str);
		static void pushMessageFormated(const char* fmt, ...);
		void Write(const std::string& message);
		void WriteSuccess(const std::string& message);
		void WriteTry(const std::string& message);
		void WriteCriticalError(const std::string& message);
		void Write(const char* str);
		void PushMessageFormated(const char* fmt, ...);
		Logger(const char* Name);
		~Logger() = default;
	private:
		std::string _FileName; 
		std::ofstream _File;
		CriticalSection _hCS;

		Logger();
		static Logger& getInstance();

		Logger(const Logger&) = delete;
		Logger& operator = (const Logger&) = delete;
	};
};

