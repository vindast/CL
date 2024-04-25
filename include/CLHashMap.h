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

namespace CL
{
	typedef size_t KeyType;
	typedef CL::String ValueType;

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

	struct Bucket
	{
		Bucket()
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
		
		HashMapPair* Pairs[CL_HASH_MAP_MAX_COLLISIONS];
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
		void Insert(HashMapPair* pPair)
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
		void Erase(size_t Index, CL::Pool<HashMapPair>& PairsPool)
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
		
		Bucket* pNext = nullptr;
		Bucket* pPrevious = nullptr;
	};

	class HashMapIterator
	{
		typedef Bucket BucketType;
		typedef HashMapPair ObjType;
		typedef HashMapIterator Iterator;
		friend class HashMap;
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
		ObjType& operator*()
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return *_pBucket->Pairs[_Index];
		}
		ObjType& operator ()()
		{
			CL_DEBUG_ASSERT(_pBucket);
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return *_pBucket->Pairs[_Index];
		}
		const KeyType& GetKey()
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

	class HashMap
	{
	public:
		HashMap() : _Bucket(32)
			, _Pairs(32)
		{
			_HashMap.Resize(32, nullptr);
		}
		HashMapIterator begin()
		{
			return HashMapIterator(_pFirstBucket, 0);
		}
		HashMapIterator end()
		{
			return HashMapIterator();
		}
		HashMapIterator Find(const KeyType& Key)
		{
			const size_t Hash = std::hash<size_t>{}(Key);
			size_t Index = Hash % _HashMap.GetSize();
			Bucket* pBucket = _HashMap[Index];
			if (pBucket)
			{
#if CL_HASH_MAP_USE_KEY_ARRAY
				Index = pBucket->FindPair(Key);
#else
				Index = pBucket->FindPair(Key, Hash);
#endif	
				if (Index < CL_HASH_MAP_MAX_COLLISIONS)
				{
					return HashMapIterator(pBucket, Index);
				}
			}

			return HashMapIterator();
		}
		HashMapIterator Erase(HashMapIterator It)
		{
			Bucket* pBucket = It._pBucket;

			if (pBucket)
			{
				const size_t Hash = It().Hash;
				pBucket->Erase(It.GetIndex(), _Pairs);
				_NumElemetns--;

				if (pBucket->IsEmpty())
				{
					Bucket* pNextBucket = pBucket->pNext;

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

					return HashMapIterator(pNextBucket, 0);
				}
				else
				{
					return It ? It : HashMapIterator(pBucket->pNext, 0);
				}
			}

			return HashMapIterator();
		}
		bool Insert(const HashMapPair& InPair)
		{
			size_t Hash = std::hash<size_t>{}(InPair.Key);
			size_t Index = Hash % _HashMap.GetSize();
			Bucket* pBucket = _HashMap[Index];
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
				ReHash();
				Index = Hash % _HashMap.GetSize();
			}

			Bucket* pNewBucket = _HashMap[Index];
			while (pNewBucket && !pNewBucket->IsAbleToInsert())
			{
				ReHash();
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

			HashMapPair* pPair = _Pairs.Alloc(InPair);
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

				Bucket* pOldBucket = _pFirstBucket;
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
		Bucket* ProbingSearch(size_t& StartIndex, const KeyType& Key) const
		{
			size_t Index = 0;

			for (size_t i = 0; i < CL_HASH_MAP_MAX_PROBING_TAPS; i++)
			{
				Index = (StartIndex + (i + i * i) / 2) % _HashMap.GetSize();

				Bucket* pBucket = _HashMap[Index];

				if (pBucket && pBucket->Pair.Key == Key)
				{
					StartIndex = Index;
					return pBucket;
				}
			}

			return nullptr;
		}
#endif
		void ReHash()
		{
			size_t NewSize = _HashMap.GetSize() * CL_HASH_MAP_REHASH_SIZE_MULTIPLIER;
			_HashMap.Clear();
			_HashMap.Resize(NewSize, nullptr);

			CL_DEBUG_ASSERT(_HashMap.GetSize() == NewSize);

			Bucket* pBucket = _pFirstBucket;

			_pFirstBucket = nullptr;
			Bucket* mBucketPool[CL_HASH_MAP_MAX_REHASH_POOL_SIZE];
			size_t PoolSize = 0;

			while (pBucket)
			{
				CL_DEBUG_ASSERT(pBucket->NumPairs > 0);
				CL_DEBUG_ASSERT(pBucket->NumPairs <= CL_HASH_MAP_MAX_COLLISIONS);

				HashMapPair** pPair = pBucket->Pairs;
				HashMapPair** pLastPair = pPair + pBucket->NumPairs;

				while (pPair != pLastPair)
				{
					const size_t Index = (*pPair)->Hash % _HashMap.GetSize();

					Bucket* pTargetBucket = _HashMap[Index];

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

				Bucket* pOldBucket = pBucket;
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
		Bucket* _pFirstBucket = nullptr;

		CL::Pool<HashMapPair> _Pairs;
		CL::Pool<Bucket> _Bucket;
		CL::Vector<Bucket*> _HashMap;
	};
}