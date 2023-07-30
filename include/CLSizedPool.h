#pragma once

#include "CLPool2.h"

//#define CL_DEBUG_POOL

#ifdef CL_DEBUG_POOL
#include <iostream>
#include <unordered_map>
#endif

namespace CL
{   
	template<class ObjType>
	struct __PoolSizedElementsBlock final
	{
		__PoolSizedElementsBlock(OneLinkedNode<ObjType>* pMemory, size_t ElementsCount) :
			_pMemory(pMemory),
			_ElementsCount(ElementsCount),
			_FreeElementsCount(ElementsCount),
			_pFirstFreeElement(pMemory)
		{
#ifdef CL_DEBUG_POOL
			CL_ASSERT(PoolElementsBlockSanityMap.find(this) == PoolElementsBlockSanityMap.end());
			PoolElementsBlockSanityMap.insert(std::make_pair(this, this));
#endif


			for (size_t i = 0; i < ElementsCount - 1; i++)
			{
				_pMemory[i].pNext = &_pMemory[i + 1];
			}

			_pMemory[ElementsCount - 1].pNext = nullptr;
		}
		template<class ...ConstructorParams>
		ObjType* Alloc(ConstructorParams... params)
		{
			CL_DEBUG_ASSERT(_FreeElementsCount);
			_FreeElementsCount--;

			ObjType* pObj = &_pFirstFreeElement->Obj;
			CL_PLACEMENT_NEW(pObj, ObjType, params...);

			_pFirstFreeElement = _pFirstFreeElement->pNext;

			return pObj;
		}
		void Free(ObjType* pObj)
		{
			CL_DEBUG_ASSERT(_FreeElementsCount < _ElementsCount);
			CL_DEBUG_ASSERT((char*)_pMemory <= (char*)pObj);
			CL_DEBUG_ASSERT((char*)pObj < (char*)_pMemory + sizeof(OneLinkedNode<ObjType>) * _ElementsCount);

			_FreeElementsCount++;

			size_t Index = size_t((char*)pObj - (char*)_pMemory) / sizeof(OneLinkedNode<ObjType>);

			CL_DEBUG_ASSERT(Index < _ElementsCount);

			_pMemory[Index].pNext = _pFirstFreeElement;
			_pFirstFreeElement = &_pMemory[Index];
		}
		bool IsFree() const
		{
			return _ElementsCount == _FreeElementsCount;
		}
		bool IsFull() const
		{
			return !_FreeElementsCount;
		}
		size_t TotalSize() const
		{
			return _ElementsCount * sizeof(OneLinkedNode<ObjType>);
		}
		const OneLinkedNode<ObjType>* Data() const
		{
			return _pMemory;
		}
		OneLinkedNode<ObjType>* Data()
		{
			return _pMemory;
		}
		~__PoolSizedElementsBlock()
		{
#ifdef CL_DEBUG_POOL
			CL_ASSERT(PoolElementsBlockSanityMap.find(this) != PoolElementsBlockSanityMap.end());
			PoolElementsBlockSanityMap.erase(this);
#endif
		}

		__PoolSizedElementsBlock<ObjType>* pNext = nullptr;
		__PoolSizedElementsBlock<ObjType>* pPrevios = nullptr;
	private:
		__PoolSizedElementsBlock<ObjType>& operator = (const __PoolSizedElementsBlock<ObjType>&) = delete;
		__PoolSizedElementsBlock<ObjType>(const __PoolSizedElementsBlock<ObjType>&) = delete;

		OneLinkedNode<ObjType>* _pMemory;
		OneLinkedNode<ObjType>* _pFirstFreeElement;
		size_t _ElementsCount;
		size_t _FreeElementsCount;
	};

	template<class ObjType>
	class SizedPool final
	{
	public:
		SizedPool(size_t ElementsInBlockCount) :
			_ElementsInBlockCount(ElementsInBlockCount),
			_BlockSize(sizeof(__PoolSizedElementsBlock<ObjType>) + sizeof(OneLinkedNode<ObjType>) * _ElementsInBlockCount),
			_LookUpTable(_BlockSize)
		{

		}
		template<class ...ConstructorParams>
		ObjType* Alloc(ConstructorParams... params)
		{
#ifdef CL_DEBUG_POOL
			BlockTotalValidation();
#endif

			ObjType* pObj = nullptr;

			if (_pFirstFreeBlock)
			{
				pObj = _pFirstFreeBlock->Alloc(params...);

				if (_pFirstFreeBlock->IsFull())
				{
					CL_DEBUG_ASSERT(!_pFirstFreeBlock->pPrevios);

					if (_pFirstFreeBlock->pNext)
					{
						_pFirstFreeBlock->pNext->pPrevios = nullptr;
					}

					__PoolSizedElementsBlock<ObjType>* pBlock = _pFirstFreeBlock;
					_pFirstFreeBlock = _pFirstFreeBlock->pNext;

					if (_pFirstFullBlock)
					{
						CL_DEBUG_ASSERT(!_pFirstFullBlock->pPrevios);
						_pFirstFullBlock->pPrevios = pBlock;
					}

					pBlock->pNext = _pFirstFullBlock;
					_pFirstFullBlock = pBlock;
				}

#ifdef CL_DEBUG_POOL
				BlockTotalValidation();
#endif
			}
			else
			{
				_pFirstFreeBlock = (__PoolSizedElementsBlock<ObjType>*)(CL_MALLOC(_BlockSize));

				OneLinkedNode<ObjType>* pObjectsMemory = (OneLinkedNode<ObjType>*)((char*)(_pFirstFreeBlock)+sizeof(__PoolSizedElementsBlock<ObjType>));

				CL_PLACEMENT_NEW(_pFirstFreeBlock, __PoolSizedElementsBlock<ObjType>, pObjectsMemory, _ElementsInBlockCount);

				_LookUpTable.Insert(_pFirstFreeBlock, (char*)_pFirstFreeBlock->Data(), _BlockSize);

				pObj = _pFirstFreeBlock->Alloc(params...);


#ifdef CL_DEBUG_POOL
				BlockTotalValidation();
#endif

			}

			return pObj;
		}
		bool IsFromThisPool(const ObjType* pObj) const { return pObj && _LookUpTable.Find((char*)pObj, Compare); }
		void Free(ObjType* pObj)
		{
			if (!pObj)
			{
				return;
			}

#ifdef CL_DEBUG_POOL
			BlockTotalValidation();
#endif

			__PoolSizedElementsBlock<ObjType>* pNode = _LookUpTable.Find((char*)pObj, Compare);
			CL_DEBUG_ASSERT(pNode);

			bool bIsFull = pNode->IsFull();
			pNode->Free(pObj);

			bool bDestroyPoolNode = false;

			if (bIsFull)
			{
				if (_pFirstFullBlock == pNode)
				{
					_pFirstFullBlock = pNode->pNext;

					CL_DEBUG_ASSERT(!_pFirstFullBlock || _pFirstFullBlock->IsFull());
					CL_DEBUG_ASSERT(!_pFirstFullBlock || !_pFirstFullBlock->pNext || _pFirstFullBlock->pNext->IsFull());
				}

				if (pNode->pNext)
				{
					CL_DEBUG_ASSERT(!pNode->pPrevios || pNode->pPrevios->IsFull());
					CL_DEBUG_ASSERT(pNode->pNext->IsFull());
					pNode->pNext->pPrevios = pNode->pPrevios;
				}

				if (pNode->pPrevios)
				{
					CL_DEBUG_ASSERT(!pNode->pNext || pNode->pNext->IsFull());
					CL_DEBUG_ASSERT(pNode->pPrevios->IsFull());
					pNode->pPrevios->pNext = pNode->pNext;
				}

#ifdef CL_DEBUG_POOL

				auto pTestNode = _pFirstFullBlock;
				size_t count = 0;
				size_t index = 0;


				while (pTestNode)
				{
					count += !pTestNode->IsFull();

					if (!pTestNode->IsFull() && pTestNode == pNode)
					{
						std::cout << "wow wow, index = " << index << std::endl;
					}

					index++;
					pTestNode = pTestNode->pNext;
				}

				BlockListValidation();
#endif

				if (_pFirstFreeBlock)
				{
					CL_DEBUG_ASSERT(!_pFirstFreeBlock->pPrevios);
					_pFirstFreeBlock->pPrevios = pNode;
				}

				pNode->pNext = _pFirstFreeBlock;
				pNode->pPrevios = nullptr;

				_pFirstFreeBlock = pNode;

				CL_DEBUG_ASSERT(!_pFirstFreeBlock->pPrevios);
				CL_DEBUG_ASSERT(!_pFirstFullBlock || !_pFirstFullBlock->pPrevios);

#ifdef CL_DEBUG_POOL
				BlockTotalValidation();
#endif
			}
			else if (pNode->IsFree())
			{
				if (_pFirstFreeBlock == pNode)
				{
					_pFirstFreeBlock = pNode->pNext;
				}

				if (pNode->pPrevios)
				{
					pNode->pPrevios->pNext = pNode->pNext;
				}

				if (pNode->pNext)
				{
					pNode->pNext->pPrevios = pNode->pPrevios;
				}

#ifdef CL_DEBUG_POOL
				BlockTotalValidation();
#endif

				bDestroyPoolNode = true;
			}

			CL_PLACEMENT_DELETE(ObjType, pObj);

			if (bDestroyPoolNode)
			{
				_LookUpTable.Erase(pNode, (char*)pNode->Data());
				CL_PLACEMENT_DELETE(__PoolSizedElementsBlock<ObjType>, pNode);
				CL_FREE(pNode);
			}

#ifdef CL_DEBUG_POOL
			BlockTotalValidation();
#endif
		}
		~SizedPool()
		{
#ifdef CL_DEBUG_POOL
			BlockTotalValidation();
#endif

			__PoolSizedElementsBlock<ObjType>* pNode = _pFirstFreeBlock;

			while (pNode)
			{
				__PoolSizedElementsBlock<ObjType>* pNodeToFree = pNode;
				pNode = pNode->pNext;
				CL_PLACEMENT_DELETE(__PoolSizedElementsBlock<ObjType>, pNodeToFree);
				CL_FREE(pNodeToFree);
			}

			pNode = _pFirstFullBlock;

			while (pNode)
			{
				__PoolSizedElementsBlock<ObjType>* pNodeToFree = pNode;
				pNode = pNode->pNext;
				CL_PLACEMENT_DELETE(__PoolSizedElementsBlock<ObjType>, pNodeToFree);
				CL_FREE(pNodeToFree);
			}
		}
	private:
		SizedPool<ObjType>& operator = (const SizedPool<ObjType>&) = delete;
		SizedPool<ObjType>(const SizedPool<ObjType>&) = delete;

		static bool Compare(const __PoolSizedElementsBlock<ObjType>* pNode, const char* ptr)
		{
			if (!pNode)
			{
				return false;
			}

			return (char*)pNode->Data() <= ptr && ptr < ((char*)pNode->Data() + pNode->TotalSize());
		};

#ifdef CL_DEBUG_POOL
		void BlockListValidation(__PoolElementsBlock<ObjType>* pNodeNotInList = nullptr)
		{
			__PoolElementsBlock<ObjType>* pNode = _pFirstFreeBlock;

			std::unordered_map< __PoolElementsBlock<ObjType>*, __PoolElementsBlock<ObjType>*> stack;

			while (pNode)
			{
				CL_ASSERT(pNodeNotInList != pNode);
				CL_ASSERT(!pNode->IsFull());
				CL_ASSERT(!pNode->IsFree());
				CL_ASSERT(stack.find(pNode->pNext) == stack.end());
				CL_ASSERT(stack.find(pNode) == stack.end());
				stack.insert(std::make_pair(pNode, pNode));
				pNode = pNode->pNext;
			}

			pNode = _pFirstFullBlock;

			while (pNode)
			{
				CL_ASSERT(pNodeNotInList != pNode);
				CL_ASSERT(pNode->IsFull());
				CL_ASSERT(!pNode->IsFree());
				CL_ASSERT(stack.find(pNode->pNext) == stack.end());
				CL_ASSERT(stack.find(pNode) == stack.end());
				stack.insert(std::make_pair(pNode, pNode));
				pNode = pNode->pNext;
			}
		}
		void BlockTotalValidation()
		{
			__PoolElementsBlock<ObjType>* pNode = _pFirstFreeBlock;

			std::unordered_map< __PoolElementsBlock<ObjType>*, __PoolElementsBlock<ObjType>*> stack;

			while (pNode)
			{
				CL_ASSERT(!pNode->IsFull());
				CL_ASSERT(!pNode->IsFree());
				CL_ASSERT(stack.find(pNode->pNext) == stack.end());
				CL_ASSERT(stack.find(pNode) == stack.end());
				stack.insert(std::make_pair(pNode, pNode));


				ValidateBlock(pNode);
				pNode = pNode->pNext;
			}

			pNode = _pFirstFullBlock;

			while (pNode)
			{
				CL_ASSERT(pNode->IsFull());
				CL_ASSERT(!pNode->IsFree());
				CL_ASSERT(stack.find(pNode->pNext) == stack.end());
				CL_ASSERT(stack.find(pNode) == stack.end());
				stack.insert(std::make_pair(pNode, pNode));

				ValidateBlock(pNode);
				pNode = pNode->pNext;
			}
		}
		void ValidateBlock(__PoolElementsBlock<ObjType>* pBlock) const
		{
			CL_ASSERT(PoolElementsBlockSanityMap.find(pBlock) != PoolElementsBlockSanityMap.end());

			if (pBlock->pNext)
			{
				CL_ASSERT(pBlock->pNext->pPrevios == pBlock);
				CL_ASSERT(PoolElementsBlockSanityMap.find(pBlock->pNext) != PoolElementsBlockSanityMap.end());
				CL_ASSERT(pBlock->IsFull() == pBlock->pNext->IsFull());
				CL_ASSERT(!pBlock->pNext->IsFree());
			}

			if (pBlock->pPrevios)
			{
				CL_ASSERT(pBlock->pPrevios->pNext == pBlock);
				CL_ASSERT(PoolElementsBlockSanityMap.find(pBlock->pPrevios) != PoolElementsBlockSanityMap.end());
				CL_ASSERT(pBlock->IsFull() == pBlock->pPrevios->IsFull());
				CL_ASSERT(!pBlock->pPrevios->IsFree());
			}
		}
#endif

		size_t _ElementsInBlockCount;
		size_t _BlockSize;

		MemoryLookUpTable<__PoolSizedElementsBlock<ObjType>, char> _LookUpTable;

		__PoolSizedElementsBlock<ObjType>* _pFirstFreeBlock = nullptr;
		__PoolSizedElementsBlock<ObjType>* _pFirstFullBlock = nullptr;

	};
};