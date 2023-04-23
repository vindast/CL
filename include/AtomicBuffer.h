#pragma once
#pragma once
#include <assert.h>
#include <atomic>
#include "CLMemory.h"

namespace CL
{
	template<class t> class AtomicBuffer final
	{
	public:
		AtomicBuffer()
		{

		}

		AtomicBuffer(const size_t maxElements) :
			_maxElements(maxElements)
		{
			assert(maxElements > 0);

			_elements = CL_NEW_ARR(t,maxElements);
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

		/*void add(const t& v)
		{
			assert(_currentSize + 1 <= _maxElements);
			_elements[_currentSize] = v;
			_currentSize++;
		}*/

		/*void add()
		{

			_currentSize++;
		}*/

		/*	t& add()
			{
				size_t s = _currentSize++;

	#ifdef _DEBUG
				assert(s <= _maxElements);
	#endif // _DEBUG

				return _elements[s - 1];
			}*/

		/*size_t add()
		{
			return _currentSize++;
		}*/

		t& add()
		{
			size_t s = _currentSize++;

#ifdef _DEBUG
			assert(s <= _maxElements);
#endif // _DEBUG

			return _elements[s];
		}

		/*t& last() 
		{
			assert(_currentSize <= _maxElements && _currentSize > 0);
			return _elements[_currentSize - 1];
		}*/

		/**/

		~AtomicBuffer()
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

		AtomicBuffer(const AtomicBuffer& arr)
		{
			this->_maxElements = arr._maxElements;
			this->_currentSize = arr._currentSize;

			this->_elements = CL_NEW_ARR(t, _maxElements);
			memcpy(this->_elements, arr._elements, this->_currentSize * sizeof(t));
		}

		AtomicBuffer& operator = (const AtomicBuffer& arr)
		{
			if (this->_elements)
			{
				CL_DELETE_ARR(this->_elements);
			}

			this->_maxElements = arr._maxElements;
			this->_currentSize = arr._currentSize;

			this->_elements = CL_NEW_ARR(t, _maxElements);
			memcpy(this->_elements, arr._elements, this->_currentSize * sizeof(t));

			return *this;
		}

		/*void deleteObj(const size_t id)
		{
			assert(id < _maxElements);

			if (id + 1 < _maxElements)
			{
				memcpy(&_elements[id], &_elements[id + 1], (this->_currentSize - id) * sizeof(t));
			}

			_currentSize--;
		}*/

	private:
		size_t _maxElements = 0;
		std::atomic<size_t> _currentSize = 0;
		t* _elements = nullptr;
	};
};