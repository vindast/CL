#pragma once
#include <unordered_map>
#include <tuple>
#include "CLHeapAllocator.h" 

//#define CL_DEBUG_BYTE_MEM_ALLOCATOR
//#define CL_DEBUG_BYTE_MEM_ALLOCATOR_TREE

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
#include <iostream>
#endif

namespace CL
{  
	class DefaultMemoryObject
	{
	public:
		DefaultMemoryObject(size_t Size): _Size(Size)
		{
			_pData = (char*)malloc(Size); 
			CL_ASSERT(Size);
		}
		char* Data() const { return _pData; };
		size_t MaxSize() const
		{
			return _Size;
		}
		~DefaultMemoryObject()
		{
			free(_pData);
		}
	private:
		DefaultMemoryObject(const DefaultMemoryObject&) = delete;
		DefaultMemoryObject& operator = (const DefaultMemoryObject&) = delete;

		char* _pData = nullptr;
		size_t _Size;
	}; 

	template<typename ValueType, class MemoryObject>
	struct HeapRefPtr
	{
		ValueType pData = nullptr;
		MemoryObject* pMemObject = nullptr;
		MemControlBlock* pMemBlock = nullptr;

		void MarkFree()
		{
			pData      = nullptr;
			pMemObject = nullptr;
			pMemBlock  = nullptr;
		}
		operator bool () const { return pData; };
	};

	template<typename ValueType, class MemoryObject, class... Args>
	class HeapRefAllocator final
	{
		typedef AblTreeNode<size_t, MemControlBlock*>* TreeNodePtr;
	public:  
		HeapRefAllocator(
			Args... params, 
			size_t TotalSize = BytesFromMB(16),
			size_t MaxDirectAllocationSize = BytesFromMB(2), 
			size_t BaseTreeNodePoolSize = 512,
			size_t MemControlSectionPoolBaseSize = 64, 
			size_t BlockAlign = 32
		) :
			_FreeMemSectionsBySize(BaseTreeNodePoolSize),
			_TotalSize(TotalSize),
			_LookupTable(TotalSize),
			_MaxDirectAllocationSize(MaxDirectAllocationSize),
			_BlockAlign(BlockAlign),
			_ControlSectionPool(MemControlSectionPoolBaseSize),
			_MemObjectConstructorParams(params...)
		{

		}
		void CreateNewBlock(size_t size)
		{
			size = CL_ALIGN(size, _TotalSize);
			size += sizeof(MemBlockData);
			size = CL_ALIGN(size, _BlockAlign); 
			 
			MemoryObject* pObject = CreateMemObj(size);
 
			MemControlBlock* pFreeMemBlock = _ControlSectionPool.Alloc(); 

			pFreeMemBlock->Size  = pObject->MaxSize();
			pFreeMemBlock->bFree = true;
			pFreeMemBlock->pData = pObject->Data();

			InsertToMemBlockTree(pFreeMemBlock);

			_LookupTable.Insert(pObject, pObject->Data(), pObject->MaxSize());
		} 
		HeapRefPtr<ValueType, MemoryObject> Allocate(size_t size, size_t alignment)
		{
			CL_DEBUG_ASSERT(alignment);
			CL_DEBUG_ASSERT(size);

			size_t AlignedSize = CL_ALIGN(size, alignment);
			AlignedSize = CL_ALIGN(AlignedSize, _BlockAlign);

			if (AlignedSize > _MaxDirectAllocationSize)
			{ 
				MemoryObject* pObject = CreateMemObj(AlignedSize);

				HeapRefPtr<ValueType, MemoryObject> ptr;
				ptr.pMemBlock = nullptr; 
				ptr.pMemObject = pObject;
				ptr.pData     = pObject->Data();

				_IndirectAllocMap.insert(std::make_pair(pObject->Data(), pObject));

				return ptr;
			}

			TreeNodePtr pNode = FindMemBlock(_FreeMemSectionsBySize.GetRoot(), AlignedSize);

			if (!pNode)
			{
				CreateNewBlock(AlignedSize);
				pNode = FindMemBlock(_FreeMemSectionsBySize.GetRoot(), AlignedSize);

				CL_DEBUG_ASSERT(pNode);
				//	return nullptr;
			}

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
			nMemAllock++;
#endif 

			CL_DEBUG_ASSERT(pNode->Key >= AlignedSize);
			CL_DEBUG_ASSERT(pNode->Value);

			MemControlBlock* pBlock = pNode->Value;

			if (!pBlock->pNextFreeBlock)
			{
				_FreeMemSectionsBySize.Remove(pNode->Key);
			}
			else
			{
				pNode->Value = pBlock->pNextFreeBlock;
				pNode->Value->pPreviosFreeBlock = nullptr;
				pBlock->pNextFreeBlock = nullptr;

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
				Validate();
#endif
			}

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR
			ValidateBlock(pBlock, true, true);
			ValidateBlockNotInTree(pBlock);
			Validate();
#endif 

			if (pBlock->Size > AlignedSize)
			{
				char* pData = pBlock->pData + AlignedSize;
				MemControlBlock* pFreeMemBlock = _ControlSectionPool.Alloc();
				pFreeMemBlock->Size = pBlock->Size - AlignedSize;
				pFreeMemBlock->pData = pData;
				pFreeMemBlock->bFree = true;

				pFreeMemBlock->pPreviosBlock = pBlock;

				if (pBlock->pNextBlock)
				{
					pBlock->pNextBlock->pPreviosBlock = pFreeMemBlock;
					pFreeMemBlock->pNextBlock = pBlock->pNextBlock;
				}

				pBlock->pNextBlock = pFreeMemBlock;
				pBlock->Size = AlignedSize;

				InsertToMemBlockTree(pFreeMemBlock);
			}

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
			CL_ASSERT(SanityMap.find(pBlock) != SanityMap.end());
			CL_ASSERT(pBlock);
			CL_ASSERT(!pBlock->pNextFreeBlock);
			CL_ASSERT(!pBlock->pPreviosFreeBlock);
#endif

			pBlock->bFree = false;

			HeapRefPtr<ValueType, MemoryObject> ptr;
			ptr.pMemObject = _LookupTable.Find(pBlock->pData, Compare);
			ptr.pData = pBlock->pData;
			ptr.pMemBlock = pBlock;

			return ptr;
		}
		bool Free(const HeapRefPtr<ValueType, MemoryObject>& ptr)
		{
			if (!ptr.pData)
			{
				return false;
			}

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
			nMemFree++;
#endif
			char* a = (char*)ptr.pData;
			MemoryObject* data = _LookupTable.Find(a, Compare);

			if (!data)
			{
				auto it = _IndirectAllocMap.find(ptr.pData); 
				delete it->second; 
				_IndirectAllocMap.erase(it);

				return true;
			}

			MemControlBlock* pBlock = ptr.pMemBlock;


#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
			CL_ASSERT(!pBlock->pNextFreeBlock);
			CL_ASSERT(!pBlock->pPreviosFreeBlock);
			ValidateBlock(pBlock, false, false);
			ValidateBlockNotInTree(pBlock);
			Validate();
#endif 

			pBlock->bFree = true;
			MergeBlock(pBlock);

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
			nRealMemFree++;
#endif

			return true;
		}
		bool IsFree() const
		{
			CL_DEBUG_ASSERT(_FreeMemSectionsBySize.GetRoot());
			return _FreeMemSectionsBySize.GetRoot()->Key == _TotalSize;
		}
		size_t Size() const
		{
			return _TotalSize;
		}
		~HeapRefAllocator()
		{
			MemoryObject* pData = nullptr;

			for (size_t i = 0; i < _LookupTable.Size(); i++)
			{
				if (pData != _LookupTable[i])
				{
					pData = _LookupTable[i];

					if (!pData)
					{
						continue;
					} 

					delete pData; 
				}
			}

			for (auto it : _IndirectAllocMap)
			{
				delete it.second;
			}
		}
	private:
		HeapRefAllocator<ValueType, MemoryObject, Args...>& operator = (const HeapRefAllocator<ValueType, MemoryObject, Args...>&) = delete;
		HeapRefAllocator<ValueType, MemoryObject, Args...>(const HeapRefAllocator<ValueType, MemoryObject, Args...>&) = delete;

		static bool Compare(const MemoryObject* Data, const char* ptr)
		{
			if (!Data)
			{
				return false;
			}

			return Data->Data() <= ptr && ptr < (Data->Data() + Data->MaxSize());
		};
		static TreeNodePtr FindMemBlock(TreeNodePtr pNode, size_t Key)
		{
			if (!pNode)
			{
				return nullptr;
			}

			size_t s0 = pNode->Key >= Key ? pNode->Key - Key : -1;
			size_t s1 = pNode->pLeft && pNode->pLeft->Key >= Key ? pNode->pLeft->Key - Key : -1;
			size_t s2 = pNode->pRight && pNode->pRight->Key >= Key ? pNode->pRight->Key - Key : -1;

			if (s0 < s1 && s0 < s2)
			{
				return pNode;
			}

			if (s1 < s2)
			{
				return FindMemBlock(pNode->pLeft, Key);
			}

			if (s0 < size_t(-1) && s1 < size_t(-1) && s2 < size_t(-1))
			{
				if (pNode->Key > Key)
				{
					if (pNode->pRight)
					{
						return FindMemBlock(pNode->pRight, Key);
					}
					else
					{
						return pNode;
					}
				}
				else
				{
					if (pNode->pLeft)
					{
						return FindMemBlock(pNode->pLeft, Key);
					}
					else
					{
						return pNode;
					}
				}
			}

			return FindMemBlock(pNode->pRight, Key);
		} 
		void InsertToMemBlockTree(MemControlBlock* pBlock)
		{
#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR  
			CL_ASSERT(pBlock);
			CL_ASSERT(pBlock->bFree);
			CL_ASSERT(!pBlock->pNextFreeBlock);
			CL_ASSERT(!pBlock->pPreviosFreeBlock);
			CL_ASSERT(SanityMap.find(pBlock) != SanityMap.end());
			ValidateBlockNotInTree(pBlock);
			Validate();
			ValidateBlock(pBlock, false, false);
#endif 
			auto pNode = _FreeMemSectionsBySize.Find(pBlock->Size);

			if (!pNode)
			{
				_FreeMemSectionsBySize.Insert(pBlock->Size, pBlock);

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR
				Validate();
				ValidateBlock(pBlock, false, false);
#endif
			}
			else
			{
#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR
				Validate();
				ValidateBlock(pBlock, false, false);
				CL_ASSERT(!pNode->Value->pPreviosFreeBlock);
#endif

				pNode->Value->pPreviosFreeBlock = pBlock;
				pBlock->pNextFreeBlock = pNode->Value;
				pNode->Value = pBlock;

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR
				Validate();
				ValidateBlock(pBlock, false, false);
#endif
			}
		}
		void MergeBlock(MemControlBlock* pTargetBlock)
		{
#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR
			ValidateBlockNotInTree(pTargetBlock);
			Validate();
			ValidateBlock(pTargetBlock, false, false);
#endif 

			if (pTargetBlock->pNextBlock && pTargetBlock->pNextBlock->bFree)
			{
				MemControlBlock* pMergeBlock = pTargetBlock->pNextBlock;

				if (pMergeBlock->pNextBlock)
				{
					pMergeBlock->pNextBlock->pPreviosBlock = pTargetBlock;
				}

				pTargetBlock->pNextBlock = pMergeBlock->pNextBlock;
				pTargetBlock->Size += pMergeBlock->Size;

				MergeDataFreePtrDataInBlock(pMergeBlock);

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
				ValidateBlockNotInTree(pMergeBlock);
				ValidateBlock(pTargetBlock, true, false);
#endif 
				 
				_ControlSectionPool.Free(pMergeBlock);
			}

			if (pTargetBlock->pPreviosBlock && pTargetBlock->pPreviosBlock->bFree)
			{
				MemControlBlock* pMergeBlock = pTargetBlock->pPreviosBlock;

				if (pTargetBlock->pNextBlock)
				{
					pTargetBlock->pNextBlock->pPreviosBlock = pMergeBlock;
				}

				MergeDataFreePtrDataInBlock(pMergeBlock);

				pMergeBlock->pNextBlock = pTargetBlock->pNextBlock;
				pMergeBlock->Size += pTargetBlock->Size;
				 
				_ControlSectionPool.Free(pTargetBlock);
				
				pTargetBlock = pMergeBlock;

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR
				CL_ASSERT(!pTargetBlock->pPreviosBlock || !pTargetBlock->pPreviosBlock->bFree);

				ValidateBlockNotInTree(pTargetBlock);
				ValidateBlock(pTargetBlock, false, true);
#endif 

			}

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
			ValidateBlockNotInTree(pTargetBlock);
			ValidateBlock(pTargetBlock, true, true);
#endif 

			if (pTargetBlock->pNextBlock || pTargetBlock->pPreviosBlock)
			{
				InsertToMemBlockTree(pTargetBlock);
			}
			else
			{
				auto pMemData = _LookupTable.Find(pTargetBlock->pData, Compare);
				CL_ASSERT(pMemData);
				_LookupTable.Erase(pMemData, pMemData->Data());
				_ControlSectionPool.Free(pTargetBlock); 

				delete pMemData; 
			}
		}
		void MergeDataFreePtrDataInBlock(MemControlBlock* Value)
		{
#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
			CL_ASSERT(Value);
			CL_ASSERT(Value->bFree);
			CL_ASSERT(SanityMap.find(Value) != SanityMap.end());
#endif  

			if (Value->pPreviosFreeBlock)
			{
				Value->pPreviosFreeBlock->pNextFreeBlock = Value->pNextFreeBlock;
			}

			if (Value->pNextFreeBlock)
			{
				Value->pNextFreeBlock->pPreviosFreeBlock = Value->pPreviosFreeBlock;
			}

			if (!Value->pPreviosFreeBlock)
			{
				if (!Value->pNextFreeBlock)
				{
					_FreeMemSectionsBySize.Remove(Value->Size);
				}
				else
				{
#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR
					CL_ASSERT(SanityMap.find(Value->pNextFreeBlock) != SanityMap.end());
#endif

					auto pNode = _FreeMemSectionsBySize.Find(Value->Size);
					pNode->Value = pNode->Value->pNextFreeBlock;
				}

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR
				ValidateBlockNotInTree(Value);
#endif
			}

			Value->pNextFreeBlock = nullptr;
			Value->pPreviosFreeBlock = nullptr;
		} 
		template<std::size_t... Is>
		MemoryObject* CreateMemObjectT(const std::tuple<Args...>& tuple, std::index_sequence<Is...>, size_t size)
		{
			return new MemoryObject(std::get<Is>(tuple)..., size);
		}
		MemoryObject* CreateMemObj(size_t size)
		{
			MemoryObject* pObject;

			if constexpr (sizeof...(Args))
			{
				pObject = CreateMemObjectT(_MemObjectConstructorParams, std::index_sequence_for<Args...>(), size);
			}
			else
			{
				pObject = new MemoryObject(size);
			}

			CL_ASSERT(pObject);

			return pObject;
			}

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR
		void ValidateBlock(MemControlBlock* pBlock, bool bValidataNextFreeFlag, bool bValidataPreviousFreeFlag)
		{
			CL_ASSERT(SanityMap.find(pBlock) != SanityMap.end());

			CL_ASSERT(pBlock != pBlock->pNextBlock);
			CL_ASSERT(pBlock != pBlock->pPreviosBlock);
			CL_ASSERT(pBlock != pBlock->pNextFreeBlock);
			CL_ASSERT(pBlock != pBlock->pPreviosFreeBlock);

			CL_ASSERT(pBlock->bFree || !pBlock->bFree && !pBlock->pNextFreeBlock && !pBlock->pPreviosFreeBlock);

			CL_ASSERT(!pBlock->pNextBlock || pBlock < pBlock->pNextBlock);
			CL_ASSERT(!pBlock->pNextBlock || ((char*)pBlock + sizeof(MemControlBlock) + pBlock->Size) == (char*)pBlock->pNextBlock);
			CL_ASSERT(!bValidataNextFreeFlag || !pBlock->pNextBlock || (!pBlock->bFree || !pBlock->pNextBlock->bFree));

			CL_ASSERT(!pBlock->pPreviosBlock || pBlock > pBlock->pPreviosBlock);
			CL_ASSERT(!pBlock->pPreviosBlock || ((char*)pBlock->pPreviosBlock + sizeof(MemControlBlock) + pBlock->pPreviosBlock->Size) == (char*)pBlock);
			CL_ASSERT(!bValidataPreviousFreeFlag || !pBlock->pPreviosBlock || (!pBlock->bFree || !pBlock->pPreviosBlock->bFree));
		}
		void ValidateBlockNotInTree(const MemControlBlock* Value)
		{
			ValidateBlockNotInTree(Value, _FreeMemSectionsBySize.GetRoot());
		}
		void ValidateBlockNotInTree(const MemControlBlock* pTargetValue, TreeNodePtr pNode)
		{
			if (!pNode)
			{
				return;
			}

			CL_ASSERT(pNode->Value);

			auto pValue = pNode->Value;
			CL_ASSERT(!pValue->pPreviosFreeBlock);

			while (pValue)
			{
				CL_ASSERT(pValue != pTargetValue);

				pValue = pValue->pNextFreeBlock;
			}

			ValidateBlockNotInTree(pTargetValue, pNode->pLeft);
			ValidateBlockNotInTree(pTargetValue, pNode->pRight);
		}
		void ValidateTree(TreeNodePtr pNode, std::unordered_map<MemControlBlock*, MemControlBlock*>& treeStack)
		{
			if (!pNode)
			{
				return;
			}

			CL_ASSERT(pNode->Value);

			auto pValue = pNode->Value;
			CL_ASSERT(!pValue->pPreviosFreeBlock);

			std::unordered_map<MemControlBlock*, MemControlBlock*> localStack;

			while (pValue)
			{
				CL_ASSERT(pValue->Size == pNode->Key);

				CL_ASSERT(localStack.find(pValue) == localStack.end());
				localStack.insert(std::make_pair(pValue, pValue));

				CL_ASSERT(!pValue->pPreviosFreeBlock || pValue == pValue->pPreviosFreeBlock->pNextFreeBlock);
				CL_ASSERT(!pValue->pNextFreeBlock || pValue == pValue->pNextFreeBlock->pPreviosFreeBlock);

				CL_ASSERT(SanityMap.find(pValue) != SanityMap.end());
				CL_ASSERT(!pValue->pPreviosFreeBlock || SanityMap.find(pValue->pPreviosFreeBlock) != SanityMap.end());
				CL_ASSERT(!pValue->pNextFreeBlock || SanityMap.find(pValue->pNextFreeBlock) != SanityMap.end());

				pValue = pValue->pNextFreeBlock;
			}

			CL_ASSERT(treeStack.find(pNode->Value) == treeStack.end());
			treeStack.insert(std::make_pair(pNode->Value, pNode->Value));

			ValidateTree(pNode->pLeft, treeStack);
			ValidateTree(pNode->pRight, treeStack);

		}
		void Validate()
		{
#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR_TREE 
			AblTreeNode<size_t, MemControlBlock*>::Validate(_FreeMemSectionsBySize.GetRoot());

			std::unordered_map<MemControlBlock*, MemControlBlock*> stack;

			ValidateTree(_FreeMemSectionsBySize.GetRoot(), stack);
#endif
		}

		size_t nMemAllock = 0;
		size_t nMemFree = 0;
		size_t nRealMemFree = 0;
#endif  
		ABLTree<size_t, MemControlBlock*> _FreeMemSectionsBySize;
		size_t _MaxDirectAllocationSize;
		size_t _TotalSize;
		size_t _BlockAlign;
		MemoryLookUpTable<MemoryObject, char> _LookupTable;
		Pool<MemControlBlock> _ControlSectionPool;
		std::unordered_map<ValueType, MemoryObject*> _IndirectAllocMap;
		std::tuple<Args...> _MemObjectConstructorParams;
	};
}