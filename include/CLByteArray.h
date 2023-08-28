#pragma once
#include "CLMemory.h"

namespace CL
{
	class ByteArray
	{
	public:
		ByteArray() = default;
		ByteArray(const void* pData, size_t Size);
		ByteArray(const ByteArray& Other);
		ByteArray(ByteArray&& Other);
		ByteArray& operator = (const ByteArray& Other)
		{
			if (this != &Other)
			{
				Free();

				if (Other._Size)
				{
					_Capacity = Other._Capacity;
					_Size = Other._Size;
					_pData = CL_MALLOC(_Capacity);
					memcpy(_pData, Other._pData, _Size);
				}
			}

			return *this;
		}
		ByteArray& operator = (ByteArray&& Other)
		{
			Free();
			_pData = Other._pData;
			_Size = Other._Size;
			_Capacity = Other._Capacity;
			Other._pData = nullptr;
			Other._Size = 0;
			return *this;
		}
		void ResetCarriage() { _Carriage = 0; }
		void SetCarriage(size_t Position);
		void Reserve(size_t NewSize, bool bKeepData = false);
		void Resize(size_t NewCapacity, bool bKeepData = false);
		void PushBack(const void* pData, size_t Size);
		template<class ObjType>
		ByteArray& operator << (const ObjType& Obj)
		{
			PushBack(&Obj, sizeof(ObjType));
			return *this;
		}
		ByteArray& operator << (const ByteArray& Other)
		{
			uint64_t Size = Other.Size();
			PushBack(&Size, sizeof(Size));
			PushBack(Other.Data(), Size);
			return *this;
		}
		void ReadForward(void* pData, size_t Size);
		template<class ObjType>
		ByteArray& operator >> (ObjType& Obj)
		{
			ReadForward(&Obj, sizeof(ObjType));
			return *this;
		}
		ByteArray& operator >> (ByteArray& Other)
		{
			uint64_t Size = 0;
			ReadForward(&Size, sizeof(Size));
			Other.Resize(Size);
			ReadForward(Other.Data(), Size);
			return *this;
		}
		size_t Size() const { return _Size; }
		size_t Capacity() const { return _Capacity; }
		void* Data() { return _pData; }
		const void* Data() const { return _pData; }
		void Free();
		~ByteArray();
	private:
		size_t _Size = 0;
		size_t _Capacity = 0;
		size_t _Carriage = 0;
		void* _pData = nullptr;
	};
}