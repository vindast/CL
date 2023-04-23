#pragma once
#include <CLString.h>
#include <CLList.h>

namespace CL
{
	struct FDPath
	{
		CL::String Path;
		bool bIsFile = false;
	};

	struct alignas(4) FileLastWriteTime
	{
		uint32_t Year;
		uint32_t Month;
		uint32_t Day;
		uint32_t Hour;
		uint32_t Minute;
		uint32_t Second;
		uint32_t Milliseconds;
	};

	static bool operator == (const FileLastWriteTime& A, const FileLastWriteTime& B)
	{
		return A.Year == B.Year
			&& A.Month == B.Month
			&& A.Day == B.Day
			&& A.Hour == B.Hour
			&& A.Minute == B.Minute
			&& A.Second == B.Second
			&& A.Milliseconds == B.Milliseconds;
	};

	static bool operator != (const FileLastWriteTime& A, const FileLastWriteTime& B)
	{
		return A.Year != B.Year
			|| A.Month != B.Month
			|| A.Day != B.Day
			|| A.Hour != B.Hour
			|| A.Minute != B.Minute
			|| A.Second != B.Second
			|| A.Milliseconds != B.Milliseconds;
	};

	void EnumerateFilesInDirectory(String directory, List<String>& files);
	void EnumerateFilesInDirectory(String directory, CL::List<String>& files, const String& ext);
	bool IsFileExist(const String& sFile);
	bool IsFolderExist(const String& sFolder);
	bool CreateFolder(const String& sFolder);
	//Folder separator "/"
	void CreateFolders(String sFolders);
	bool GetLastFileWriteTime(const String& sFile, FileLastWriteTime& OutLastWriteTime);
	String GetFileNameFromPath(String sFile);
	String GetWorkingDirectory();

	//Split Directory by "\\" or by "/" and return path as list
	CL::List<CL::String> ExtractPath(CL::String Directory);
};