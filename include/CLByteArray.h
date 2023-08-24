#pragma once
#include "CLMemory.h"

namespace CL
{
	class ByteArray
	{
	public:
		ByteArray() = default;
		ByteArray(const void* pData, size_t Size) :
			_Size(Size), _Capacity(Size)
		{
			if (_Size)
			{
				_pData = CL_MALLOC(_Capacity);

				if (pData)
				{
					memcpy(_pData, pData, _Size);
				}
			}
		}
		ByteArray(const ByteArray& Other) :
			_Size(Other._Size), _Capacity(Other._Capacity)
		{
			if (_Size)
			{
				_pData = CL_MALLOC(_Capacity);
				memcpy(_pData, Other._pData, _Size);
			}
		}
		ByteArray(ByteArray&& Other) :
			_Size(Other._Size), _pData(Other._pData), _Capacity(Other._Capacity)
		{
			Other._pData = nullptr;
			Other._Size = 0;
			Other._Capacity = 0;
		}
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
		void SetCarriage(size_t Position) 
		{
			CL_ASSERT(Position < _Capacity);
			_Carriage = Position; 
		}
		void Resize(size_t NewCapacity, bool bKeepData = false)
		{
			void* pNewData = nullptr;
			_Size = CL_MIN(_Size, NewCapacity);

			if (NewCapacity)
			{
				pNewData = CL_MALLOC(NewCapacity);

				if (_pData && bKeepData)
				{
					memcpy(pNewData, _pData, _Size);
				}
			}

			CL_FREE(_pData);
			_Capacity = NewCapacity;
			_pData = pNewData;
		}
		void PushBack(const void* pData, size_t Size)
		{
			CL_DEBUG_ASSERT(pData);
			CL_DEBUG_ASSERT(Size);

			if (_Size + Size > _Capacity)
			{
				Resize(CL_MAX(_Size + Size, _Capacity * 2 + 1), true);
			}

			memcpy((char*)_pData + _Size, pData, Size);
			_Size += Size;
		}
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
		void ReadForward(void* pData, size_t Size)
		{
			CL_DEBUG_ASSERT(pData);
			CL_DEBUG_ASSERT(Size);
			CL_ASSERT(_Carriage + Size <= _Capacity);
			memcpy(pData, (char*)_pData + _Carriage, Size);
			_Carriage += Size;
		}
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
			Other._Size = Size;
			ReadForward(Other.Data(), Size);
			return *this;
		}
		size_t Size() const { return _Size; }
		size_t Capacity() const { return _Capacity; }
		void* Data() { return _pData; }
		const void* Data() const { return _pData; }
		void Free()
		{
			if (_pData)
			{
				CL_FREE(_pData);
				_Size = 0;
				_Capacity = 0;
				_pData = nullptr;
			}
		}
		~ByteArray()
		{
			Free();
		}
	private:
		size_t _Size = 0;
		size_t _Capacity = 0;
		size_t _Carriage = 0;
		void* _pData = nullptr;
	};
}