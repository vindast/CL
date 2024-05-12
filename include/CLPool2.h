#pragma once

#include "CLMemoryLookUpTable.h"

// #TODO CLConfig
//#define CL_DEBUG_POOL

#ifdef CL_DEBUG_POOL
#include <iostream>
#include <unordered_map>
#endif
 
namespace CL
{
	template<class ObjType>
	struct OneLinkedNode
	{
		ObjType Obj;
		OneLinkedNode<ObjType>* pNext = nullptr;
	};

	template<class ObjType>
	struct TwoLinkedNode
	{
		TwoLinkedNode() = default;
		TwoLinkedNode(const ObjType& InObj) : Obj(InObj)
		{

		}
		TwoLinkedNode(ObjType&& InObj) : Obj(Forward(InObj))
		{

		}

		ObjType Obj;
		TwoLinkedNode<ObjType>* pPrevios = nullptr;
		TwoLinkedNode<ObjType>* pNext = nullptr;
	};


#ifdef CL_DEBUG_POOL 
	static std::unordered_map<void*, void*> PoolElementsBlockSanityMap;
#endif

	template<class ObjType>
	struct __PoolElementsBlock final
	{ 
		__PoolElementsBlock(TwoLinkedNode<ObjType>* pMemory, size_t ElementsCount):
			_pMemory(pMemory), 
			_ElementsCount(ElementsCount), 
			_FreeElementsCount(ElementsCount),
			_pFirstFreeElement(pMemory),
			_pFirstUsedElement(nullptr)
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
		ObjType* Alloc()
		{
			CL_DEBUG_ASSERT(_FreeElementsCount);
			_FreeElementsCount--;

			TwoLinkedNode<ObjType>* pCurrentNode = _pFirstFreeElement;
			ObjType* pObj = &_pFirstFreeElement->Obj;
			_pFirstFreeElement = _pFirstFreeElement->pNext;

			if (_pFirstUsedElement)
			{
				_pFirstUsedElement->pPrevios = pCurrentNode;
			}

			pCurrentNode->pNext = _pFirstUsedElement;

			_pFirstUsedElement = pCurrentNode;
			pCurrentNode->pPrevios = nullptr;

			return pObj;
		} 
		void Free(ObjType* pObj)
		{
			CL_DEBUG_ASSERT(_FreeElementsCount < _ElementsCount);
			CL_DEBUG_ASSERT((char*)_pMemory <= (char*)pObj);
			CL_DEBUG_ASSERT((char*)pObj < (char*)_pMemory + CL_ALIGN(sizeof(TwoLinkedNode<ObjType>), alignof(TwoLinkedNode<ObjType>)) * _ElementsCount);

			_FreeElementsCount++; 

			size_t Index = size_t((char*)pObj - (char*)_pMemory) / CL_ALIGN(sizeof(TwoLinkedNode<ObjType>), alignof(TwoLinkedNode<ObjType>));

			CL_DEBUG_ASSERT(Index < _ElementsCount);

			TwoLinkedNode<ObjType>* pObjNode = &_pMemory[Index];

			if (pObjNode->pNext)
			{
				pObjNode->pNext->pPrevios = pObjNode->pPrevios;
			}

			if (pObjNode->pPrevios)
			{
				pObjNode->pPrevios->pNext = pObjNode->pNext;
			}

			if (pObjNode == _pFirstUsedElement)
			{
				_pFirstUsedElement = pObjNode->pNext;
			}

			_pMemory[Index].pNext = _pFirstFreeElement; 
			_pFirstFreeElement = pObjNode;
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
			return _ElementsCount * sizeof(TwoLinkedNode<ObjType>);
		}
		const TwoLinkedNode<ObjType>* Data() const
		{
			return _pMemory;
		}
		TwoLinkedNode<ObjType>* Data()
		{
			return _pMemory;
		}
		~__PoolElementsBlock()
		{
#ifdef CL_DEBUG_POOL
			CL_ASSERT(PoolElementsBlockSanityMap.find(this) != PoolElementsBlockSanityMap.end());
			PoolElementsBlockSanityMap.erase(this);
#endif

			while (_pFirstUsedElement)
			{
				auto pCurrentNode = _pFirstUsedElement;
				_pFirstUsedElement = _pFirstUsedElement->pNext;

				CL_PLACEMENT_DELETE(ObjType, &pCurrentNode->Obj);
			}
		}

		__PoolElementsBlock<ObjType>* pNext    = nullptr;
		__PoolElementsBlock<ObjType>* pPrevios = nullptr;
	private:
		__PoolElementsBlock<ObjType>& operator = (const __PoolElementsBlock<ObjType>&) = delete;
		__PoolElementsBlock<ObjType>(const __PoolElementsBlock<ObjType>&) = delete;

		TwoLinkedNode<ObjType>* _pMemory;
		TwoLinkedNode<ObjType>* _pFirstFreeElement;
		TwoLinkedNode<ObjType>* _pFirstUsedElement;
		size_t _ElementsCount;
		size_t _FreeElementsCount;
	}; 

	template<class ObjType>
	class Pool final
	{
	public:
		Pool(size_t ElementsInBlockCount) :
			_ElementsInBlockCount(ElementsInBlockCount),
			_BlockSize(CL_ALIGN(sizeof(__PoolElementsBlock<ObjType>), alignof(TwoLinkedNode<ObjType>)) + CL_ALIGN(sizeof(TwoLinkedNode<ObjType>), alignof(TwoLinkedNode<ObjType>)) * _ElementsInBlockCount),
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
				pObj = _pFirstFreeBlock->Alloc();
				 
				if (_pFirstFreeBlock->IsFull())
				{
					CL_DEBUG_ASSERT(!_pFirstFreeBlock->pPrevios);
					 
					if (_pFirstFreeBlock->pNext)
					{
						_pFirstFreeBlock->pNext->pPrevios = nullptr;
					}

					__PoolElementsBlock<ObjType>* pBlock = _pFirstFreeBlock; 
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
				_pFirstFreeBlock = (__PoolElementsBlock<ObjType>*)(CL_MALLOC(_BlockSize));

				TwoLinkedNode<ObjType>* pObjectsMemory = (TwoLinkedNode<ObjType>*)((char*)(_pFirstFreeBlock)+CL_ALIGN(sizeof(__PoolElementsBlock<ObjType>), alignof(TwoLinkedNode<ObjType>)));

				CL_PLACEMENT_NEW(_pFirstFreeBlock, __PoolElementsBlock<ObjType>, pObjectsMemory, _ElementsInBlockCount);

				_LookUpTable.Insert(_pFirstFreeBlock, (char*)_pFirstFreeBlock->Data(), _BlockSize);

				pObj = _pFirstFreeBlock->Alloc();

#ifdef CL_DEBUG_POOL
				BlockTotalValidation();
#endif
			} 

			CL_PLACEMENT_NEW(pObj, ObjType, params...);
			return pObj;
		}
		void Free(ObjType* pObj)
		{
			if (!pObj)
			{
				return;
			}

#ifdef CL_DEBUG_POOL
			BlockTotalValidation();
#endif

			__PoolElementsBlock<ObjType>* pNode = _LookUpTable.Find((char*)pObj, Compare);
			CL_DEBUG_ASSERT(pNode);

			CL_PLACEMENT_DELETE(ObjType, pObj);

			bool bIsFull = pNode->IsFull();
			pNode->Free(pObj);

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

				_LookUpTable.Erase(pNode, (char*)pNode->Data());
				CL_PLACEMENT_DELETE(__PoolElementsBlock<ObjType>,pNode);
				CL_FREE(pNode);
			}

#ifdef CL_DEBUG_POOL
			BlockTotalValidation();
#endif
		}
		~Pool()
		{
#ifdef CL_DEBUG_POOL
			BlockTotalValidation();
#endif

			__PoolElementsBlock<ObjType>* pNode = _pFirstFreeBlock;

			while (pNode)
			{
				__PoolElementsBlock<ObjType>* pNodeToFree = pNode;
				pNode = pNode->pNext;
				CL_PLACEMENT_DELETE(__PoolElementsBlock<ObjType>,pNodeToFree);
				CL_FREE(pNodeToFree);
			} 

			pNode = _pFirstFullBlock;

			while (pNode)
			{
				__PoolElementsBlock<ObjType>* pNodeToFree = pNode;
				pNode = pNode->pNext;
				CL_PLACEMENT_DELETE(__PoolElementsBlock<ObjType>,pNodeToFree);
				CL_FREE(pNodeToFree);
			}
		}
		size_t GetElementsInBlock() const
		{
			return _ElementsInBlockCount;
		}
	private:
		Pool<ObjType>& operator = (const Pool<ObjType>&) = delete;
		Pool<ObjType>(const Pool<ObjType>&) = delete;

		static bool Compare(const __PoolElementsBlock<ObjType>* pNode, const char* ptr)
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

		MemoryLookUpTable<__PoolElementsBlock<ObjType>, char> _LookUpTable;

		__PoolElementsBlock<ObjType>* _pFirstFreeBlock = nullptr;
		__PoolElementsBlock<ObjType>* _pFirstFullBlock = nullptr;

	}; 
};