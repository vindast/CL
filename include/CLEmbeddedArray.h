#pragma once
#include <CLAssert.h>

namespace CL
{
	template<class ObjType>
	class VectorIterator
	{
		typedef VectorIterator<ObjType> Iterator;
		template<class ObjType, size_t>
		friend class EmbeddedArray;
	public:
		Iterator& operator++(int)
		{
			_Obj++;

			if (_Obj > _LastObj)
			{
				_Obj = nullptr;
				_LastObj = nullptr;
			}

			return *this;
		}
		Iterator& operator++()
		{
			_Obj++;

			if (_Obj > _LastObj)
			{
				_Obj = nullptr;
				_LastObj = nullptr;
			}

			return *this;
		}
		ObjType& operator*()
		{
			return *_Obj;
		}
		ObjType& operator ()()
		{
			return *_Obj;
		}
		auto& operator ->()
		{
			return *_Obj;
		}
		operator bool() const
		{
			return _Obj;
		}
		bool operator == (const Iterator& other) const
		{
			return _Obj == other._Obj;
		}
		bool operator != (const Iterator& other) const
		{
			return _Obj != other._Obj;
		}
	private:
		ObjType* _Obj = nullptr;
		ObjType* _LastObj = nullptr;
	};

	template<class ObjType, size_t Size>
	class EmbeddedArray
	{
	public:
		typedef EmbeddedArray<ObjType, Size> ArrayType;
		typedef VectorIterator<ObjType> ForwardIterator;

		EmbeddedArray()
		{
			static_assert(Size, "Size must be greater then zero.");
		}
		ForwardIterator begin()
		{
			ForwardIterator it;
			if (_NumElements)
			{
				it._Obj = GetObjMemory(0);
				it._LastObj = GetObjMemory(_NumElements - 1);
			}
			return it;
		}
		ForwardIterator end() const { return ForwardIterator(); }
		size_t GetElementsCount() const { return _NumElements; }
		size_t GetCapacity() const { return Size; }
		ForwardIterator EraseSwap(const ForwardIterator& It)
		{
			size_t Index = It._Obj - GetObjMemory(0);

			CL_ASSERT(Index < _NumElements);

			ObjType* pObj = GetObjMemory(Index);
			pObj->~ObjType();

			size_t LastElementIndex = _NumElements - 1;

			if (Index != LastElementIndex)
			{
				ObjType* pLastObj = GetObjMemory(LastElementIndex);
				new(pObj) ObjType(Forward(*GetObjMemory(LastElementIndex)));
				pLastObj->~ObjType();
			}

			_NumElements--;
			ForwardIterator NewIt;
			if (Index < _NumElements)
			{
				NewIt._Obj = GetObjMemory(Index);
				NewIt._LastObj = GetObjMemory(_NumElements - 1);
			}
			return NewIt;
		}
		void EraseSwap(size_t Index)
		{
			CL_ASSERT(Index < _NumElements);

			ObjType* pObj = GetObjMemory(Index);
			pObj->~ObjType();

			size_t LastElementIndex = _NumElements - 1;
			
			if (Index != LastElementIndex)
			{
				ObjType* pLastObj = GetObjMemory(LastElementIndex);
				new(pObj) ObjType(Forward(*GetObjMemory(LastElementIndex)));
				pLastObj->~ObjType();
			}

			_NumElements--;
		}
		template<class... Args>
		void PushBack(Args&... args)
		{
			CL_ASSERT(_NumElements < Size);
			ObjType* pObj = GetObjMemory(_NumElements++);
			new(pObj) ObjType(args...);
		}
		void PushBack(const ObjType& Obj)
		{
			CL_ASSERT(_NumElements < Size);
			ObjType* pObj = GetObjMemory(_NumElements++);
			new(pObj) ObjType(Obj);
		}
		void PushBack(ObjType&& Obj)
		{
			CL_ASSERT(_NumElements < Size);
			ObjType* pObj = GetObjMemory(_NumElements++);
			new(pObj) ObjType(Forward(Obj));
		}
		void Clear()
		{
			for (size_t i = 0; i < _NumElements; i++)
			{
				ObjType* pObj = GetObjMemory(i);
				pObj->~ObjType();
			}

			_NumElements = 0;
		}
		const ObjType& operator [](size_t Index) const
		{
			CL_ASSERT(Index < _NumElements);
			return *GetObjMemory(Index);
		}
		ObjType& operator [](size_t Index)
		{
			CL_ASSERT(Index < _NumElements);
			return *GetObjMemory(Index);
		}
		~EmbeddedArray()
		{
			Clear();
		}
	private:
		__forceinline CL_NO_DISCARD ObjType* GetObjMemory(size_t Index) { return (ObjType*)(&_mObjMemory[sizeof(ObjType) * Index]); }

		char _mObjMemory[sizeof(ObjType) * Size];
		size_t _NumElements = 0;

		ArrayType(const ArrayType&) = delete;
		ArrayType& operator = (const ArrayType&) = delete;
	};
}