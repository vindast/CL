#include "CLByteArray.h"

namespace CL
{
	ByteArray::ByteArray(const void* pData, size_t Size) :
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

	ByteArray::ByteArray(const ByteArray& Other) :
		_Size(Other._Size), _Capacity(Other._Capacity)
	{
		if (_Size)
		{
			_pData = CL_MALLOC(_Capacity);
			memcpy(_pData, Other._pData, _Size);
		}
	}
	
	ByteArray::ByteArray(ByteArray&& Other) :
		_Size(Other._Size), _pData(Other._pData), _Capacity(Other._Capacity)
	{
		Other._pData = nullptr;
		Other._Size = 0;
		Other._Capacity = 0;
	}
	
	void ByteArray::SetCarriage(size_t Position)
	{
		CL_ASSERT(Position < _Capacity);
		_Carriage = Position;
	}
	
	void ByteArray::Reserve(size_t NewCapacity, bool bKeepData)
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

	void ByteArray::Resize(size_t NewSize, bool bKeepData)
	{
		void* pNewData = nullptr;

		if (NewSize)
		{
			pNewData = CL_MALLOC(NewSize);

			if (_pData && bKeepData)
			{
				memcpy(pNewData, _pData, CL_MIN(_Size, NewSize));
			}
		}

		CL_FREE(_pData);
		_Size = NewSize;
		_Capacity = NewSize;
		_pData = pNewData;
	}
	
	void ByteArray::PushBack(const void* pData, size_t Size)
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
	
	void ByteArray::ReadForward(void* pData, size_t Size)
	{
		CL_DEBUG_ASSERT(pData);
		CL_DEBUG_ASSERT(Size);
		CL_ASSERT(_Carriage + Size <= _Capacity);
		memcpy(pData, (char*)_pData + _Carriage, Size);
		_Carriage += Size;
	}
	
	void ByteArray::Free()
	{
		if (_pData)
		{
			CL_FREE(_pData);
			_Size = 0;
			_Capacity = 0;
			_pData = nullptr;
		}
	}
	
	ByteArray::~ByteArray()
	{
		Free();
	}
}