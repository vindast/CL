#include "FilesSystem.h"
#include <fstream>
#include <direct.h>

namespace CL
{
	bool FileCopy(const CL::String& OriginalFilePath, const CL::String& NewFilePath, bool bOverrideIfExist)
	{
		return CopyFile(
			OriginalFilePath.CStr(),
			NewFilePath.CStr(),
			!bOverrideIfExist
		);
	}

	void EnumerateFilesInDirectory(String directory, List<String>& files)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hf;

		directory += "\\*";

		hf = FindFirstFile(directory.CStr(), &FindFileData);

		if (hf != INVALID_HANDLE_VALUE)
		{
			do {
				if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					if (lstrcmp(FindFileData.cFileName, ".") != 0 && lstrcmp(FindFileData.cFileName, "..") != 0)
					{
						files.PushBack(FindFileData.cFileName);
					}
				}
			} while (FindNextFile(hf, &FindFileData) != 0);

			FindClose(hf);
		}
	}

	void EnumerateFilesInDirectory(String directory, CL::List<String>& files, const String& ext)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hf;

		directory += "\\*." + ext;

		hf = FindFirstFile(directory.CStr(), &FindFileData);

		if (hf != INVALID_HANDLE_VALUE)
		{
			do {

				if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				{
					if (lstrcmp(FindFileData.cFileName, ".") != 0 && lstrcmp(FindFileData.cFileName, "..") != 0)
					{

						files.PushBack(FindFileData.cFileName);

					}

				}

			} while (FindNextFile(hf, &FindFileData) != 0);

			FindClose(hf);
		}
	}

	bool IsFileExist(const String& sFile)
	{
		std::fstream file;

		file.open(sFile.CStr(), std::ios::in);

		bool bIsOpen = file.is_open();

		if (bIsOpen)
		{
			file.close();
		}

		return bIsOpen;
	}

	bool IsFolderExist(const String& sFolder)
	{
		struct stat buff;
		int i = stat(sFolder.CStr(), &buff);

		return i == 0;
	}

	bool CreateFolder(const String& sFolder)
	{
		int i = _mkdir(sFolder.CStr());
		return i == 0;
	}

	//Folder separator "/"
	void CreateFolders(String sFolders)
	{
		size_t iStartPos = 0;
		size_t iEndPos = sFolders.FindFirst("/");

		while (iEndPos != String::NullPos())
		{
			String sFolder = sFolders.Substring(iStartPos, iEndPos - iStartPos);

			iStartPos = iEndPos + 1;
			iEndPos = sFolders.FindFirst("/", iEndPos + 1);
			iStartPos = 0;

			_mkdir(sFolder.CStr());
		}

		_mkdir(sFolders.CStr());
	}

	bool GetLastFileWriteTime(const String& sFile, FileLastWriteTime& OutLastWriteTime)
	{
		WIN32_FIND_DATAA findData;
		HANDLE hFind;

		hFind = FindFirstFileA(sFile.CStr(), &findData);

		if (hFind == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		SYSTEMTIME sutc, st;
		FileTimeToSystemTime(&(findData.ftLastWriteTime), &sutc);
		SystemTimeToTzSpecificLocalTime(NULL, &sutc, &st);

		OutLastWriteTime.Year = st.wYear;
		OutLastWriteTime.Day = st.wDay;
		OutLastWriteTime.Month = st.wMonth;
		OutLastWriteTime.Hour = st.wHour;
		OutLastWriteTime.Minute = st.wMinute;
		OutLastWriteTime.Second = st.wSecond;
		OutLastWriteTime.Milliseconds = st.wMilliseconds;

		return true;
	}

	String GetFileNameFromPath(String sFile, bool bEraseExtention)
	{
		size_t SlashIndex0 = sFile.FindLast("/");
		size_t SlashIndex1 = sFile.FindLast("\\");
		size_t SlashIndex = String::NullPos();

		if (SlashIndex0 != String::NullPos() && SlashIndex1 != String::NullPos())
		{
			SlashIndex = CL_MAX(SlashIndex0, SlashIndex1);
		}
		else if (SlashIndex0 != String::NullPos())
		{
			SlashIndex = SlashIndex0;
		}
		else
		{
			SlashIndex = SlashIndex1;
		}

		if (SlashIndex != String::NullPos())
		{
			sFile = sFile.Substring(SlashIndex + 1);
		}

		if (bEraseExtention)
		{
			size_t PointIndex = sFile.FindLast(".");

			if (PointIndex != String::NullPos())
			{
				sFile = sFile.Substring(0, PointIndex);
			}
		}

		return sFile;
	}

	String GetWorkingDirectory()
	{
		char Buffer[256];
		GetCurrentDirectory(256, Buffer);
		return Buffer;
	}

	List<String> ExtractPath(String Directory)
	{
		List<String> Path;
		size_t SplitPosition = String::NullPos();

		while ((SplitPosition = Directory.FindFirst("/")) != String::NullPos() || (SplitPosition = Directory.FindFirst("\\")) != String::NullPos())
		{
			size_t Offset = Directory.GetLength() > (SplitPosition + 1) && Directory[SplitPosition + 1] == '\\' ? 2 : 1;

			Path.PushBack(Directory.Substring(0, SplitPosition));

			if (Directory.GetLength() > SplitPosition + Offset)
			{
				Directory = Directory.Substring(SplitPosition + Offset);
			}
			else
			{
				break;
			}
		}

		return Path;
	}
}