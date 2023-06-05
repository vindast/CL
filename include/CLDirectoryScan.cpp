#include "CLDirectoryScan.h"

namespace CL
{
	void DirectoryScan::AddExtention(const String& Ext)
	{
		_mFileExt.PushBack(Ext);
	}

	List<FDPath> DirectoryScan::Find(const String& Key, bool bSearchFiles, bool bSearchFolders) const
	{
		List<FDPath> Files;

		for (const FDPath& Path : _Files)
		{
			if ((Path.bIsFile && bSearchFiles || !Path.bIsFile && bSearchFolders) && Path.Path.FindFirst(Key.CStr()) != String::NullPos())
			{
				Files.PushBack(Path);
			}
		}

		return Files;
	}

	void DirectoryScan::Clear()
	{
		_mFileExt.Clear();
		_Files.Clear();
	}

	void DirectoryScan::Scan(const String& StartPath, bool bRecursive, bool bClear)
	{
		if (bClear)
		{
			Clear();
		}

		ScanDirectory(StartPath, bRecursive);
	}

	DirectoryScan::~DirectoryScan()
	{
		Clear();
	}

	void DirectoryScan::ScanDirectory(String Dirrectory, bool bRecursive)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hf;

		if (Dirrectory[Dirrectory.GetLength() - 1] != '/' || Dirrectory[Dirrectory.GetLength() - 1] == '\\')
		{
			Dirrectory += String("/");
		}

		hf = FindFirstFile((Dirrectory + "*").CStr(), &FindFileData);

		if (hf != INVALID_HANDLE_VALUE)
		{
			do {

				if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (lstrcmp(FindFileData.cFileName, ".") != 0 && lstrcmp(FindFileData.cFileName, "..") != 0)
					{
						FDPath f;
						f.Path = Dirrectory + FindFileData.cFileName;
						f.bIsFile = false;
						_Files.PushBack(f);

						if (bRecursive)
						{
							ScanDirectory(f.Path, bRecursive);
						}
					}

				}

			} while (FindNextFile(hf, &FindFileData) != 0);

			FindClose(hf);
		}

		if (_mFileExt.GetElementsCount())
		{
			for (auto fileExt : _mFileExt)
			{
				String currentDirectory = Dirrectory + "*." + fileExt;

				hf = FindFirstFile(currentDirectory.CStr(), &FindFileData);

				if (hf != INVALID_HANDLE_VALUE)
				{
					do {
						if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
						{
							if (lstrcmp(FindFileData.cFileName, ".") != 0 && lstrcmp(FindFileData.cFileName, "..") != 0)
							{
								FDPath f;
								f.Path = Dirrectory + FindFileData.cFileName;
								f.bIsFile = true;
								_Files.PushBack(f);
							}
						}

					} while (FindNextFile(hf, &FindFileData) != 0);

					FindClose(hf);
				}
			}
		}
		else
		{
			String currentDirectory = Dirrectory + "*.*";

			hf = FindFirstFile(currentDirectory.CStr(), &FindFileData);

			if (hf != INVALID_HANDLE_VALUE)
			{
				do {
					if (!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) && !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						if (lstrcmp(FindFileData.cFileName, ".") != 0 && lstrcmp(FindFileData.cFileName, "..") != 0)
						{
							FDPath f;
							f.Path = Dirrectory + FindFileData.cFileName;
							f.bIsFile = true;
							_Files.PushBack(f);
						}
					}

				} while (FindNextFile(hf, &FindFileData) != 0);

				FindClose(hf);
			}
		}
	}
}