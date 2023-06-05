#pragma once
#include <FilesSystem.h>

namespace CL
{
	class DirectoryScan
	{
	public:
		void AddExtention(const String& Ext);
		List<FDPath> Find(const String& Key, bool bSearchFiles = true, bool bSearchFolders = true) const;
		void Clear();
		void Scan(const String& StartPath, bool bRecursive = true, bool bClear = false);
		const List<FDPath>& GetFilesAndFilders() const { return _Files; }
		~DirectoryScan();
	private:
		void ScanDirectory(String Dirrectory, bool bRecursive);

		List<String> _mFileExt;
		List<FDPath> _Files;
	};
}