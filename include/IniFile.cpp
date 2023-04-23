#include "IniFile.h"
#include <assert.h>
#include <iostream>
#include <sstream>
#include <iomanip>

CL::IniFileInput::IniFileInput(const std::string& sFile)
{ 
	_file.open(sFile, std::ios::in);

	while (_file.is_open() && !_file.eof())
	{
		std::string line;
		std::stringstream converter;
		std::string sName;

		std::getline(_file, line);

		converter << line;
		converter >> sName;

		size_t nCommentPosition = sName.find_first_of(";#");
		bool bIsName =
			nCommentPosition == std::string::npos &&
			_iMap.find(sName) == _iMap.end() &&
			_fMap.find(sName) == _fMap.end();

		if (bIsName)
		{

			std::string sValue;
			converter >> sValue;

			INI_FILE_VALUE_TYPE type;

			sValue = parseValue(sValue, type);

			switch (type)
			{
			case CL::INI_FILE_VALUE_TYPE::bad_value:
				break;
			case CL::INI_FILE_VALUE_TYPE::int_value:
			{
				int i = std::stoi(sValue);
				_iMap.insert(std::make_pair(sName, i));
			}
			break;
			case CL::INI_FILE_VALUE_TYPE::float_value:
			{
				float f = std::stof(sValue);
				_fMap.insert(std::make_pair(sName, f));
			}
			break;
			default:
				break;
			}

		}
	}
}

int CL::IniFileInput::readIntValue(const std::string& sName, int iDefaultValue)
{
	int i = iDefaultValue;
	auto it = _iMap.find(sName);

	if (it != _iMap.end())
	{
		i = it->second;
	}

	return i;
}

float CL::IniFileInput::readFloatValue(const std::string& sName, float fDefaultValue)
{
	float f = fDefaultValue;
	auto it = _fMap.find(sName);

	if (it != _fMap.end())
	{
		f = it->second;
	}

	return f;
}

std::string CL::IniFileInput::parseValue(std::string sSourse, INI_FILE_VALUE_TYPE& type)
{
	std::string sValue;
	type = INI_FILE_VALUE_TYPE::bad_value;
	size_t nFirstSymbol = sSourse.find_first_not_of(" ");

	if (nFirstSymbol != std::string::npos)
	{
		sSourse = sSourse.substr(nFirstSymbol, sSourse.size());
	}

	size_t nCommentPosition = sSourse.find_first_of(";#");

	if (nCommentPosition != std::string::npos)
	{
		if (nCommentPosition != 0)
		{
			sSourse = sSourse.substr(0, nCommentPosition); 
		}
		else
		{
			sSourse.clear();
		}
	}

	if (sSourse.size())
	{
		sValue = sSourse;

		size_t nPointPosition = sSourse.find('.');
		size_t nMinusPosition = sSourse.find('-');
		bool bHaveMinus = nMinusPosition != std::string::npos;
		bool bHavePoint = nPointPosition != std::string::npos;
		bool bIsNumber = false;
		bool bIsFloat = false;

		if (nMinusPosition == std::string::npos || nMinusPosition == 0)
		{
			std::string s;

			if (bHaveMinus)
			{
				s = sSourse.substr(1, sSourse.size() - 1);
			}
			else
			{
				s = sSourse;
			}

			if (s.size())
			{
				if (bHavePoint)
				{
					std::string A = s.substr(0, nPointPosition);
					std::string B = s.substr(nPointPosition + 1, s.size() - 1);
					bIsNumber = A.find_first_not_of("0123456789") == std::string::npos;
					bIsNumber = B.find_first_not_of("0123456789") == std::string::npos;
				}
				else
				{
					bIsNumber = s.find_first_not_of("0123456789") == std::string::npos;
				}
			}
		}

		bIsFloat = bIsNumber && bHavePoint;

		if (bIsNumber)
		{
			if (bIsFloat)
			{
				type = INI_FILE_VALUE_TYPE::float_value;
			}
			else
			{
				type = INI_FILE_VALUE_TYPE::int_value;
			}
		}
	}

	return sValue;
}

CL::IniFileOutput::IniFileOutput(const std::string& sFile)
{
	_file.open(sFile);
}

void CL::IniFileOutput::write(const std::string& sName, int i, const std::string& sComment)
{
	assert(_file.is_open());
	_file << sName << " " << i << ";" << sComment << std::endl;
}

void CL::IniFileOutput::write(const std::string& sName, int i)
{
	assert(_file.is_open());
	_file << sName << " " << i << std::endl;
}

void CL::IniFileOutput::write(const std::string& sName, float f, const std::string& sComment)
{
	assert(_file.is_open());
	_file << sName << " " << std::fixed << std::setprecision(5) << f << ";" << sComment << std::endl;
}

void CL::IniFileOutput::write(const std::string& sName, float f)
{
	assert(_file.is_open());
	_file << sName << " " << std::fixed << std::setprecision(5) << f << std::endl;
}

void CL::IniFileOutput::close()
{
	_file.close();
}
