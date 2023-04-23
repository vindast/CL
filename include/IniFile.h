#pragma once
#include <fstream>
#include <map>
#include "FilesSystem.h" 
 
namespace CL
{  
	enum class INI_FILE_VALUE_TYPE
	{
		bad_value = 0,
		int_value = 1,
		float_value = 2
	};

	class IniFileOutput
	{
	public:
		IniFileOutput(const std::string& sFile);
		void write(const std::string& sName, int i, const std::string& sComment);
		void write(const std::string& sName, int i);
		void write(const std::string& sName, float f, const std::string& sComment);
		void write(const std::string& sName, float f);
		void close();
	private:
		std::ofstream _file;
	};

	class IniFileInput
	{
	public:
		IniFileInput(const std::string& sFile);
		//������ int �������� � ������ sName, ���� �� �������� int ��� �� �� ����������, ������ iDefaultValue
		int readIntValue(const std::string& sName, int iDefaultValue);
		//������ float �������� � ������ sName, ���� �� �������� float ��� �� �� ����������, ������ fDefaultValue
		float readFloatValue(const std::string& sName, float fDefaultValue);
	private:
		std::ifstream _file;

		std::map<std::string, int> _iMap;
		std::map<std::string, float> _fMap;

		std::string parseValue(std::string sSourse, INI_FILE_VALUE_TYPE& type); 
	}; 
};
