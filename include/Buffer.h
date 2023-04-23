#pragma once
#include <assert.h>
#include "CLMemory.h"
 
namespace CL
{
	template<class t> class Buffer final
	{
	public:
		Buffer()
		{

		}

		Buffer(const size_t maxElements) :
			_maxElements(maxElements)
		{
			assert(maxElements > 0);

			_elements = CL_NEW_ARR( t, maxElements);
		}

		t& operator [](const size_t id)
		{
			assert(id < _currentSize);

			return _elements[id];
		}

		const t& operator [](const size_t id) const
		{
			assert(id < _currentSize);

			return _elements[id];
		}

		const size_t getSize() const
		{
			return _currentSize;
		}

		const size_t getMaxSize() const
		{
			return _maxElements;
		}

		void add(const t& v)
		{
      			assert(_currentSize + 1 <= _maxElements);
			_elements[_currentSize] = v;
			_currentSize++;
		}

		void add()
		{
			assert(_currentSize + 1 <= _maxElements);
			_currentSize++;
		}

		t& last()
		{
			assert(_currentSize <= _maxElements && _currentSize > 0);
			return _elements[_currentSize - 1];
		}

		const t& last() const
		{
			assert(_currentSize <= _maxElements && _currentSize > 0);
			return _elements[_currentSize - 1];
		}

		~Buffer()
		{
			if (_elements)
			{
				CL_DELETE_ARR(_elements);
			}
		}

		void clear()
		{
			_currentSize = 0;
		}

		Buffer(const Buffer& arr)
		{
			this->_maxElements = arr._maxElements;
			this->_currentSize = arr._currentSize;

			this->_elements = CL_NEW_ARR( t, _maxElements);
			memcpy(this->_elements, arr._elements, this->_currentSize * sizeof(t));
		}

		Buffer& operator = (const Buffer& arr)
		{
			if (this->_elements)
			{
				CL_DELETE_ARR(this->_elements);
			}

			this->_maxElements = arr._maxElements;
			this->_currentSize = arr._currentSize;

			this->_elements = CL_NEW_ARR( t, _maxElements);
			memcpy(this->_elements, arr._elements, this->_currentSize * sizeof(t));

			return *this;
		}

		void deleteObj(const size_t id)
		{
			assert(id < _maxElements);

			if (id + 1 < _maxElements)
			{
				memcpy(&_elements[id], &_elements[id + 1], (this->_currentSize - id) * sizeof(t));
			}

			_currentSize--;
		}

	private:
		size_t _maxElements = 0;
		size_t _currentSize = 0;
		t* _elements = nullptr;
	};
}

