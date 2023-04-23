#include "Logger.h"
#include <FilesSystem.h>

CL::Logger& CL::Logger::getInstance()
{
	static Logger loger;
	return loger;
}

void CL::Logger::write(const std::string& message)
{
	CL_MESSAGE_RUNTIME(message.c_str());
	getInstance().Write(message);
}

void CL::Logger::writeSuccess(const std::string& message)
{
	CL_MESSAGE_RUNTIME(message.c_str());
	getInstance().WriteSuccess(message);
}

void CL::Logger::writeTry(const std::string& message)
{
	CL_MESSAGE_RUNTIME(message.c_str());
	getInstance().WriteTry(message);
}

void CL::Logger::writeCriticalError(const std::string& message)
{
	CL_MESSAGE_RUNTIME(message.c_str());
	getInstance().WriteCriticalError(message);
}

std::string CL::Logger::getDate()
{
	std::string data;

	std::time_t t = std::time(0);   // get time now
	std::tm* now = std::localtime(&t);

	data += std::to_string(now->tm_year + 1900) + ".";
	data += std::to_string(now->tm_mon + 1) + ".";
	data += std::to_string(now->tm_mday) + " : ";
	data += std::to_string(now->tm_hour) + "h ";
	data += std::to_string(now->tm_min) + "m ";
	data += std::to_string(now->tm_sec) + "s ";

	return data;
}

void CL::Logger::write(const char* str)
{
	getInstance().Write(str);
}

void CL::Logger::pushMessageFormated(const char* fmt, ...)
{
	const size_t buf_size = 2048;
	char buf[buf_size];

	va_list args;
	va_start(args, fmt);

	int w = vsnprintf(buf, buf_size, fmt, args);


	if (w == -1 || w >= (int)buf_size)
		w = (int)buf_size - 1;
	buf[w] = 0;

	va_end(args);

	Logger::getInstance().Write(buf);
}

void CL::Logger::Write(const std::string& message)
{
	CL_SCOPE_LOCK_GUARD(_hCS);
	CL_MESSAGE_RUNTIME(message.c_str());
	_File << getDate() << "|" << message << std::endl;
}

void CL::Logger::WriteSuccess(const std::string& message)
{
	CL_SCOPE_LOCK_GUARD(_hCS);
	CL_MESSAGE_RUNTIME(message.c_str());
	_File << getDate() << "|" << message << ": óñïåøíî" << std::endl;
}

void CL::Logger::WriteTry(const std::string& message)
{
	CL_SCOPE_LOCK_GUARD(_hCS);
	CL_MESSAGE_RUNTIME(message.c_str());
	_File << getDate() << "|" << message << "..." << std::endl;
}

void CL::Logger::WriteCriticalError(const std::string& message)
{
	CL_SCOPE_LOCK_GUARD(_hCS);
	CL_MESSAGE_RUNTIME(message.c_str());
	_File << getDate() << "| ÊÐÈÒÈ×ÅÑÊÀß ÎØÈÁÊÀ: " << message << std::endl;
}

void CL::Logger::Write(const char* str)
{
	CL_SCOPE_LOCK_GUARD(_hCS);
	CL_MESSAGE_RUNTIME(str);
	_File << getDate() << "|" << str << std::endl;
}

void CL::Logger::PushMessageFormated(const char* fmt, ...)
{
	const size_t buf_size = 2048;
	char buf[buf_size];

	va_list args;
	va_start(args, fmt);

	int w = vsnprintf(buf, buf_size, fmt, args);


	if (w == -1 || w >= (int)buf_size)
		w = (int)buf_size - 1;
	buf[w] = 0;

	va_end(args);

	Write(buf);
}

CL::Logger::Logger(const char* Name)
{
	std::string date;

	std::time_t t = std::time(0);   // get time now
	std::tm* now = std::localtime(&t);

	date += std::to_string(now->tm_year + 1900) + "_";
	date += std::to_string(now->tm_mon + 1) + "_";
	date += std::to_string(now->tm_mday) + "_";
	date += std::to_string(now->tm_hour) + "_";
	date += std::to_string(now->tm_min) + "_";
	date += std::to_string(now->tm_sec);

	CreateFolders(CL_LOG_FOLDER);

	_FileName = std::string(Name) + "_" + date + std::string(".txt");
	_File.open(std::string(CL_LOG_FOLDER) + "/" + _FileName, std::ofstream::app);

	CL_ASSERT(_File.is_open());
}

CL::Logger::Logger()
{
	std::string date;

	std::time_t t = std::time(0);   // get time now
	std::tm* now = std::localtime(&t);
	 
	date += std::to_string(now->tm_year + 1900) + "_";
	date += std::to_string(now->tm_mon + 1) + "_";
	date += std::to_string(now->tm_mday) + "_";
	date += std::to_string(now->tm_hour) + "_";
	date += std::to_string(now->tm_min) + "_";
	date += std::to_string(now->tm_sec);
	 
	CreateFolders(CL_LOG_FOLDER);

	_FileName = std::string("log_") + date + std::string(".txt");
	_File.open(std::string(CL_LOG_FOLDER) + "/" + _FileName, std::ofstream::app);

	CL_ASSERT(_File.is_open());
}
