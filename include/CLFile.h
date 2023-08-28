#pragma once 
#include <CLString.h>
#include <CLByteArray.h>

namespace CL
{
	class File
	{
		CL_DELETE_COPY_OPERATORS(File)
	public:
		File() = default;
		File(const String& Path, bool bRead, bool bWrite, bool bOverride);
		bool Open(const String& Path, bool bRead, bool bWrite, bool bOverride);
		void Close();
		bool Write(const void* pData, size_t Size);
		template<class ObjectType>
		void operator << (const ObjectType& Obj)
		{
			Write(&Obj, sizeof(Obj));
		}
		void operator << (const ByteArray& Array)
		{
			uint64_t Size = Array.Size();
			Write(&Size, sizeof(Size));
			Write(Array.Data(), Size);
		}
		bool Read(void* pData, size_t Size);
		template<class ObjectType>
		void operator >> (ObjectType& Obj)
		{
			Read(&Obj, sizeof(Obj));
		}
		void operator >> (ByteArray& Array)
		{
			uint64_t Size = 0;
			Read(&Size, sizeof(Size));
			Array.Resize(Size);
			if (Size)
			{
				Read(Array.Data(), Size);
			}
		}
		uint64_t GetLocation() const;
		bool Seek(size_t Position);
		//TODO: eof
		bool IsOpen() const { return _hFile != INVALID_HANDLE_VALUE; }
		const CL::String& GetPath() const { return _Path; }
		bool IsReadMode() const { return _bReadMode; }
		bool IsWriteMode() const { return _bWriteMode; }
		HANDLE GetHandle() const { return _hFile; }
		~File();
	private:
		bool _bReadMode = false;
		bool _bWriteMode = false;
		CL::String _Path;
		HANDLE _hFile = INVALID_HANDLE_VALUE;
	};
};

