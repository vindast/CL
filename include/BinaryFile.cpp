#include "BinaryFile.h"
#include <Windows.h>
#include "CLMemory.h"

CL::BinaryInputFile::BinaryInputFile(const std::string& patch) : file(patch.c_str(), std::ios::binary)
{
//	assert(file.is_open() && "fail to open ");
}

bool CL::BinaryInputFile::isOpen() const noexcept
{
	return file.is_open();
}

void CL::BinaryInputFile::read(std::string& str)
{
	size_t length = 0;
	read(length);

	str.clear();

	if (length)
	{

		char* buffer = CL_NEW_ARR(char, length);

		file.read(buffer, length);

		for (size_t i = 0; i < length; i++)
		{
			str += buffer[i];
		}

		CL_DELETE_ARR(buffer);
	}
}

size_t CL::BinaryInputFile::getCurrentLoc()
{
	return file.tellg();
}

void CL::BinaryInputFile::seek(const size_t byte)
{
	file.seekg(byte);
}

void CL::BinaryInputFile::close()
{
	file.close();
}

bool CL::BinaryInputFile::eof() const
{
	return file.eof();
}

CL::BinaryInputFile::~BinaryInputFile()
{
	file.close();
}

CL::BinaryOutputFile::BinaryOutputFile(const std::string& patch) : file(patch.c_str(), std::ios::binary)
{

}

void CL::BinaryOutputFile::write(const std::string& str)
{
	//assert(str.length());
	size_t length = str.length();
	file.write((char*)&length, sizeof(length));

	if (length)
	{
		file.write(str.c_str(), length);
	}
}

void CL::BinaryOutputFile::seek(const size_t byte)
{
	file.seekp(byte);
}

size_t CL::BinaryOutputFile::getCurrentLoc()
{
	return file.tellp();
}

void CL::BinaryOutputFile::write(const char* data)
{
	std::string str = data;

	write(str);
}

void CL::BinaryOutputFile::close()
{
	file.close();
}

CL::BinaryOutputFile::~BinaryOutputFile()
{
	file.close();
}

void CL::createDirectories(const std::string& patch) noexcept
{
	std::string currentPath;

	for (size_t i = 0; i < patch.length(); i++)
	{
		if (patch[i] == '/')
		{
			currentPath = patch.substr(0, i);

			CreateDirectory(currentPath.c_str(), nullptr);
		}
	}

	CreateDirectory(patch.c_str(), nullptr);
}
