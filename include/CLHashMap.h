#pragma once
#include <CLMacroHelper.h>
#include <CLHashString.h>
#include <CLMemory.h>
#include <CLList.h>
#include <CLEmbeddedArray.h>

#define CL_HASH_MAP_LOAD_FACTOR 0.75
#define CL_HASH_MAP_MAX_LINEAR_SEARCH_THRESHOLD 4
//#define CL_HASH_MAP_LIST_IMPLEMNTATION 0

#ifdef CL_HASH_MAP_LIST_IMPLEMNTATION
#define CL_HASH_MAX_MAX_COLLISIONS 4
#else
#define CL_HASH_MAX_MAX_COLLISIONS 4
#endif

namespace CL
{
	template<typename KeyType, typename ValueType>
	struct HashMapPair
	{
		HashMapPair() = default;
		HashMapPair(const size_t& InKey, const String& InValue) : Key(InKey), Value(InValue)
		{

		}
		HashMapPair(const size_t& InKey, String&& InValue) : Key(InKey), Value(Forward(InValue))
		{

		}
		HashMapPair(size_t&& InKey, String&& InValue) : Key(Forward(InKey)), Value(Forward(InValue))
		{

		}
		HashMapPair(size_t&& InKey, const String& InValue) : Key(Forward(InKey)), Value(InValue)
		{
			std::forward
		}
		HashMapPair(HashMapPair&& Other):
			Key(Move(Other.Key)), Value(Move(Other.Value))
		{
			
		}
		HashMapPair& operator = (HashMapPair&& Other)
		{
			Key   = Move(Other.Key);
			Value = Move(Other.Value);
			return *this;
		}

		size_t Key;
		String Value;

	private:
		HashMapPair(const HashMapPair&) = delete;
		HashMapPair& operator = (const HashMapPair&) = delete;
	};

	template<typename KeyType, typename ValueType>
	struct HashMapNode
	{
		typedef HashMapPair<KeyType, ValueType> HashMapPairType;
#ifdef CL_HASH_MAP_LIST_IMPLEMNTATION
		HashMapNode(CL::RefPtr<Pool<typename List<HashMapPairType>::ListNode>>& pHashMapPairPool) : Values(pHashMapPairPool)
		{

		}

		typename List<HashMapPairType>::ForwardIterator FindPairIndex(const size_t& Key)
		{
			for (auto it = Values.begin(); it; it++)
			{
				if (it().Key == Key)
				{
					return it;
				}
			}

			return List<HashMapPairType>::ForwardIterator();
		}
		
		List<HashMapPairType> Values;
#else
		HashMapNode() = default;

		typename EmbeddedArray<HashMapPairType, CL_HASH_MAX_MAX_COLLISIONS>::ForwardIterator FindPairIndex(const size_t& Key)
		{
			for (auto it = Values.begin(); it; it++)
			{
				if (it().Key == Key)
				{
					return it;
				}
			}

			return EmbeddedArray<HashMapPairType, CL_HASH_MAX_MAX_COLLISIONS>::ForwardIterator();
		}

		EmbeddedArray<HashMapPairType, CL_HASH_MAX_MAX_COLLISIONS> Values;
#endif

	};

	template<typename KeyType, typename ValueType>
	class HashMapIterator
	{
		template<typename KeyType, typename ValueType>
		friend class HashMap;
	public:
		typedef HashMapNode<KeyType, ValueType> HashMapNodeType;
		typedef HashMapPair<KeyType, ValueType> HashMapPairType;
		typedef HashMapIterator<KeyType, ValueType> Iterator;
#ifdef CL_HASH_MAP_LIST_IMPLEMNTATION
		typedef typename List<HashMapPairType>::ForwardIterator ContainerIterator;
#else
		typedef typename EmbeddedArray<HashMapPairType, CL_HASH_MAX_MAX_COLLISIONS>::ForwardIterator ContainerIterator;
#endif
		Iterator& operator++(int)
		{
			CL_DEBUG_ASSERT(_mEntry);
			FindNext();
			return *this;
		}
		Iterator& operator++()
		{
			CL_DEBUG_ASSERT(_mEntry);
			FindNext();
			return *this;
		}
		const HashMapPairType& operator*()
		{
			return _ContainerIterator();
		}
		const HashMapPairType& operator ()()
		{
			return _ContainerIterator();
		}
		const HashMapPairType& operator ->()
		{
			return _ContainerIterator();
		}
		operator bool() const
		{
			return _mEntry;
		}
		bool operator == (const Iterator& other) const
		{
			return _ContainerIterator == other._ContainerIterator;
		}
		bool operator != (const Iterator& other) const
		{
			return _ContainerIterator != other._ContainerIterator;
		}
	private:
		CL_FORCEINLINE void FindNext()
		{
			_ContainerIterator++;

			if (!_ContainerIterator)
			{
				_Index++;
				for (; _Index < _Capacity; _Index++)
				{
					if (_mEntry[_Index])
					{
						_ContainerIterator = _mEntry[_Index]->Values.begin();
						break;
					}
				}
			}
		}

		HashMapNodeType** _mEntry = nullptr;
		size_t _Capacity;
		size_t _Index;
		ContainerIterator _ContainerIterator;
	};

	template<typename KeyType, typename ValueType>
	class HashMap
	{
	public:
		typedef HashMapNode<KeyType, ValueType> HashMapNodeType;
		typedef HashMapPair<KeyType, ValueType> HashMapPairType;
		typedef typename HashMapIterator<KeyType, ValueType> ForwardIterator;

		HashMap() : 
#ifdef CL_HASH_MAP_LIST_IMPLEMNTATION
			_pHashMapPairPool(CL::RefPtr<Pool<List<HashMapPairType>::ListNode>>::MakeRefPtr(256)),
#endif
			_EntriesPool(256)
		{
			_Capacity = 64;
			_NumUsedEntries = 0;
			_mEntry = (HashMapNodeType**)CL_MALLOC(_Capacity * sizeof(HashMapNodeType*));
			memset(_mEntry, 0, sizeof(HashMapNodeType*) * _Capacity);
		}
		__forceinline size_t IndexFromKey(size_t Key) const
		{
			size_t Index = std::hash<size_t>::_Do_hash(Key) & (_Capacity - 1);
			return Index;
			//return Key % _Capacity;
		}
		void Insert(size_t Key, String&& Value)
		{
			if (float(_NumUsedEntries) / _Capacity > CL_HASH_MAP_LOAD_FACTOR)
			{
				ReHash(_Capacity * 2);
			}

			size_t Index = IndexFromKey(Key);
			HashMapNodeType* pNode = _mEntry[Index];

			if (!pNode)
			{
				_NumUsedEntries++;
				_NumElements++;
#ifdef CL_HASH_MAP_LIST_IMPLEMNTATION
				pNode = _EntriesPool.Alloc(_pHashMapPairPool);
#else
				pNode = _EntriesPool.Alloc();
#endif
				pNode->Values.PushBack(HashMapPairType(Key, Value));
				_mEntry[Index] = pNode;
				return;
			}

			auto it = pNode->FindPairIndex(Key);

			if (it)
			{
				it().Value = Forward(Value);
			}
			else if (pNode->Values.GetElementsCount() >= CL_HASH_MAX_MAX_COLLISIONS - 1)
			{
				const size_t MaxIndex = CL_MIN(_Capacity, Index + CL_HASH_MAP_MAX_LINEAR_SEARCH_THRESHOLD);
				HashMapNodeType* pFoundNode = nullptr;

				for (size_t i = Index + 1; i < MaxIndex; i++)
				{
					HashMapNodeType* pNode = _mEntry[i];
					if (pNode)
					{
						auto it = pNode->FindPairIndex(Key);

						if(it)
						{
							it().Value = Move(Value);
							return;
						}

						if (pNode->Values.GetElementsCount() < CL_HASH_MAX_MAX_COLLISIONS - 1)
						{
							pFoundNode = pNode;
						}
					}
					else
					{
						_NumUsedEntries++;
						_NumElements++;
#ifdef CL_HASH_MAP_LIST_IMPLEMNTATION
						pNode = _EntriesPool.Alloc(_pHashMapPairPool);
#else
						pNode = _EntriesPool.Alloc();
#endif
						pNode->Values.PushBack(HashMapPairType(Key, Value));
						_mEntry[i] = pNode;
						return;
					}
				}

				if (pFoundNode)
				{
					pFoundNode->Values.PushBack(HashMapPairType(Key, Value));
				}

				ReHash(_Capacity * 2);
				Insert(Key, CL::Forward(Value));
			}
			else
			{
				pNode->Values.PushBack(HashMapPairType(Key, Value));
				_NumElements++;
			}
		}
		CL_NO_DISCARD ForwardIterator Find(const size_t& Key) const
		{
			ForwardIterator MapIterator;

			if (_mEntry)
			{
				size_t Index = IndexFromKey(Key);
				const size_t MaxIndex = CL_MIN(_Capacity, Index + CL_HASH_MAP_MAX_LINEAR_SEARCH_THRESHOLD);
				HashMapNodeType* pFoundNode = nullptr;

				for (size_t i = Index; i < MaxIndex; i++)
				{
					HashMapNodeType* pNode = _mEntry[i];
					if (pNode)
					{
						auto it = pNode->FindPairIndex(Key);

						if (it)
						{
							MapIterator._Capacity = _Capacity;
							MapIterator._mEntry = _mEntry;
							MapIterator._Index = i;
							MapIterator._ContainerIterator = it;
							return MapIterator;
						}
					}
				}
			}

			return MapIterator;
		}
		ForwardIterator Erase(const ForwardIterator& It)
		{
			CL_ASSERT(It);

			HashMapNodeType*& pNode = _mEntry[It._Index];

			ForwardIterator NewIt = It;
#ifdef CL_HASH_MAP_LIST_IMPLEMNTATION
			NewIt._ContainerIterator = pNode->Values.Erase(It._ContainerIterator);
#else
			NewIt._ContainerIterator = pNode->Values.EraseSwap(It._ContainerIterator);
#endif
			
			_NumElements--;

			if (!NewIt._ContainerIterator)
			{
				NewIt._Index++;
				for (; NewIt._Index < _Capacity; NewIt._Index++)
				{
					if (NewIt._mEntry[NewIt._Index])
					{
						NewIt._ContainerIterator = NewIt._mEntry[NewIt._Index]->Values.begin();
						break;
					}
				}
			}

			if (!pNode->Values.GetElementsCount())
			{
				_NumUsedEntries--;
				_EntriesPool.Free(pNode);
				pNode = nullptr;

				//if (float(_NumUsedEntries) / (_Capacity) < 0.1)
				//{
				//	if (NewIt._ContainerIterator)
				//	{
				//		KeyType Key = NewIt().Key;
				//		ReHash(_Capacity * 0.75);
				//		NewIt = Find(Key);
				//	}
				//	else
				//	{
				//		ReHash(_Capacity * 0.75);
				//	}
				//}
			}

			return NewIt;
		}
		void Clear()
		{
			if (_mEntry)
			{
				for (size_t i = 0; i < _Capacity; i++)
				{
					_EntriesPool.Free(_mEntry[i]);
				}

				CL_FREE(_mEntry);
				_Capacity = 0;
				_NumElements = 0;
				_NumUsedEntries = 0;
				_mEntry = nullptr;
#ifdef CL_HASH_MAP_LIST_IMPLEMNTATION
				_pHashMapPairPool.Free();
#endif
			}
		}
		ForwardIterator begin()
		{
			ForwardIterator MapIterator;

			for (size_t Index = 0; Index < _Capacity; Index++)
			{
				if (_mEntry[Index])
				{
					MapIterator._Capacity = _Capacity;
					MapIterator._mEntry = _mEntry;
					MapIterator._Index = Index;
					MapIterator._ContainerIterator = _mEntry[Index]->Values.begin();
					break;
				}
			}

			return MapIterator;
		}
		ForwardIterator end()
		{
			return ForwardIterator();
		}
		~HashMap()
		{
			Clear();
		}

		HashMap(const HashMap&) = delete;
		HashMap& operator = (const HashMap&) = delete;

	private:
		CL::Pool<HashMapNodeType> _EntriesPool;
#ifdef CL_HASH_MAP_LIST_IMPLEMNTATION
		CL::RefPtr<Pool<typename List<HashMapPairType>::ListNode>> _pHashMapPairPool;
#endif

		bool bRehash = false;

		HashMapNodeType** _mEntry = nullptr;
		size_t _Capacity = 0;
		size_t _NumUsedEntries = 0;
		size_t _NumElements = 0;
		
		void Insert(HashMapPairType&& Pair)
		{
			if (float(_NumUsedEntries) / _Capacity > CL_HASH_MAP_LOAD_FACTOR)
			{
				ReHash(_Capacity * 2);
			}

			size_t Index = IndexFromKey(Pair.Key);
			HashMapNodeType* pNode = _mEntry[Index];

			if (!pNode)
			{
				_NumUsedEntries++;
				_NumElements++;

#ifdef CL_HASH_MAP_LIST_IMPLEMNTATION
				pNode = _EntriesPool.Alloc(_pHashMapPairPool);
#else
				pNode = _EntriesPool.Alloc();
#endif

				pNode->Values.PushBack(Forward(Pair));
				_mEntry[Index] = pNode;
				return;
			}

			if (pNode->Values.GetElementsCount() >= CL_HASH_MAX_MAX_COLLISIONS - 1)
			{
				const size_t MaxIndex = CL_MIN(_Capacity, Index + CL_HASH_MAP_MAX_LINEAR_SEARCH_THRESHOLD);
				HashMapNodeType* pFoundNode = nullptr;

				for (size_t i = Index + 1; i < MaxIndex; i++)
				{
					HashMapNodeType* pNode = _mEntry[i];
					if (pNode)
					{
						if (pNode->Values.GetElementsCount() <= CL_HASH_MAX_MAX_COLLISIONS)
						{
							pFoundNode = pNode;
						}
					}
					else
					{
						_NumUsedEntries++;
						_NumElements++;

#ifdef CL_HASH_MAP_LIST_IMPLEMNTATION
						pNode = _EntriesPool.Alloc(_pHashMapPairPool);
#else
						pNode = _EntriesPool.Alloc();
#endif

						pNode->Values.PushBack(Forward(Pair));
						_mEntry[i] = pNode;
						return;
					}
				}

				if (pFoundNode)
				{
					pFoundNode->Values.PushBack(Forward(Pair));
					return;
				}

				CL_FATAL();
			}
			else
			{
				pNode->Values.PushBack(Forward(Pair));
				_NumElements++;
			}
		}
		void ReHash(size_t NewCapacity)
		{
			//CL_ASSERT(!bRehash);

			if (NewCapacity < 64)
			{
				return;
			}

			bRehash = true;

			size_t OldCapacity = _Capacity;
			_Capacity = NewCapacity;
			_NumUsedEntries = 0;
			_NumElements = 0;

			HashMapNodeType** mOldEntry = _mEntry;
			_mEntry = (HashMapNodeType**)CL_MALLOC(_Capacity * sizeof(HashMapNodeType*));
			memset(_mEntry, 0, sizeof(HashMapNodeType*) * _Capacity);

			for (size_t ArrayIndex = 0; ArrayIndex < OldCapacity; ArrayIndex++)
			{
				HashMapNodeType* pNode = mOldEntry[ArrayIndex];

				if (pNode && pNode->Values.GetElementsCount())
				{
					size_t Index = IndexFromKey(pNode->Values.begin()().Key);
					if (pNode->Values.GetElementsCount() == 1 && !_mEntry[Index])
					{
						_mEntry[Index] = pNode;
						pNode = nullptr;
					}
					else
					{
						for (HashMapPairType& Pair : pNode->Values)
						{
							Insert(Move(Pair));
						}
					}
				}

				_EntriesPool.Free(pNode);
			}

			CL_FREE(mOldEntry);

			bRehash = false;
		}
	};
}