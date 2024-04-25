#pragma once
#include <CLMacroHelper.h>
#include <CLHashString.h>
#include <CLMemory.h>
#include <CLList.h>
#include <CLEmbeddedArray.h>
#include <CLVector.h>

#define CL_HASH_MAP_LOAD_FACTOR 0.85

// #TODO CLConfig
#define CL_ENABLE_PROBING_INSERT 0
#define CL_HASH_MAP_USE_KEY_ARRAY 0

#if CL_ENABLE_PROBING_INSERT
#define CL_HASH_MAP_MAX_PROBING_TAPS 4
#define CL_HASH_MAP_REHASH_SIZE_MULTIPLIER 8
#else
#define CL_HASH_MAP_REHASH_SIZE_MULTIPLIER 2
#endif

// #TODO CLConfig
#define CL_HASH_MAP_ENABLE_COLLISION_RESOLVE 1
//#define CL_PARANOIDAL_DEBUG 1

#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
#define CL_HASH_MAP_MAX_COLLISIONS 4
#define CL_HASH_MAP_MAX_REHASH_POOL_SIZE 16
#endif

#if CL_PARANOIDAL_DEBUG && CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
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
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
		//	memset(Pairs, 0, sizeof(HashMapPair) * CL_HASH_MAP_MAX_COLLISIONS);

			for (size_t I = 0; I < 4; I++)
			{
				Pairs[I] = nullptr;
			}
#endif
		}

#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
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
		void Erase(size_t Index)
		{
			CL_DEBUG_ASSERT(NumPairs > 0);

			NumPairs--;

			if (NumPairs != Index)
			{
				Pairs[Index] = Pairs[NumPairs];
				Hashes[Index] = Hashes[NumPairs];
			}

			Pairs[NumPairs] = nullptr;
		}
		bool IsAbleToInsert() const { return NumPairs < CL_HASH_MAP_MAX_COLLISIONS; }
		bool IsEmpty() const { return NumPairs == 0; }

#else
		HashMapPair Pair;
#endif
		
		Bucket* pNext = nullptr;
		Bucket* pPrevious = nullptr;
	};

	class HashMapIterator
	{
		typedef Bucket BucketType;
		typedef HashMapPair ObjType;
		typedef HashMapIterator Iterator;
	public:
		HashMapIterator() : _pBucket(nullptr)
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			, _Index(0)
#endif
		{

		}
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
		HashMapIterator(BucketType* pBucket, size_t Index) : _pBucket(pBucket), _Index(Index)
		{

		}
#else
		HashMapIterator(BucketType* pBucket) : _pBucket(pBucket)
		{

		}
#endif	
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
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			CL_DEBUG_ASSERT(_Index < _pBucket->NumPairs);
			_Index++;
			if (_Index >= _pBucket->NumPairs)
			{
				_Index = 0;
				_pBucket = _pBucket->pNext;
		}
#else
			_pBucket = _pBucket->pNext;
#endif
		}
		ObjType& operator*()
		{
			CL_DEBUG_ASSERT(_pBucket);
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return *_pBucket->Pairs[_Index];
#else
			return _pBucket->Pair;
#endif
		}
		ObjType& operator ()()
		{
			CL_DEBUG_ASSERT(_pBucket);
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return *_pBucket->Pairs[_Index];
#else
			return _pBucket->Pair;
#endif
		}
		const KeyType& GetKey()
		{
			CL_DEBUG_ASSERT(_pBucket);
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return _pBucket->Pairs[_Index]->Key;
#else
			return _pBucket->Pair.Key;
#endif
		}
		ValueType& GetValue()
		{
			CL_DEBUG_ASSERT(_pBucket);
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return _pBucket->Pairs[_Index]->Value;
#else
			return _pBucket->Pair.Value;
#endif
		}
		const ValueType& GetValue() const
		{
			CL_DEBUG_ASSERT(_pBucket);
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			CL_DEBUG_ASSERT(_Index < CL_HASH_MAP_MAX_COLLISIONS);
			return _pBucket->Pairs[_Index]->Value;
#else
			return _pBucket->Pair.Value;
#endif
		}
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
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
#else
		operator bool() const
		{
			return _pBucket;
		}
		bool operator == (const Iterator& other) const
		{
			return _pBucket == other._pBucket;
		}
		bool operator != (const Iterator& other) const
		{
			return _pBucket != other._pBucket;
		}
#endif
		const BucketType* GetBucket() const { return _pBucket; }
	private:
		BucketType* _pBucket;

#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
		size_t _Index;
#endif
	};

	class HashMap
	{
	public:
		HashMap() : _Bucket(32)
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			, _Parirs(32)
#endif
		{
			_HashMap.Resize(64, nullptr);
		}
		HashMapIterator begin()
		{
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			return HashMapIterator(_pFirstBucket, 0);
#else
			return HashMapIterator(_pFirstBucket);
#endif
		}
		HashMapIterator end()
		{
			return HashMapIterator();
		}
		HashMapIterator Find(const KeyType& Key)
		{
			const size_t Hash = std::hash<size_t>{}(Key);
			size_t Index = Hash % _HashMap.GetSize();
#if CL_ENABLE_PROBING_INSERT
			Bucket* pBucket = ProbingSearch(Index, Key);
			return HashMapIterator(pBucket);
#else
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
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
#else
			Bucket* pBucket = _HashMap[Index];
			return HashMapIterator(pBucket);
#endif
#endif
		}
		HashMapIterator Erase(HashMapIterator It)
		{
			size_t Index = It().Hash % _HashMap.GetSize();

#if CL_ENABLE_PROBING_INSERT
			Bucket* pBucket = ProbingSearch(Index, It.GetKey());
#else
			Bucket* pBucket = _HashMap[Index];
#endif

			if (pBucket)
			{
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
				_Parirs.Free(pBucket->Pairs[It.GetIndex()]);
				pBucket->Erase(It.GetIndex());
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
					_HashMap[Index] = nullptr;
					_NumBuckets--;

					return HashMapIterator(pNextBucket, 0);
				}
				else
				{
					return It ? It : HashMapIterator(pBucket->pNext, 0);
				}
#else
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
				_HashMap[Index] = nullptr;
				_NumBuckets--;
				_NumElemetns--;
				return HashMapIterator(pNextBucket);
#endif
			}

			return HashMapIterator();
		}
		bool Insert(const HashMapPair& InPair)
		{
			size_t Hash = std::hash<size_t>{}(InPair.Key);
			size_t Index = Hash % _HashMap.GetSize();
#if CL_ENABLE_PROBING_INSERT
			Bucket* pBucket = ProbingSearch(Index, InPair.Key);
#else
			Bucket* pBucket = _HashMap[Index];
#endif

#if CL_ENABLE_PROBING_INSERT
			if (pBucket)
#else
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
#if CL_HASH_MAP_USE_KEY_ARRAY
			if (pBucket && pBucket->FindPair(InPair.Key) < CL_HASH_MAP_MAX_COLLISIONS)
#else
			if (pBucket && pBucket->FindPair(InPair.Key, Hash) < CL_HASH_MAP_MAX_COLLISIONS)
#endif
#else
			if (pBucket && pBucket->Pair.Key == InPair.Key)
#endif
#endif
			{
				return false;
			}

			if (float(_NumBuckets) / _HashMap.GetSize() > CL_HASH_MAP_LOAD_FACTOR)
			{
				ReHash();
				Index = Hash % _HashMap.GetSize();
			}

#if CL_ENABLE_PROBING_INSERT
			Index = ProbingInsert(Index);
#endif

#if CL_ENABLE_PROBING_INSERT
			while (Index == -1)
#else
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			while (_HashMap[Index] && !_HashMap[Index]->IsAbleToInsert())
#else
			while (_HashMap[Index])
#endif
#endif
			{
				ReHash();
#if CL_ENABLE_PROBING_INSERT
				Index = ProbingInsert(Hash % _HashMap.GetSize());
#else
				Index = Hash % _HashMap.GetSize();
#endif
			}

#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			Bucket* pNewBucket = _HashMap[Index];
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

			HashMapPair* pPair = _Parirs.Alloc(InPair);
			pPair->Hash = Hash;
			pNewBucket->Insert(pPair);
#else
			Bucket* pNewBucket = _Bucket.Alloc();
			_NumBuckets++;
			pNewBucket->pNext = _pFirstBucket;

			if (_pFirstBucket)
			{
				_pFirstBucket->pPrevious = pNewBucket;
			}

			_pFirstBucket = pNewBucket;
			_HashMap[Index] = pNewBucket;

			pNewBucket->Pair = InPair;
			pNewBucket->Pair.Hash = Hash;
#endif

			_NumElemetns++;

			return true;
		}
		size_t GetNumElements() const
		{
			return _NumElemetns;
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

#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			_pFirstBucket = nullptr;
			Bucket* mBucketPool[CL_HASH_MAP_MAX_REHASH_POOL_SIZE];
			size_t PoolSize = 0;
#endif

			while (pBucket)
			{
#if CL_ENABLE_PROBING_INSERT
				const size_t Index = ProbingInsert(pBucket->Pair.Hash % _HashMap.GetSize());
				_HashMap[Index] = pBucket;
#else
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
// #TODO optimize
				for (size_t I = 0; I < pBucket->NumPairs; I++)
				{
					HashMapPair* pPair = pBucket->Pairs[I];
					if (pPair)
					{
						const size_t Index = pPair->Hash % _HashMap.GetSize();
						
						if (!_HashMap[Index])
						{
							Bucket* pNewBucket;

							if (PoolSize > 0)
							{
								PoolSize--;
								pNewBucket = mBucketPool[PoolSize];
							}
							else
							{
								pNewBucket = _Bucket.Alloc();
							}

							_NumBuckets++;
							_HashMap[Index] = pNewBucket;

							if (_pFirstBucket)
							{
								_pFirstBucket->pPrevious = pNewBucket;
								pNewBucket->pNext = _pFirstBucket;
							}
							
							_pFirstBucket = pNewBucket;
						}

						_HashMap[Index]->Insert(pPair);
					}
				}

				pBucket->NumPairs = 0;
#else
				const size_t Index = pBucket->Pair.Hash % _HashMap.GetSize(); 
				CL_DEBUG_ASSERT(!_HashMap[Index]);
				_HashMap[Index] = pBucket;
#endif
#endif			

#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
				Bucket* pOldBucket = pBucket;
#endif
				pBucket = pBucket->pNext;
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE

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
#endif
			}

#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
			for (size_t I = 0; I < PoolSize; I++)
			{
				_Bucket.Free(mBucketPool[I]);
			}
#endif

#if CL_HASH_MAP_DEBUG && CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
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
#if CL_HASH_MAP_ENABLE_COLLISION_RESOLVE
		CL::Pool<HashMapPair> _Parirs;
#endif
		CL::Pool<Bucket> _Bucket;
		CL::Vector<Bucket*> _HashMap;
	};
}