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

				if (_pCurrentElement >= _pLastElement)
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

				if (_pCurrentElement >= _pLastElement)
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

				if (_pCurrentElement >= _pLastElement)
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

				if (_pCurrentElement >= _pLastElement)
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
				ResizeImpl(OtherVector._Capacity);
				_nElement = OtherVector._nElement;

				for (size_t i = 0; i < _nElement; i++)
				{
					CL_PLACEMENT_NEW(_pObjects + i, Element, OtherVector._pObjects[i]);
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
					CL_PLACEMENT_DELETE(Element, _pObjects + i);
				}

				_nElement = OtherVector._nElement;

				for (size_t i = 0; i < _nElement; i++)
				{
					CL_PLACEMENT_NEW(_pObjects + i, Element, OtherVector._pObjects[i]);
				}
			}
			else
			{
				Clear();
				ResizeImpl(OtherVector._Capacity);
				_nElement = OtherVector._nElement;

				for (size_t i = 0; i < _nElement; i++)
				{
					CL_PLACEMENT_NEW(_pObjects + i, Element, OtherVector._pObjects[i]);
				}
			}

			return *this;
		}
		VectorType(VectorType&& OtherVector) : 
			_Capacity(OtherVector._Capacity),
			_pObjects(OtherVector._pObjects),
			_nElement(OtherVector._nElement)
		{
			OtherVector._Capacity = 0;
			OtherVector._pObjects = nullptr;
			OtherVector._nElement = 0;
		}
		VectorType& operator = (VectorType&& OtherVector)
		{
			Clear();

			_Capacity = OtherVector._Capacity;
			_pObjects = OtherVector._pObjects;
			_nElement = OtherVector._nElement;

			OtherVector._Capacity = 0;
			OtherVector._pObjects = nullptr;
			OtherVector._nElement = 0;

			return *this;
		}
		VectorType(const std::initializer_list<Element>& Init)
		{
			if (Init.size())
			{
				Reserve(Init.size());

				for (auto& it : Init)
				{
					PushBack(it);
				}
			}
		}
		VectorType& operator = (const std::initializer_list<Element>& Init)
		{
			Clear();

			if (Init.size())
			{
				Reserve(Init.size());

				for (auto& it : Init)
				{
					PushBack(it);
				}
			}

			return *this;
		}
		VectorType(const std::initializer_list<const Element*>& Init)
		{
			//initializer_list must contain range of elements to copy and we fill them from memory.
			CL_ASSERT(Init.size() == 2);

			const Element* First = *Init.begin();
			const Element* Last = *(Init.begin() + 1);
			size_t Size = Last - First;

			if (Size)
			{
				Reserve(Size);

				while (First <= Last)
				{
					PushBack(*First);
					First++;
				}
			}

		}
		VectorType& operator = (const std::initializer_list<const Element*>& Init)
		{
			Clear();

			if (Init.size())
			{
				//initializer_list must contain range of elements to copy and we fill them from memory.
				CL_ASSERT(Init.size() == 2);

				const Element* First = *Init.begin();
				const Element* Last = *(Init.begin() + 1);
				size_t Size = Last - First;

				Reserve(Size);

				while (First <= Last)
				{
					PushBack(*First);
					First++;
				}
			}

			return *this;
		}
		template<class... Args>
		VectorType(size_t NumElements, Args... VArgs)
		{
			if (NumElements)
			{
				ResizeImpl(NumElements);

				for (size_t i = 0; i < NumElements; i++)
				{
					CL_PLACEMENT_NEW(_pObjects + i, Element, VArgs...);
				}

				_nElement = NumElements;
			}
		}
		template<class Lambda>
		void ForEach(Lambda& L)
		{
			for (size_t i = 0; i < _nElement; i++)
			{
				L(_pObjects[i]);
			}
		}
		template<class Lambda>
		void ForEach(Lambda& L) const
		{
			for (size_t i = 0; i < _nElement; i++)
			{
				L(_pObjects[i]);
			}
		}
		void PushBack(const Element& NewElement)
		{
			if (_Capacity <= _nElement)
			{
				ResizeImpl(_Capacity * 2 + 1);
			}

			CL_PLACEMENT_NEW(_pObjects + _nElement, Element, NewElement);
			_nElement++;
		}
		void EmplaceBack(Element& NewElement)
		{
			EmplaceBack(Move(NewElement));
		}
		void EmplaceBack(Element&& NewElement)
		{
			if (_Capacity <= _nElement)
			{
				ResizeImpl(_Capacity * 2 + 1);
			}

			CL_PLACEMENT_NEW(_pObjects + _nElement, Element, Forward(NewElement));
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
					if constexpr (std::is_move_constructible<Element>::value)
					{
						_pObjects[Index] = Move(_pObjects[LastElementIndex]);
					}
					else
					{
						_pObjects[Index] = _pObjects[LastElementIndex];
					}
				}
				else if constexpr (std::is_trivially_copyable<Element>::value)
				{
					memmove(_pObjects + Index, _pObjects + Index + 1, sizeof(Element) * (_nElement - Index - 1));
				}
				else
				{
					for (size_t i = Index; i < LastElementIndex; i++)
					{
						if constexpr (std::is_move_assignable<Element>::value)
						{
							_pObjects[i] = Move(_pObjects[i + 1]);
						}
						else
						{
							_pObjects[i] = _pObjects[i + 1];
						}
					}
				}
			}
			
			CL_PLACEMENT_DELETE(Element, _pObjects + LastElementIndex);
			_nElement--;

			if (!_nElement)
			{
				Clear();
			}
		}
		size_t GetCapacity() const { return _Capacity; }
		size_t GetSize() const { return _nElement; }
		void Truncate()
		{
			if (_nElement < _Capacity)
			{
				ResizeImpl(_nElement);
			}
		}
		void Reserve(size_t RequestedSize)
		{
			if (_Capacity < RequestedSize)
			{
				ResizeImpl(RequestedSize);
			}
		}
		template<class... Args>
		void Resize(size_t RequestedSize, Args... VArgs)
		{
			ResizeImpl(RequestedSize);

			if (_nElement < RequestedSize)
			{
				for (size_t i = _nElement; i < RequestedSize; i++)
				{
					CL_PLACEMENT_NEW(_pObjects + i, Element, VArgs...);
				}

				_nElement = RequestedSize;
			}
		}
		void Clear()
		{
			if (_pObjects)
			{
				for (size_t i = 0; i < _nElement; i++)
				{
					CL_PLACEMENT_DELETE(Element, _pObjects + i);
				}

				CL_FREE(_pObjects);
				_Capacity = 0;
				_nElement = 0;
				_pObjects = nullptr;
			}
		}
		Element& operator [](size_t Index)
		{
			CL_ASSERT(Index < _nElement);
			return _pObjects[Index];
		}
		const Element& operator [](size_t Index) const
		{
			CL_ASSERT(Index < _nElement);
			return _pObjects[Index];
		}
		ForwardIterator begin()
		{
			return ForwardIterator(_pObjects, _pObjects + _nElement);
		}
		ForwardIterator end()
		{
			return ForwardIterator();
		}
		ConstForwardIterator begin() const
		{
			return ConstForwardIterator(_pObjects, _pObjects + _nElement);
		}
		ConstForwardIterator end() const
		{
			return ConstForwardIterator();
		}
		unsigned long ComputeCrc32() const
		{
			return Crc32((const unsigned char*)_pObjects, sizeof(Element) * _nElement);
		}
		Element* GetData() { return _pObjects; }
		const Element* GetData() const { return _pObjects; }
		bool IsEmpty() const { return !_nElement; }
		~Vector()
		{
			Clear();
		}
	private:
		void ResizeImpl(size_t NewCapacity)
		{
			Element* pNewObjects = (Element*)CL_MALLOC(sizeof(Element) * NewCapacity);

			if (_nElement)
			{
				for (size_t i = 0; i < _nElement; i++)
				{
					if constexpr (std::is_move_constructible<Element>::value)
					{
						CL_PLACEMENT_NEW(pNewObjects + i, Element, CL::Move(_pObjects[i]));
					}
					else
					{ 
						CL_PLACEMENT_NEW(pNewObjects + i, Element, _pObjects[i]);
					}

					CL_PLACEMENT_DELETE(Element, _pObjects + i);
				}
			}

			CL_FREE(_pObjects);
			_pObjects = pNewObjects;
			_Capacity = NewCapacity;
		}

		Element* _pObjects = nullptr;
		size_t _Capacity = 0;
		size_t _nElement = 0;
	};
}