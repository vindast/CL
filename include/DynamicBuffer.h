#pragma once 
#include "CLObjects/CLCriticalSection.h"
#include "CLMemory.h"

namespace CL
{
	template<class object>class DynamicBuffer
	{
	public:
		DynamicBuffer()
		{
			_nRealSize = 128;
			_pData = CL_NEW_ARR( object, _nRealSize);
		}
		DynamicBuffer(size_t iBaseSize)
		{
			_nRealSize = iBaseSize;

			_pData = CL_NEW_ARR( object, _nRealSize);
		}
		void pushBack(const object& obj)
		{ 
			if (_nSize == _nRealSize - 1)
			{
				size_t nNewSize = _nRealSize * 2;

				object* pTMPData = _pData;

				_pData = CL_NEW_ARR( object, nNewSize);
				 
				for (size_t i = 0; i < _nRealSize; i++)
				{
					_pData[i] = pTMPData[i];
				}

				CL_DELETE_ARR(pTMPData);

				_nRealSize = nNewSize;
			} 

			_pData[_nSize++] = obj;
		}
		void pushBackAtomic(const object& obj)
		{
			LockGuard<CriticalSection> lock(_hCS);

			if (_nSize == _nRealSize - 1)
			{
				size_t nNewSize = _nRealSize * 2;

				object* pTMPData = _pData;

				_pData = CL_NEW_ARR( object, nNewSize);

				for (size_t i = 0; i < _nRealSize; i++)
				{
					_pData[i] = pTMPData[i];
				}

				CL_DELETE_ARR(pTMPData);

				_nRealSize = nNewSize;
			}

			_pData[_nSize++] = obj;
		}
		void resetSize()
		{
			_nSize = 0;
		}
		const object* data() const
		{
			return _pData;
		}
		object* data()
		{
			return _pData;
		}
		size_t getSize() const
		{
			return _nSize;
		}
		size_t getRealSize() const
		{
			return _nRealSize;
		}
		~DynamicBuffer()
		{
			CL_DELETE_ARR(_pData);
		}
		object& operator[] (size_t id)
		{
			return _pData[id];
		}
		const object& operator[] (size_t id) const
		{
			return _pData[id];
		}
	private:
		CriticalSection _hCS;
		object* _pData = nullptr;

		size_t _nSize = 0;
		size_t _nRealSize = 0;

		DynamicBuffer(const DynamicBuffer&) = delete;
		DynamicBuffer& operator = (const DynamicBuffer&) = delete;
	};
};


