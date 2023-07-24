#pragma once
#include "CLMemory.h"
#include "CLCommon.h"

namespace CL
{
	template<class Element>
	class VectorForwardIterator
	{
		typedef VectorForwardIterator Iterator;
	public:
		VectorForwardIterator() = default;
		VectorForwardIterator(Element* pCurrentElement, const Element* pLastElement) :
			_pCurrentElement(pCurrentElement), _pLastElement(pLastElement)
		{

		}
		Iterator& operator++(int)
		{
			if (_pCurrentElement)
			{
				_pCurrentElement++;

				if (_pCurrentElement > _pLastElement)
				{
					_pCurrentElement = nullptr;
					_pLastElement = nullptr;
				}
			}

			return *this;
		}
		Iterator& operator++()
		{
			if (_pCurrentElement)
			{
				_pCurrentElement++;

				if (_pCurrentElement > _pLastElement)
				{
					_pCurrentElement = nullptr;
					_pLastElement = nullptr;
				}
			}

			return *this;
		}
		Element& operator*()
		{
			CL_DEBUG_ASSERT(_pCurrentElement);
			return *_pCurrentElement;
		}
		Element& operator ()()
		{
			CL_DEBUG_ASSERT(_pCurrentElement);
			return *_pCurrentElement;
		}
		Element& operator ->()
		{
			CL_DEBUG_ASSERT(_pCurrentElement);
			return *_pCurrentElement;
		}
		operator bool() const
		{
			return _pCurrentElement;
		}
		bool operator == (const Iterator& other) const
		{
			return _pCurrentElement == other._pCurrentElement && _pLastElement == other._pLastElement;
		}
		bool operator != (const Iterator& other) const
		{
			return _pCurrentElement != other._pCurrentElement || _pLastElement != other._pLastElement;
		}
	private:
		Element* _pCurrentElement = nullptr;
		const Element* _pLastElement = nullptr;
	};

	template<class Element>
	class VectorConstForwardIterator
	{
		typedef VectorConstForwardIterator Iterator;
	public:
		VectorConstForwardIterator() = default;
		VectorConstForwardIterator(Element* pCurrentElement, const Element* pLastElement) :
			_pCurrentElement(pCurrentElement), _pLastElement(pLastElement)
		{

		}
		Iterator& operator++(int)
		{
			if (_pCurrentElement)
			{
				_pCurrentElement++;

				if (_pCurrentElement > _pLastElement)
				{
					_pCurrentElement = nullptr;
					_pLastElement = nullptr;
				}
			}

			return *this;
		}
		Iterator& operator++()
		{
			if (_pCurrentElement)
			{
				_pCurrentElement++;

				if (_pCurrentElement > _pLastElement)
				{
					_pCurrentElement = nullptr;
					_pLastElement = nullptr;
				}
			}

			return *this;
		}
		const Element& operator*()
		{
			CL_DEBUG_ASSERT(_pCurrentElement);
			return *_pCurrentElement;
		}
		const Element& operator ()()
		{
			CL_DEBUG_ASSERT(_pCurrentElement);
			return *_pCurrentElement;
		}
		const Element& operator ->()
		{
			CL_DEBUG_ASSERT(_pCurrentElement);
			return *_pCurrentElement;
		}
		operator bool() const
		{
			return _pCurrentElement;
		}
		bool operator == (const Iterator& other) const
		{
			return _pCurrentElement == other._pCurrentElement && _pLastElement == other._pLastElement;
		}
		bool operator != (const Iterator& other) const
		{
			return _pCurrentElement != other._pCurrentElement || _pLastElement != other._pLastElement;
		}
	private:
		const Element* _pCurrentElement = nullptr;
		const Element* _pLastElement = nullptr;
	};

	template<class Element>
	class Vector
	{
		typedef Vector<Element> VectorType;
		typedef VectorForwardIterator<Element> ForwardIterator;
		typedef VectorConstForwardIterator<Element> ConstForwardIterator;
	public:
		Vector() = default;
		VectorType(const VectorType& OtherVector)
		{
			if (OtherVector._nElement)
			{
				Resize(OtherVector._Capacity);
				_nElement = OtherVector._nElement;

				for (size_t i = 0; i < _nElement; i++)
				{
					CL_PLACEMENT_NEW(_pMemory + i, Element, OtherVector._pMemory[i]);
				}
			}
		}
		VectorType& operator = (const VectorType& OtherVector)
		{
			if (this == &OtherVector)
			{
				return *this;
			}

			if (!OtherVector._Capacity)
			{
				Clear();
			}
			else if (_Capacity == OtherVector._Capacity)
			{
				for (size_t i = 0; i < _nElement; i++)
				{
					CL_PLACEMENT_DELETE(_pMemory + i);
				}

				_nElement = OtherVector._nElement;

				for (size_t i = 0; i < _nElement; i++)
				{
					CL_PLACEMENT_NEW(_pMemory + i, Element, OtherVector._pMemory[i]);
				}
			}
			else
			{
				Clear();
				Resize(OtherVector._Capacity);
				_nElement = OtherVector._nElement;

				for (size_t i = 0; i < _nElement; i++)
				{
					CL_PLACEMENT_NEW(_pMemory + i, Element, OtherVector._pMemory[i]);
				}
			}

			return *this;
		}
		VectorType(VectorType&& OtherVector) : 
			_Capacity(OtherVector._Capacity),
			_pMemory(OtherVector._pMemory),
			_nElement(OtherVector._nElement)
		{
			OtherVector._Capacity = 0;
			OtherVector._pMemory = nullptr;
			OtherVector._nElement = 0;
		}
		VectorType& operator = (VectorType&& OtherVector)
		{
			Clear();

			_Capacity = OtherVector._Capacity;
			_pMemory = OtherVector._pMemory;
			_nElement = OtherVector._nElement;

			OtherVector._Capacity = 0;
			OtherVector._pMemory = nullptr;
			OtherVector._nElement = 0;

			return *this;
		}

		template<class Lambda>
		void ForEach(Lambda& L)
		{
			for (size_t i = 0; i < _nElement; i++)
			{
				L(_pMemory[i]);
			}
		}
		template<class Lambda>
		void ForEach(Lambda& L) const
		{
			for (size_t i = 0; i < _nElement; i++)
			{
				L(_pMemory[i]);
			}
		}
		void PushBack(const Element& NewElement)
		{
			if (_Capacity <= _nElement)
			{
				Resize(_Capacity * 2 + 1);
			}

			CL_PLACEMENT_NEW(_pMemory + _nElement, Element, NewElement);
			_nElement++;
		}
		void EmplaceBack(Element&& NewElement)
		{
			if (_Capacity <= _nElement)
			{
				Resize(_Capacity * 2 + 1);
			}

			CL_PLACEMENT_NEW(_pMemory + _nElement, Element, NewElement);
			_nElement++;
		}
		void Erase(size_t Index, bool bSwapBack = false)
		{
			CL_ASSERT(Index < _nElement);

			size_t LastElementIndex = _nElement - 1;
			if (Index != LastElementIndex)
			{
				if (bSwapBack)
				{
					if (std::is_move_assignable<Element>::value)
					{
						_pMemory[Index] = Move(_pMemory[LastElementIndex]);
					}
					else
					{
						_pMemory[Index] = _pMemory[LastElementIndex];
					}
				}
				else if (std::is_trivially_copyable<Element>::value)
				{
					memmove(_pMemory + Index, _pMemory + Index + 1, sizeof(Element) * (_nElement - Index - 1));
				}
				else
				{
					for (size_t i = Index; i < LastElementIndex; i++)
					{
						if (std::is_move_assignable<Element>::value)
						{
							_pMemory[i] = Move(_pMemory[i + 1]);
						}
						else
						{
							_pMemory[i] = _pMemory[i + 1];
						}
					}
				}
			}
			
			CL_PLACEMENT_DELETE(_pMemory + LastElementIndex);
			_nElement--;

			if (!_nElement)
			{
				Clear();
			}
		}
		size_t GetCapacity() const { return _Capacity; }
		size_t GetNumElement() const { return _nElement; }
		void Truncate()
		{
			if (_nElement < _Capacity)
			{
				Resize(_nElement);
			}
		}
		void Clear()
		{
			if (_pMemory)
			{
				for (size_t i = 0; i < _nElement; i++)
				{
					CL_PLACEMENT_DELETE(_pMemory + i);
				}

				CL_FREE(_pMemory);
				_Capacity = 0;
				_nElement = 0;
				_pMemory = nullptr;
			}
		}
		Element& operator [](size_t Index)
		{
			CL_ASSERT(Index < _nElement);
			return _pMemory[Index];
		}
		const Element& operator [](size_t Index) const
		{
			CL_ASSERT(Index < _nElement);
			return _pMemory[Index];
		}
		ForwardIterator begin()
		{
			return ForwardIterator(_pMemory, _pMemory + _nElement - 1);
		}
		ForwardIterator end()
		{
			return ForwardIterator();
		}
		ConstForwardIterator begin() const
		{
			return ConstForwardIterator(_pMemory, _pMemory + _nElement - 1);
		}
		ConstForwardIterator end() const
		{
			return ConstForwardIterator();
		}
		unsigned long ComputeCrc32() const
		{
			return Crc32((const unsigned char*)_pMemory, sizeof(Element) * _nElement);
		}
		Element* GetData() { return _pMemory; }
		const Element* GetData() const { return _pMemory; }
		~Vector()
		{
			Clear();
		}
	private:
		void Resize(size_t NewCapacity)
		{
			Element* pNewMemory = (Element*)CL_MALLOC(sizeof(Element) * NewCapacity);

			if (_nElement)
			{
				for (size_t i = 0; i < _nElement; i++)
				{
					if (std::is_move_assignable<Element>::value)
					{
						CL_PLACEMENT_NEW(pNewMemory + i, Element, CL::Move(_pMemory[i]));
					}
					else
					{ 
						CL_PLACEMENT_NEW(pNewMemory + i, Element, _pMemory[i]);
					}

					CL_PLACEMENT_DELETE(_pMemory + i);
				}
			}

			CL_FREE(_pMemory);
			_pMemory = pNewMemory;
			_Capacity = NewCapacity;
		}

		Element* _pMemory = nullptr;
		size_t _Capacity = 0;
		size_t _nElement = 0;
	};
}