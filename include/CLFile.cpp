#include "CLFile.h"

namespace CL
{
	
	File::File(const String& Path, bool bRead, bool bWrite, bool bOverride)
	{
		Open(Path, bRead, bWrite, bOverride);
	}
	
	bool File::Open(const String& Path, bool bRead, bool bWrite, bool bOverride)
	{
		if (_hFile != INVALID_HANDLE_VALUE)
		{
			return false;
		}

		_hFile = CreateFile(Path.CStr(),
			(bWrite ? GENERIC_WRITE : 0) | (bRead ? GENERIC_READ : 0),
			0,
			NULL,
			bOverride ? CREATE_ALWAYS : OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);

		if (_hFile == INVALID_HANDLE_VALUE)
		{
			return false;
		}

		_Path = Path;
		_bReadMode = bRead;
		_bWriteMode = bWrite;

		return true;
	}
	
	void File::Close()
	{
		if (_hFile == INVALID_HANDLE_VALUE)
		{
			return;
		}

		CloseHandle(_hFile);
		_hFile = INVALID_HANDLE_VALUE;
		_bReadMode = false;
		_bWriteMode = false;
		_Path.Clear();
	}
	
	bool File::Write(const void* pData, size_t Size)
	{
		CL_ASSERT(_hFile != INVALID_HANDLE_VALUE);
		CL_ASSERT(_bWriteMode);

		DWORD dwBytesWritten = 0;

		BOOL bErrorFlag = WriteFile(
			_hFile,
			pData,
			Size,
			&dwBytesWritten,
			NULL);

		return bErrorFlag != FALSE;
	}
	
	bool File::Read(void* pData, size_t Size)
	{
		CL_ASSERT(_hFile != INVALID_HANDLE_VALUE);
		CL_ASSERT(_bReadMode);
		DWORD dwBytesWritten = 0;

		BOOL bErrorFlag = ReadFile(
			_hFile,
			pData,
			Size,
			&dwBytesWritten,
			NULL);

		return bErrorFlag != FALSE;
	}

	uint64_t File::GetLocation() const
	{
		long high_dword = 0;
		DWORD low_dword = SetFilePointer(_hFile, 0, &high_dword, FILE_CURRENT);
		return high_dword << 32 | uint64_t(low_dword);
	}

	bool File::Seek(size_t Position)
	{
		BOOL bErrorFlag = SetFilePointer(
			_hFile,
			Position,
			NULL,
			FILE_BEGIN
		);

		return bErrorFlag != INVALID_SET_FILE_POINTER;
	}

	File::~File()
	{
		Close();
	}
}