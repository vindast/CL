#pragma once
#include "CLABLTree.h" 
#include "CLMemoryCommon.h" 

//#define CL_DEBUG_BYTE_MEM_ALLOCATOR
//#define CL_DEBUG_BYTE_MEM_ALLOCATOR_TREE

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
#include <iostream>
#include <unordered_map>
#endif
 
namespace CL
{
	struct MemControlBlock; 

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
	static std::unordered_map<MemControlBlock*, MemControlBlock*> SanityMap;
	static size_t nMemControlBlocks = 0;
#endif 

	struct MemControlBlock
	{
		MemControlBlock()
		{
#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR
			CL_ASSERT(SanityMap.find(this) == SanityMap.end());
			SanityMap.insert(std::make_pair(this, this));
			nMemControlBlocks++;
#endif
		}
		~MemControlBlock()
		{
#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR
			CL_ASSERT(SanityMap.find(this) != SanityMap.end());
			SanityMap.erase(SanityMap.find(this));
			nMemControlBlocks--;
#endif
		}

		bool bFree = false;
		char* pData = 0;
		size_t Size = 0;

		MemControlBlock* pPreviosFreeBlock = nullptr;
		MemControlBlock* pNextFreeBlock    = nullptr;

		MemControlBlock* pPreviosBlock = nullptr;
		MemControlBlock* pNextBlock    = nullptr;
	}; 

	struct MemBlockData
	{
		char* pData;
		size_t TotalSize;
	};

	class HeapAllocator final
	{
		typedef AblTreeNode<size_t, MemControlBlock*>* TreeNodePtr;
	public: 
		HeapAllocator(size_t TotalSize = BytesFromMB(16), size_t MaxDirectAllocationSize = BytesFromMB(2), size_t BaseTreeNodePoolSize = 512, size_t BlockAlign = 32) :
			_FreeMemSectionsBySize(BaseTreeNodePoolSize),
			_TotalSize(TotalSize),
			_LookupTable(TotalSize),
			_MaxDirectAllocationSize(MaxDirectAllocationSize),
			_BlockAlign(BlockAlign)
		{  
		
		}
		void CreateNewBlock(size_t size)
		{     
			size = CL_ALIGN(size, _TotalSize);
			size += sizeof(MemControlBlock) + sizeof(MemBlockData);
			size = CL_ALIGN(size, _BlockAlign);

			char* pMem = (char*)CL_MALLOC(size);

			CL_ASSERT(pMem);

			MemBlockData* pDataBlock = CL_PLACEMENT_NEW(pMem,MemBlockData);

			pDataBlock->pData = pMem;
			pDataBlock->TotalSize = size;

			MemControlBlock* pFreeMemBlock = CL_PLACEMENT_NEW(pMem + sizeof(MemBlockData), MemControlBlock);

			pFreeMemBlock->Size = size - sizeof(MemControlBlock) - sizeof(MemBlockData);
			pFreeMemBlock->bFree = true;
			pFreeMemBlock->pData = pMem + sizeof(MemControlBlock) + sizeof(MemBlockData);

			InsertToMemBlockTree(pFreeMemBlock);
			 
			_LookupTable.Insert(pDataBlock, pMem, pFreeMemBlock->Size);
		} 
		void* Allocate(size_t size, size_t alignment)
		{ 
			CL_DEBUG_ASSERT(alignment);
			CL_DEBUG_ASSERT(size); 
			return Allocate(CL_ALIGN(size, alignment));
		}
		void* Allocate(size_t size)
		{
			CL_DEBUG_ASSERT(size);
			size_t AlignedSize = CL_ALIGN(size, _BlockAlign);

			if (AlignedSize > _MaxDirectAllocationSize)
			{
				return CL_MALLOC(AlignedSize);
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

			if (pBlock->Size > AlignedSize + sizeof(MemControlBlock))
			{
				char* pData = pBlock->pData + AlignedSize;
				MemControlBlock* pFreeMemBlock = CL_PLACEMENT_NEW(pData, MemControlBlock);
				pFreeMemBlock->Size = pBlock->Size - AlignedSize - sizeof(MemControlBlock);
				pFreeMemBlock->pData = pData + sizeof(MemControlBlock);
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

			return pBlock->pData;
		}
		bool Free(void* ptr)
		{
			if (!ptr)
			{
				return false;
			}

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
			nMemFree++;
#endif
			char* a = (char*)ptr;
			MemBlockData* data = _LookupTable.Find(a, Compare);

			if (!data)
			{
				CL_FREE(ptr);
				return true;
			} 

			MemControlBlock* pBlock = (MemControlBlock*)(a - sizeof(MemControlBlock));
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
			return _FreeMemSectionsBySize.GetRoot()->Key == _TotalSize - sizeof(MemControlBlock);
		} 
		size_t Size() const
		{
			return _TotalSize;
		}
		~HeapAllocator() 
		{
			MemBlockData* pData = nullptr; 

			for (size_t i = 0; i < _LookupTable.Size(); i++)
			{
				if (pData != _LookupTable[i])
				{
					pData = _LookupTable[i];

					if (!pData)
					{
						continue;
					}

					MemControlBlock* pBlock = (MemControlBlock*)((char*)pData + sizeof(MemBlockData));

					while (pBlock)
					{
						MemControlBlock* pNextBlock = pBlock->pNextBlock;

						CL_PLACEMENT_DELETE(pBlock);

						pBlock = pNextBlock;
					}

					CL_FREE(pData);
				}
			}  
		}
	private:   
		static bool Compare(const MemBlockData* Data, const char* ptr)
		{
			if (!Data)
			{
				return false;
			}

			return Data->pData <= ptr && ptr < (Data->pData + Data->TotalSize);
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
		/*static TreeNodePtr FindRight(TreeNodePtr pNode, size_t Key, TreeNodePtr pFoundNode = nullptr)
		{
			if (!pNode)
			{
				return pFoundNode;
			}

			if (pNode->Key > Key)
			{
				if (!pFoundNode || pFoundNode && pFoundNode->Key > pNode->Key)
				{
					pFoundNode = pNode;
				}

				return FindRight(pNode->pLeft, Key, pFoundNode);
			}
			else
			{
				return FindRight(pNode->pRight, Key, pFoundNode);
			}
		}
		static TreeNodePtr FindLeft(TreeNodePtr pNode, size_t Key, TreeNodePtr pFoundNode = nullptr)
		{
			if (!pNode)
			{
				return pFoundNode;
			}

#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR
			CL_ASSERT(SanityMap.find(pNode->Value.DataPtr()) != SanityMap.end());
			CL_ASSERT(pNode->Key == pNode->Value->Index);
#endif

			if (pNode->Key < Key)
			{
				if (!pFoundNode || pFoundNode && pFoundNode->Key < pNode->Key)
				{
					pFoundNode = pNode;
				}

				return FindLeft(pNode->pRight, Key, pFoundNode);
			}
			else
			{
				return FindLeft(pNode->pLeft, Key, pFoundNode);
			}
		}*/
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
				pTargetBlock->Size += sizeof(MemControlBlock) + pMergeBlock->Size; 
				 
				MergeDataFreePtrDataInBlock(pMergeBlock);
				 
#ifdef CL_DEBUG_BYTE_MEM_ALLOCATOR 
				ValidateBlockNotInTree(pMergeBlock);
				ValidateBlock(pTargetBlock, true, false);
#endif 
				CL_PLACEMENT_DELETE(pMergeBlock);
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
				pMergeBlock->Size += sizeof(MemControlBlock) + pTargetBlock->Size;

				CL_PLACEMENT_DELETE(pTargetBlock);
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
				_LookupTable.Erase(pMemData, pMemData->pData);

				CL_PLACEMENT_DELETE(pTargetBlock);

				CL_FREE(pMemData);
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

			Value->pNextFreeBlock    = nullptr;
			Value->pPreviosFreeBlock = nullptr;
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
			CL_ASSERT(!bValidataPreviousFreeFlag ||  !pBlock->pPreviosBlock || (!pBlock->bFree || !pBlock->pPreviosBlock->bFree));
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
		MemoryLookUpTable<MemBlockData, char> _LookupTable;
	}; 
}