#pragma once
#include "CLPool2.h"

//#define CL_DEBUG_ABL_TREE

#ifdef CL_DEBUG_ABL_TREE 
#include <iostream>
#include <unordered_map>
#endif

namespace CL
{
#ifdef CL_DEBUG_ABL_TREE
	static size_t TreeNodeCreation = 0;
	static size_t TreeNodeMaxCount = 0; 
	static std::unordered_map<void*, void*> AblTreeSanity;
#endif

	template<class KeyType, class ValueType>
	struct AblTreeNode
	{  
		typedef AblTreeNode<KeyType, ValueType>* AblTreeNodePtr;

		AblTreeNode(const KeyType& InKey, ValueType& InValue, Pool<AblTreeNode<KeyType, ValueType>>* pInNodePool) :
			Key(InKey),
			hight(1),
			pLeft(nullptr),
			pRight(nullptr),
			Value(InValue),
			_pNodePool(pInNodePool)
		{
#ifdef CL_DEBUG_ABL_TREE
			CL_ASSERT(Key == Value->Index || Key == Value->Size);
			CL_ASSERT(AblTreeSanity.find(this) == AblTreeSanity.end());
			AblTreeSanity.insert(std::make_pair(this, this));

			TreeNodeCreation++; 
			TreeNodeMaxCount = TreeNodeMaxCount > TreeNodeCreation ? TreeNodeMaxCount : TreeNodeCreation;
#endif
		}
		static void Validate(AblTreeNodePtr pNode)
		{
			if (!pNode)
			{
				return;
			}
#ifdef CL_DEBUG_ABL_TREE
			CL_ASSERT(AblTreeSanity.find(pNode.DataPtr()) != AblTreeSanity.end());
			CL_ASSERT(!pNode->pLeft || AblTreeSanity.find(pNode->pLeft.DataPtr()) != AblTreeSanity.end());
			CL_ASSERT(!pNode->pLeft || pNode->pLeft->Key < pNode->Key);
#endif
			Validate(pNode->pLeft); 
#ifdef CL_DEBUG_ABL_TREE
			CL_ASSERT(!pNode->pRight || AblTreeSanity.find(pNode->pRight.DataPtr()) != AblTreeSanity.end());
			CL_ASSERT(!pNode->pRight || pNode->pRight->Key > pNode->Key);
#endif
			Validate(pNode->pRight);

		}
		static AblTreeNodePtr Remove(const KeyType& Key, AblTreeNodePtr pNode, Pool<AblTreeNode<KeyType, ValueType>>* pNodePool)
		{ 
			if (!pNode)
			{
				return nullptr;
			}

			if (Key < pNode->Key)
			{
				pNode->pLeft = Remove(Key, pNode->pLeft, pNodePool);
			}
			else if (Key > pNode->Key)
			{
				pNode->pRight = Remove(Key, pNode->pRight, pNodePool);
			}
			else
			{
				AblTreeNodePtr q = pNode->pLeft;
				AblTreeNodePtr r = pNode->pRight;

				pNode->pRight = nullptr;
				pNode->pLeft  = nullptr;

				pNodePool->Free(pNode); 

				if (!r)
				{
					return q;
				}

				AblTreeNodePtr min = FindMinNode(r);
				min->pRight = RemoveMin(r);
				min->pLeft = q;
				return Balance(min);
			}

			return Balance(pNode);
		}
		static AblTreeNodePtr Find(AblTreeNodePtr& pNode, const KeyType& InKey)
		{
			if (!pNode)
			{
				return AblTreeNodePtr(nullptr);
			}

			if (InKey > pNode->Key)
			{
				return Find(pNode->pRight, InKey);
			}
			else if (InKey < pNode->Key)
			{
				return Find(pNode->pLeft, InKey);
			}

			return pNode;
		}
		static AblTreeNodePtr Find(const AblTreeNodePtr& pNode, const KeyType& InKey)
		{
			if (!pNode)
			{
				return AblTreeNodePtr(nullptr);
			}

			if (InKey > pNode->Key)
			{
				return Find(pNode->pRight, InKey);
			}
			else if (InKey < pNode->Key)
			{
				return Find(pNode->pLeft, InKey);
			}

			return pNode;
		}
		static AblTreeNodePtr Insert(const KeyType& Key, ValueType& Value, AblTreeNodePtr pNode, Pool<AblTreeNode<KeyType, ValueType>>* pNodePool)
		{
			if (!pNode)
			{
				return pNodePool->Alloc(Key, Value, pNodePool); 
			}

			CL_ASSERT(pNode->Key != Key);

			if (pNode->Key < Key)
			{
				pNode->pRight = Insert(Key, Value, pNode->pRight, pNodePool);
			}
			else
			{
				pNode->pLeft = Insert(Key, Value, pNode->pLeft, pNodePool);
			}

			return Balance(pNode);
		}
		~AblTreeNode()
		{
#ifdef CL_DEBUG_ABL_TREE
			CL_ASSERT(AblTreeSanity.find(this) != AblTreeSanity.end());
			AblTreeSanity.erase(this);
			TreeNodeCreation--;
#endif 
		//	std::cout << " ~TreeNode(), Key = " << Key << ", total node count = " << TreeNodeCreation << std::endl;
			 
			_pNodePool->Free(pLeft);
			_pNodePool->Free(pRight); 
		}
		 
		AblTreeNodePtr pLeft;
		AblTreeNodePtr pRight;
		unsigned char hight;
		const KeyType Key;
		ValueType Value;
	private:
		AblTreeNode<KeyType, ValueType>(const AblTreeNode<KeyType, ValueType>&) = delete;
		AblTreeNode<KeyType, ValueType>& operator = (const AblTreeNode<KeyType, ValueType>&) = delete;
		 
		__forceinline static unsigned char GetHeight(const AblTreeNodePtr& pNode)
		{
			return pNode ? pNode->hight : 0;
		}
		__forceinline static int GetBalanceFactor(const AblTreeNodePtr& pNode)
		{
			return GetHeight(pNode->pRight) - GetHeight(pNode->pLeft);
		}
		__forceinline static void FixHeight(AblTreeNodePtr& pNode)
		{
			unsigned char hl = GetHeight(pNode->pLeft);
			unsigned char hr = GetHeight(pNode->pRight);
			pNode->hight = (hl > hr ? hl : hr) + 1;
		}
		__forceinline static  AblTreeNodePtr RemoveMin(AblTreeNodePtr p)
		{
			if (!p->pLeft)
			{
				return p->pRight;
			}

			p->pLeft = RemoveMin(p->pLeft);

			return Balance(p);
		}
		__forceinline static AblTreeNodePtr FindMinNode(AblTreeNodePtr p)
		{
			return p->pLeft ? FindMinNode(p->pLeft) : p;
		}
		__forceinline static AblTreeNodePtr RotateRight(AblTreeNodePtr p)
		{
			AblTreeNodePtr q = p->pLeft;
			p->pLeft = q->pRight;
			q->pRight = p;
			FixHeight(p);
			FixHeight(q);
			return q;
		}
		__forceinline static AblTreeNodePtr RotateLeft(AblTreeNodePtr q)
		{
			AblTreeNodePtr p = q->pRight;
			q->pRight = p->pLeft;
			p->pLeft = q;
			FixHeight(q);
			FixHeight(p);
			return p;
		}
		__forceinline static AblTreeNodePtr Balance(AblTreeNodePtr p)
		{
			FixHeight(p);

			if (GetBalanceFactor(p) == 2)
			{
				if (GetBalanceFactor(p->pRight) < 0)
				{
					p->pRight = RotateRight(p->pRight);
				}

				return RotateLeft(p);
			}

			if (GetBalanceFactor(p) == -2)
			{
				if (GetBalanceFactor(p->pLeft) > 0)
				{
					p->pLeft = RotateLeft(p->pLeft);
				}

				return RotateRight(p);
			}

			return p;
		}

		Pool<AblTreeNode<KeyType, ValueType>>* _pNodePool;
	};

	template<class KeyType, class ValueType>
	class ABLTree
	{ 
		typedef AblTreeNode<KeyType, ValueType>* AblTreeNodePtr;
	public:
		ABLTree(size_t NodePoolBaseSize):
			_NodePool(NodePoolBaseSize)
		{

		}
		AblTreeNodePtr Find(const KeyType& Key)
		{
			return AblTreeNode<KeyType, ValueType>::Find(_pRoot, Key);
		}
		const AblTreeNodePtr Find(const KeyType& Key) const
		{
			return AblTreeNode<KeyType, ValueType>::Find(_pRoot, Key);
		}
		void Insert(const KeyType& Key, ValueType& Value)
		{
			_pRoot = AblTreeNode<KeyType, ValueType>::Insert(Key, Value, _pRoot, &_NodePool);
		} 
		void Remove(const KeyType& Key)
		{
			_pRoot = AblTreeNode<KeyType, ValueType>::Remove(Key, _pRoot, &_NodePool);
		}
		AblTreeNodePtr GetRoot()
		{
			return _pRoot;
		}
		const AblTreeNodePtr GetRoot() const
		{
			return _pRoot;
		}
		~ABLTree()
		{
			_NodePool.Free(_pRoot); 
		}
	private:
		ABLTree(const ABLTree&) = delete;
		ABLTree& operator = (const ABLTree&) = delete;

		AblTreeNodePtr _pRoot = nullptr;
		Pool<AblTreeNode<KeyType, ValueType>> _NodePool;
	};
}