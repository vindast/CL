#pragma once
#include <assert.h>

namespace CL
{
	template<class t> class CircularBufferBehavior
	{
	public:
		CircularBufferBehavior(t* data, size_t elementsCount)
		{
			_elementsCount = elementsCount;
			_data = data;
		}

		void clear()
		{
			_currentSize = 0;
			_currentElement = 0;
		}

		void add()
		{
			_currentSize++;

			if (_currentSize > _elementsCount)
			{
				_currentSize = _elementsCount;
			}

			if (_currentElement == _elementsCount)
			{
				_currentElement = 0;
			}

			_currentElement++;
		}

		const size_t size() const
		{
			return _currentSize;
		}

		t& last()
		{
			return _data[_currentElement - 1];
		}

		const t& last() const
		{
			return _data[_currentElement - 1];
		}

		t& operator[] (const size_t id)
		{
			assert(id < _currentSize);
			return _data[id];
		}

		const t& operator[] (const size_t id) const
		{
			assert(id < _currentSize);
			return _data[id];
		}

	private:
		size_t _elementsCount = 0;
		size_t _currentSize = 0;
		size_t _currentElement = 0;

		t* _data = nullptr;
	};
}