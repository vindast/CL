#pragma once
#include "CLMemory.h"

namespace CL
{
	class ByteArray
	{
	public:
		ByteArray() = default;
		ByteArray(const void* pData, size_t Size) :
			_Size(Size)
		{
			if (_Size)
			{
				_pData = CL_MALLOC(_Size);

				if (pData)
				{
					memcpy(_pData, pData, _Size);
				}
			}
		}
		ByteArray(const ByteArray& Other) :
			_Size(Other._Size)
		{
			if (_Size)
			{
				_pData = CL_MALLOC(_Size);
				memcpy(_pData, Other._pData, _Size);
			}
		}
		ByteArray(ByteArray&& Other) :
			_Size(Other._Size), _pData(Other._pData)
		{
			Other._pData = nullptr;
			Other._Size = 0;
		}
		ByteArray& operator = (const ByteArray& Other)
		{
			if (this != &Other)
			{
				Free();

				if (Other._Size)
				{
					_Size = Other._Size;
					_pData = CL_MALLOC(_Size);
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
			Other._pData = nullptr;
			Other._Size = 0;
			return *this;
		}
		void Resize(size_t NewSize, bool bKeepData = false)
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

			_pData = pNewData;
			_Size = NewSize;
		}
		size_t Size() const { return _Size; }
		void* Data() { return _pData; }
		const void* Data() const { return _pData; }
		void Free()
		{
			if (_pData)
			{
				CL_FREE(_pData);
				_Size = 0;
				_pData = nullptr;
			}
		}
		~ByteArray()
		{
			Free();
		}
	private:
		size_t _Size = 0;
		void* _pData = nullptr;
	};
}