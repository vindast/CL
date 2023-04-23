#pragma once
#include <iostream>
#include <assert.h>
#include "CLObjects/CLCriticalSection.h"
#include "CLMemory.h"

namespace CL
{
	template<size_t maxBlockCount, size_t blockElementCount, class type, size_t updateMaxIteration = 32> class DynamicArray
	{
	public:
		DynamicArray():
			_maxBlockCount(maxBlockCount),
			_blockElementCount(blockElementCount),
			_updateMaxIteration(updateMaxIteration) 
		{
			aloc();
		}
		void update()
		{
			if (_blockCount && _currentSize < (_blockCount - 1)* _blockElementCount)
			{
				_updateCallCount++;

				if (_updateCallCount > _updateMaxIteration)
				{ 
					_updateCallCount = 0;

					_blockCount--; 

					CL_DELETE(_mData[_blockCount]);
				}
			}
			else
			{
				_updateCallCount = 0;
			}
			 
		}
		~DynamicArray()
		{
			for (size_t i = 0; i < _blockCount; i++)
			{
				CL_DELETE( _mData[i]);
			}
		}
		type& push_back(const type& value)
		{
			LockGuard<CriticalSection> lock(_hCS);

			size_t id = _currentSize++;
			size_t blockId = id / _blockElementCount;
			size_t element = id % _blockElementCount;
			
			//std::cout << "currentSize = " << _currentSize << std::endl;

			if (blockId >= _blockCount)
			{
				aloc();
			} 

			//std::cout << "id = " << id << std::endl;

			_mData[blockId][element] = value;

			return _mData[blockId][element];
		}
		void clear()
		{
			_currentSize = 0;
		}
		type& operator [] (size_t id)
		{
			size_t blockId = id / _blockElementCount;
			size_t element = id % _blockElementCount;
			 

			assert(blockId < _blockCount);
			assert(element < (_currentSize -_blockElementCount * blockId));

			return _mData[blockId][element];
		}
		const type& operator [] (size_t id) const
		{
			size_t blockId = id / _blockElementCount;
			size_t element = id % _blockElementCount;
			 
			assert(blockId < _blockCount);
			assert(element < (_currentSize - _blockElementCount * blockId));

			return _mData[blockId][element];
		}
		size_t size() const
		{
			return _currentSize;
		}
		size_t getMaxCount() const
		{
			return _maxBlockCount * _blockElementCount;
		}
	private:
		CriticalSection _hCS;
		size_t _currentSize = 0;
		size_t _blockCount = 0;
		size_t _updateCallCount = 0;

		const size_t _updateMaxIteration;
		const size_t _maxBlockCount;
		const size_t _blockElementCount;

		type* _mData[maxBlockCount]; 

		void aloc()
		{
			_updateCallCount = 0;

			assert(_blockCount < _maxBlockCount);
			 
			_mData[_blockCount] = CL_NEW_ARR( type, _blockElementCount);
			_blockCount++; 
		}

		DynamicArray& operator = (const DynamicArray&)
		{

		}

		DynamicArray(const DynamicArray&)
		{

		}
	};


}