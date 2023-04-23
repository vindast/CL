#pragma once
#include <CLAssert.h>
#include <CLMemory.h>

//#define CL_DEBUG_MEMORY_LOOKUP_TABLE

#ifdef CL_DEBUG_MEMORY_LOOKUP_TABLE 
#include <iostream>
#endif //CL_DEBUG_MEMORY_LOOKUP_TABLE

namespace CL
{
	template<class ObjType, typename KeyType>
	class MemoryLookUpTable
	{
	public:
		MemoryLookUpTable(size_t BlockSize):
			_BlockSize(BlockSize)
		{

		}
		void Insert(ObjType* pValue, KeyType* pMemPtr, size_t size)
		{
			CL_ASSERT(pValue);
		//	CL_ASSERT(pMemPtr);
			CL_ASSERT(size >= _BlockSize);
			 
			size_t MemId = (size_t)pMemPtr; 
			size_t ptrMinBound = (MemId / _BlockSize) * _BlockSize;
			size_t ptrMaxBound = CL_ALIGN(MemId + size, _BlockSize);

#ifdef CL_DEBUG_MEMORY_LOOKUP_TABLE
			CL_ASSERT(ptrMinBound <= MemId);
			CL_ASSERT(MemId + size <= ptrMaxBound);
#endif  

			if (ptrMinBound < _MinBound || _MaxBound < ptrMaxBound)
			{
				size_t OldSize     = _Size;
				size_t OldMinBound = _MinBound;
				 
				_MinBound = CL_MIN(_MinBound, ptrMinBound);
				_MaxBound = CL_MAX(_MaxBound, ptrMaxBound);

				_Size = (_MaxBound - _MinBound) / _BlockSize + 1;
#ifdef CL_DEBUG_MEMORY_LOOKUP_TABLE
				CL_ASSERT(_MinBound <= MemId);
				CL_ASSERT(MemId + _Size <= _MaxBound);
				CL_ASSERT(_MinBound + _Size * _BlockSize == _MaxBound + _BlockSize);
				CL_DEBUG_ASSERT(_Size);
#endif  
				ObjType** pTmpTable = _pTable;

				_pTable = CL_NEW_ARR( ObjType*, _Size);
				CL_ASSERT(_pTable);

				memset(_pTable, 0, _Size * sizeof(ObjType*));

				if (pTmpTable)
				{
					size_t Offset = (OldMinBound - _MinBound) / _BlockSize; 

					memcpy(_pTable + Offset, pTmpTable, OldSize * sizeof(ObjType*));
					CL_DELETE_ARR(pTmpTable);
				}
			}

			size_t MinId = CL_ALIGN_DIVIDE(MemId - _MinBound, _BlockSize);
			size_t MaxId = (MemId + size - _MinBound )/ _BlockSize;


#ifdef CL_DEBUG_MEMORY_LOOKUP_TABLE 
			CL_ASSERT(_MinBound <= MemId);
			CL_ASSERT(MemId + size <= _MaxBound);
			CL_ASSERT(MaxId < _Size); 
#endif

			for (size_t i = MinId; i <= MaxId; i++)
			{
#ifdef CL_DEBUG_MEMORY_LOOKUP_TABLE  
				if (_pTable[i])
				{
					std::cout << "FAIL: i = " << i << ", MinId = " << MinId << ", MaxId = " << MaxId << ", size = " << _Size <<", % = " <<(MemId + size - _MinBound) % _BlockSize << std::endl;
				}
#endif

				CL_ASSERT(!_pTable[i]); 
				_pTable[i] = pValue;
			} 
		}
		ObjType* Find(const KeyType* ptr, bool (*compareFunction)(const ObjType*, const KeyType*))
		{ 
			size_t MemId = (size_t)ptr;

			if (MemId < _MinBound || _MaxBound < MemId)
			{
				return nullptr;
			}

			size_t Id = (MemId - _MinBound) / _BlockSize;

			if (!compareFunction(_pTable[Id], ptr))
			{
				Id++;

				if (Id < _Size && compareFunction(_pTable[Id], ptr))
				{
					return _pTable[Id];
				}
				else
				{
					return nullptr;
				}
			}
			else
			{
				return _pTable[Id];
			}
		}
		const ObjType* Find(const KeyType* ptr, bool (*compareFunction)(const ObjType*, const KeyType*)) const
		{
			size_t MemId = (size_t)ptr;

			if (MemId < _MinBound || _MaxBound < MemId)
			{
				return nullptr;
			}

			size_t Id = (MemId - _MinBound) / _BlockSize;

			if (!compareFunction(_pTable[Id], ptr))
			{
				Id++;

				if (Id < _Size && compareFunction(_pTable[Id], ptr))
				{
					return _pTable[Id];
				}
				else
				{
					return nullptr;
				}
			}
			else
			{
				return _pTable[Id];
			}
		}
		void Erase(const ObjType* pValue, const KeyType* pMemPtr)
		{
			size_t MemId = (size_t)pMemPtr;

			if (MemId < _MinBound || _MaxBound < MemId)
			{
#ifdef CL_DEBUG_MEMORY_LOOKUP_TABLE  
				for (size_t i = 0; i < _Size; i++)
				{
					CL_ASSERT(_pTable[i] != pValue);
				}
#endif

				return;
			}

			size_t StartId = (MemId - _MinBound + _BlockSize) / _BlockSize;

			for (size_t i = StartId; i < _Size; i++)
			{
				if (_pTable[i] != pValue)
				{
					break;
				}

				_pTable[i] = nullptr;
			}

			for (size_t i = StartId; i > 0; i--)
			{ 
				size_t Id = i - 1;

				if (_pTable[Id] != pValue)
				{
					break;
				}
				 
				_pTable[Id] = nullptr;
			}  

#ifdef CL_DEBUG_MEMORY_LOOKUP_TABLE  
			for (size_t i = 0; i < _Size; i++)
			{ 
				CL_ASSERT(_pTable[i] != pValue);
			}
#endif
		}
		size_t Size() const noexcept
		{
			return _Size;
		}
		ObjType* operator [](size_t id) 
		{
			CL_ASSERT(id < _Size);
			return _pTable[id];
		}
		const ObjType* operator [](size_t id) const
		{
			CL_ASSERT(id < _Size);
			return _pTable[id];
		}
		~MemoryLookUpTable()
		{ 
			if (_pTable)
			{
				CL_DELETE_ARR(_pTable);
			} 
		}
	private:
		ObjType** _pTable = nullptr;

		size_t _Size = 0;
		size_t _MinBound = size_t(-1);
		size_t _MaxBound = 0;

		size_t _BlockSize;
	};
};