#pragma once 
#include <fstream>
#include <assert.h>
#include <string> 
#include <iostream>

namespace CL
{

	class BinaryInputFile
	{
	public:
		BinaryInputFile(const std::string& patch); 
		bool isOpen() const noexcept; 
		void read(std::string& str); 
		template<class t> void read(t* data, const size_t bytes)
		{
			file.read((char*)data, bytes);
		} 
		template<class t> void read(t& data)
		{
			file.read((char*)&data, sizeof(t));
		} 
		size_t getCurrentLoc(); 
		void seek(const size_t byte); 
		void close(); 
		bool eof() const;  
		~BinaryInputFile(); 
	private:
		std::ifstream file;
	};


	class BinaryOutputFile
	{
	public:
		BinaryOutputFile(const std::string& patch);

		void write(const std::string& str);

		void seek(const size_t byte);

		size_t getCurrentLoc();

		template<class t> void write(const t* data, const size_t bytes)
		{
			file.write((char*)data, bytes);
		}

		template<class t> void write(const t& data)
		{
			file.write((char*)&data, sizeof(t));
		}

		void write(const char* data);

		void close();

		~BinaryOutputFile();

	private:
		std::ofstream file;
	};

	void createDirectories(const std::string& patch) noexcept;
};

