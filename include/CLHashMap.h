#pragma once
#include <CLMacroHelper.h>
#include <CLHashString.h>
#include <CLMemory.h>
#include <CLList.h>
#include <CLEmbeddedArray.h>
#include <CLVector.h>

#define CL_HASH_MAP_LOAD_FACTOR 0.75

// #TODO CLConfig
#define CL_ENABLE_PROBING_INSERT 0
#define CL_HASH_MAP_USE_KEY_ARRAY 0
#define CL_PARANOIDAL_DEBUG 0

#if CL_ENABLE_PROBING_INSERT
#define CL_HASH_MAP_MAX_PROBING_TAPS 4
#define CL_HASH_MAP_REHASH_SIZE_MULTIPLIER 8
#else
#define CL_HASH_MAP_REHASH_SIZE_MULTIPLIER 2
#endif

#define CL_HASH_MAP_MAX_COLLISIONS 4
#define CL_HASH_MAP_MAX_REHASH_POOL_SIZE 16

#if CL_PARANOIDAL_DEBUG 
#ifndef CL_HASH_MAP_DEBUG
#define CL_HASH_MAP_DEBUG 1
#endif
#endif

#include <CLString.h>

// #TODO CL::HashMap: add direct hash map to increase searching performance with small amount of elements.

namespace CL
{
	template<class KeyType, class ValueType> struct HashMapPair;
	template<class KeyType, class ValueType> struct HashMapBucket;
	template<class KeyType, class ValueType> class HashMapIterator;
	template<class KeyType, class ValueType> class HashMap;

	template<class KeyType, class ValueType>
	struct HashMapPair
	{
		HashMapPair() = default;
		HashMapPair(const KeyType& InKey, const ValueType& InValue) :
			Key(InKey), Value(InValue)
		{

		}
		HashMapPair(const KeyType&& InKey, const ValueType& InValue) :
			Key(CL::Move(InKey)), Value(InValue)
		{

		}
		HashMapPair(const KeyType& InKey, const ValueType&& InValue) :
			Key(InKey), Value(CL::Move(InValue))
		{

		}
		HashMapPair(const KeyType&& InKey, const ValueType&& InValue) :
			Key(CL::Move(InKey)), Value(CL::Move(InValue))
		{

		}

		size_t Hash;
		KeyType Key;
		ValueType Value;
	};

	template<class KeyType, class ValueType>
	struct HashMapBucket
	{
		typedef HashMapPair<KeyType, ValueType> HashMapPairType;

		HashMapBucket()
		{
			for (size_t I = 0; I < 4; I++)
			{
				Pairs[I] = nullptr;
			}
		}

		size_t NumPairs = 0;
#if CL_HASH_MAP_USE_KEY_ARRAY
		KeyType Hashes[CL_HASH_MAP_MAX_COLLISIONS];
#else
		size_t Hashes[CL_HASH_MAP_MAX_COLLISIONS];
#endif
		
		HashMapPairType* Pairs[CL_HASH_MAP_MAX_COLLISIONS];
#if CL_HASH_MAP_USE_KEY_ARRAY
		size_t FindPair(const KeyType& Key) const
#else
		size_t FindPair(const KeyType& Key, const size_t& Hash) const
#endif
		{
			for (size_t Index = 0; Index < NumPairs; Index++)
			{
#if CL_HASH_MAP_USE_KEY_ARRAY
				if (Hashes[Index] == Key)
				{
					return Index;
				}
#else
				if (Hashes[Index] == Hash && Pairs[Index]->Key == Key)
				{
					return Index;
				}
#endif
			}

			return CL_HASH_MAP_MAX_COLLISIONS;
		}
		void Insert(HashMapPairType* pPair)
		{
			CL_DEBUG_ASSERT(NumPairs < CL_HASH_MAP_MAX_COLLISIONS);
			Hashes[NumPairs] = pPair->Hash;
#if CL_HASH_MAP_USE_KEY_ARRAY
			Hashes[NumPairs] = pPair->Key;
#else
			Hashes[NumPairs] = pPair->Hash;
#endif
			Pairs[NumPairs] = pPair;
			NumPairs++;
		}
		void Erase(size_t Index, CL::Pool<HashMapPairType>& PairsPool)
		{
			CL_DEBUG_ASSERT(NumPairs > 0);

			PairsPool.Free(Pairs[Index]);

			NumPairs--;

			if (NumPairs != Index)
			{
				Pairs[Index] = Pairs[NumPairs];
				Hashes[Index] = Hashes[NumPairs];
			}

			//Pairs[NumPairs] = nullptr;
		}
		bool IsAbleToInsert() const { return NumPairs < CL_HASH_MAP_MAX_COLLISIONS; }
		bool IsEmpty() const { return NumPairs == 0; }
		
		HashMapBucket* pNext = nullptr;
		HashMapBucket* pPrevious = nullptr;
	};

	template<class KeyType, class ValueType>
	class HashMapIterator
	{
		typedef HashMapBucket<KeyType, ValueType> BucketType;
		typedef HashMapPair<KeyType, ValueType> HashMapPairType;
		typedef HashMapIterator<KeyType, ValueType> Iterator;
		friend class HashMap<KeyType, ValueType>;
	public:
		HashMapIterator() : _pBucket(nullptr)
			, _Index(0)
		{

		}
		HashMapIterator(BucketType* pBucket, size_t Index) : _pBucket(pBucket), _Index(Index)
		{

		}
		Iterator& operator++(int)
		{
			Inc();
			return *this;
		}
		Iterator& operator++()
		{
			Inc();
			return *this;
		}
		void Inc()
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < _pBucket->NumPairs);
			_Index++;
			if (_Index >= _pBucket->NumPairs)
			{
				_Index = 0;
				_pBucket = _pBucket->pNext;
			}
		}
		HashMapPairType& operator*() const
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return *_pBucket->Pairs[_Index];
		}
		HashMapPairType& operator ()() const
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return *_pBucket->Pairs[_Index];
		}
		const KeyType& GetKey() const
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return _pBucket->Pairs[_Index]->Key;
		}
		ValueType& GetValue()
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return _pBucket->Pairs[_Index]->Value;
		}
		const ValueType& GetValue() const
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return _pBucket->Pairs[_Index]->Value;
		}
		operator bool() const
		{
			return _pBucket && _Index < _pBucket->NumPairs;
		}
		bool operator == (const Iterator& other) const
		{
			return _pBucket == other._pBucket && (_pBucket ? _Index == other._Index : true);
		}
		bool operator != (const Iterator& other) const
		{
			return _pBucket != other._pBucket || (_pBucket ? _Index != other._Index : false);
		}
		size_t GetIndex() const { return _Index; }
		const BucketType* GetBucket() const { return _pBucket; }
	private:
		BucketType* _pBucket;
		size_t _Index;
	};

	template<class KeyType, class ValueType>
	class HashMapConstIterator
	{
		typedef HashMapBucket<KeyType, ValueType> const BucketType;
		typedef HashMapPair<KeyType, ValueType> const HashMapPairType;
		typedef HashMapConstIterator<KeyType, ValueType> Iterator;
		friend class HashMap<KeyType, ValueType>;
	public:
		HashMapConstIterator() : _pBucket(nullptr)
			, _Index(0)
		{

		}
		HashMapConstIterator(BucketType* pBucket, size_t Index) : _pBucket(pBucket), _Index(Index)
		{

		}
		HashMapConstIterator(const HashMapIterator<KeyType, ValueType>& It) : _pBucket(It.GetBucket()), _Index(It.GetIndex())
		{

		}
		Iterator& operator++(int)
		{
			Inc();
			return *this;
		}
		Iterator& operator++()
		{
			Inc();
			return *this;
		}
		void Inc()
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < _pBucket->NumPairs);
			_Index++;
			if (_Index >= _pBucket->NumPairs)
			{
				_Index = 0;
				_pBucket = _pBucket->pNext;
			}
		}
		HashMapPairType& operator*() const
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return *_pBucket->Pairs[_Index];
		}
		HashMapPairType& operator ()() const
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return *_pBucket->Pairs[_Index];
		}
		const KeyType& GetKey() const
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return _pBucket->Pairs[_Index]->Key;
		}
		ValueType& GetValue() const
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return _pBucket->Pairs[_Index]->Value;
		}
		operator bool() const
		{
			return _pBucket && _Index < _pBucket->NumPairs;
		}
		bool operator == (const Iterator& other) const
		{
			return _pBucket == other._pBucket && (_pBucket ? _Index == other._Index : true);
		}
		bool operator != (const Iterator& other) const
		{
			return _pBucket != other._pBucket || (_pBucket ? _Index != other._Index : false);
		}
		size_t GetIndex() const { return _Index; }
		BucketType* GetBucket() const { return _pBucket; }
	private:
		BucketType* _pBucket;
		size_t _Index;
	};

	template<class KeyType, class ValueType>
	class HashMap
	{
		typedef HashMapIterator<KeyType, ValueType> HashMapIteratorType;
		typedef HashMapConstIterator<KeyType, ValueType> HashMapConstIteratorType;
		typedef HashMapBucket<KeyType, ValueType> BucketType;
		typedef HashMapPair<KeyType, ValueType> HashMapPairType;
	public:
		HashMap() : _Bucket(32)
			, _Pairs(32)
		{
			_HashMap.Resize(32, nullptr);
		}
		HashMapIteratorType begin()
		{
			return HashMapIteratorType(_pFirstBucket, 0);
		}
		HashMapIteratorType end()
		{
			return HashMapIteratorType();
		}
		HashMapConstIteratorType begin() const
		{
			return HashMapConstIteratorType(_pFirstBucket, 0);
		}
		HashMapConstIteratorType end() const
		{
			return HashMapConstIteratorType();
		}
		HashMapIteratorType Find(const KeyType& Key)
		{
			const size_t Hash = std::hash<size_t>{}(Key);
			size_t Index = Hash % _HashMap.GetSize();
			BucketType* pBucket = _HashMap[Index];
			if (pBucket)
			{
#if CL_HASH_MAP_USE_KEY_ARRAY
				Index = pBucket->FindPair(Key);
#else
				Index = pBucket->FindPair(Key, Hash);
#endif	
				if (Index < CL_HASH_MAP_MAX_COLLISIONS)
				{
					return HashMapIteratorType(pBucket, Index);
				}
			}

			return HashMapIteratorType();
		}
		HashMapConstIteratorType Find(const KeyType& Key) const
		{
			const size_t Hash = std::hash<size_t>{}(Key);
			size_t Index = Hash % _HashMap.GetSize();
			const BucketType* pBucket = _HashMap[Index];
			if (pBucket)
			{
#if CL_HASH_MAP_USE_KEY_ARRAY
				Index = pBucket->FindPair(Key);
#else
				Index = pBucket->FindPair(Key, Hash);
#endif	
				if (Index < CL_HASH_MAP_MAX_COLLISIONS)
				{
					return HashMapConstIteratorType(pBucket, Index);
				}
			}

			return HashMapConstIteratorType();
		}
		HashMapIteratorType Erase(const HashMapConstIteratorType& It)
		{
			BucketType* pBucket = const_cast<BucketType*>(It._pBucket);

			if (pBucket)
			{
				const size_t Hash = It().Hash;
				pBucket->Erase(It.GetIndex(), _Pairs);
				_NumElemetns--;

				if (pBucket->IsEmpty())
				{
					BucketType* pNextBucket = pBucket->pNext;

					if (pBucket->pNext)
					{
						pBucket->pNext->pPrevious = pBucket->pPrevious;
					}

					if (pBucket->pPrevious)
					{
						pBucket->pPrevious->pNext = pBucket->pNext;
					}

					if (_pFirstBucket == pBucket)
					{
						_pFirstBucket = pBucket->pNext;
					}

					_Bucket.Free(pBucket);
					_HashMap[Hash % _HashMap.GetSize()] = nullptr;
					_NumBuckets--;

					return HashMapIteratorType(pNextBucket, 0);
				}
				else
				{
					return It ? HashMapIteratorType(pBucket, It.GetIndex()) : HashMapIteratorType(pBucket->pNext, 0);
				}
			}

			return HashMapIteratorType();
		}
		bool Insert(const KeyType& Key, const ValueType& Value)
		{
			return Insert(HashMapPairType(Key, Value));
		}
		bool Insert(const HashMapPairType& InPair)
		{
			size_t Hash = std::hash<size_t>{}(InPair.Key);
			size_t Index = Hash % _HashMap.GetSize();
			BucketType* pBucket = _HashMap[Index];
#if CL_HASH_MAP_USE_KEY_ARRAY
			if (pBucket && pBucket->FindPair(InPair.Key) < CL_HASH_MAP_MAX_COLLISIONS)
#else
			if (pBucket && pBucket->FindPair(InPair.Key, Hash) < CL_HASH_MAP_MAX_COLLISIONS)
#endif
			{
				return false;
			}

			if (float(_NumBuckets) / _HashMap.GetSize() > CL_HASH_MAP_LOAD_FACTOR)
			{
				ReHash(_HashMap.GetSize() * CL_HASH_MAP_REHASH_SIZE_MULTIPLIER);
				Index = Hash % _HashMap.GetSize();
			}

			BucketType* pNewBucket = _HashMap[Index];
			while (pNewBucket && !pNewBucket->IsAbleToInsert())
			{
				ReHash(_HashMap.GetSize() * CL_HASH_MAP_REHASH_SIZE_MULTIPLIER);
				Index = Hash % _HashMap.GetSize();
				pNewBucket = _HashMap[Index];
			}

			if (!pNewBucket)
			{
				pNewBucket = _Bucket.Alloc();
				_NumBuckets++;
				pNewBucket->pNext = _pFirstBucket;

				if (_pFirstBucket)
				{
					_pFirstBucket->pPrevious = pNewBucket;
				}

				_pFirstBucket = pNewBucket;
				_HashMap[Index] = pNewBucket;
			}

			HashMapPairType* pPair = _Pairs.Alloc(InPair);
			pPair->Hash = Hash;
			pNewBucket->Insert(pPair);

			_NumElemetns++;

			return true;
		}
		size_t GetNumElements() const
		{
			return _NumElemetns;
		}
		void Clean()
		{
			_HashMap.Clear();

			while (_pFirstBucket)
			{
				for (size_t I = 0; I < _pFirstBucket->NumPairs; I++)
				{
					_Pairs.Free(_pFirstBucket->Pairs[I]);
				}

				BucketType* pOldBucket = _pFirstBucket;
				_pFirstBucket = _pFirstBucket->pNext;
				_Bucket.Free(pOldBucket);
			}

			_pFirstBucket = nullptr;
			_NumElemetns = 0;
			_NumBuckets = 0;
		}
		~HashMap()
		{
			Clean();
		}
	private:
#if CL_ENABLE_PROBING_INSERT
		size_t ProbingInsert(size_t StartIndex) const
		{
			size_t Index = StartIndex;

			for (size_t i = 1; i < CL_HASH_MAP_MAX_PROBING_TAPS && _HashMap[Index]; i++)
			{
				Index = (StartIndex + (i + i * i) / 2) % _HashMap.GetSize();
			}

			return _HashMap[Index] ? -1 : Index;
		}
		BucketType* ProbingSearch(size_t& StartIndex, const KeyType& Key) const
		{
			size_t Index = 0;

			for (size_t i = 0; i < CL_HASH_MAP_MAX_PROBING_TAPS; i++)
			{
				Index = (StartIndex + (i + i * i) / 2) % _HashMap.GetSize();

				BucketType* pBucket = _HashMap[Index];

				if (pBucket && pBucket->Pair.Key == Key)
				{
					StartIndex = Index;
					return pBucket;
				}
			}

			return nullptr;
		}
#endif
		void ReHash(size_t NewSize)
		{
			_HashMap.Clear();
			_HashMap.Resize(NewSize, nullptr);

			CL_DEBUG_ASSERT(_HashMap.GetSize() == NewSize);

			BucketType* pBucket = _pFirstBucket;

			_pFirstBucket = nullptr;
			BucketType* mBucketPool[CL_HASH_MAP_MAX_REHASH_POOL_SIZE];
			size_t PoolSize = 0;

			while (pBucket)
			{
				CL_DEBUG_ASSERT(pBucket->NumPairs > 0);
				CL_DEBUG_ASSERT(pBucket->NumPairs <= CL_HASH_MAP_MAX_COLLISIONS);

				HashMapPairType** pPair = pBucket->Pairs;
				HashMapPairType** pLastPair = pPair + pBucket->NumPairs;

				while (pPair != pLastPair)
				{
					const size_t Index = (*pPair)->Hash % _HashMap.GetSize();

					BucketType* pTargetBucket = _HashMap[Index];

					if (!pTargetBucket)
					{
						if (PoolSize > 0)
						{
							PoolSize--;
							pTargetBucket = mBucketPool[PoolSize];
						}
						else
						{
							pTargetBucket = _Bucket.Alloc();
						}

						_NumBuckets++;
						_HashMap[Index] = pTargetBucket;

						if (_pFirstBucket)
						{
							_pFirstBucket->pPrevious = pTargetBucket;
							pTargetBucket->pNext = _pFirstBucket;
						}

						_pFirstBucket = pTargetBucket;
					}

					pTargetBucket->Insert(*pPair);
					pPair++;
				}

				BucketType* pOldBucket = pBucket;
				pBucket = pBucket->pNext;

				if (PoolSize < CL_HASH_MAP_MAX_REHASH_POOL_SIZE)
				{
					mBucketPool[PoolSize++] = pOldBucket;
					pOldBucket->pNext = nullptr;
					pOldBucket->pPrevious = nullptr;
					pOldBucket->NumPairs = 0;
				}
				else
				{
					_Bucket.Free(pOldBucket);
				}
				
				_NumBuckets--;
			}

			for (size_t I = 0; I < PoolSize; I++)
			{
				_Bucket.Free(mBucketPool[I]);
			}

#if CL_HASH_MAP_DEBUG
			pBucket = _pFirstBucket;

			while (pBucket)
			{
				for (size_t I = 0; I < pBucket->NumPairs; I++)
				{
					auto It = Find(pBucket->Pairs[I]->Key);
					CL_ASSERT(It);
					CL_ASSERT(It.GetIndex() == I);
					CL_ASSERT(It.GetBucket() == pBucket);
				}
				pBucket = pBucket->pNext;
			}
#endif
		}

		size_t _NumElemetns = 0;
		size_t _NumBuckets = 0;
		BucketType* _pFirstBucket = nullptr;

		CL::Pool<HashMapPairType> _Pairs;
		CL::Pool<BucketType> _Bucket;
		CL::Vector<BucketType*> _HashMap;
	};
}