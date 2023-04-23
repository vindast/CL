#pragma once 
#include <vector>
#include <assert.h>
#include <new>
#include "CLObjects/CLCriticalSection.h"
#include "CLMemory.h"

namespace CL
{
	template<class objType> struct ObjectPoolNode
	{
		ObjectPoolNode* pNext;
		objType data;
	};

	template<class objType, size_t nObjects> class ObjectPool
	{
	public:
		ObjectPool()
		{
			pData = reinterpret_cast<ObjectPoolNode<objType>*>(CL_MALLOC(sizeof(ObjectPoolNode<objType>) * nCurrentBlocSize));

			for (size_t i = 0; i < nCurrentBlocSize - 1; i++)
			{
				pData[i].pNext = &pData[i + 1];
			}

			pData[nCurrentBlocSize - 1].pNext = nullptr;

			pCurrentFreeNod = &pData[0];
		}
		ObjectPoolNode<objType>* alloc()
		{
			ObjectPoolNode<objType>* pNode = nullptr;

			if (pCurrentFreeNod)
			{ 
				pNode = pCurrentFreeNod;
				pCurrentFreeNod = pCurrentFreeNod->pNext;

				CL_PLACEMENT_NEW(&pNode->data, objType);
			}

			return pNode;
		}
		void free(ObjectPoolNode<objType>* pNode)
		{
			CL_PLACEMENT_DELETE(pNode->data);

			pNode->pNext = pCurrentFreeNod;
			pCurrentFreeNod = pNode;
		}
		~ObjectPool()
		{
			CL_FREE(pData);
		}
	private:
		ObjectPool(const ObjectPool<objType, nObjects>&) = delete;
		ObjectPool<objType, nObjects> operator = (const ObjectPool<objType, nObjects>&) = delete;
		
		size_t nCurrentBlocSize = nObjects;

		ObjectPoolNode<objType>* pData = nullptr;
		ObjectPoolNode<objType>* pCurrentFreeNod = nullptr;
	};
	 
	template<class objType, size_t nInitialObjects> class ObjectPoolDynamic
	{
	public:
		ObjectPoolDynamic()
		{
		
		}
		template<class ...ConstructorParams> ObjectPoolNode<objType>* alloc(ConstructorParams... params)
		{
			ObjectPoolNode<objType>* pNode = nullptr;

			if (pCurrentFreeNod)
			{
				pNode = pCurrentFreeNod;
				pCurrentFreeNod = pCurrentFreeNod->pNext;
			}
			else
			{
				createNewBuffer();

				pNode = alloc(params...);
			}

			CL_PLACEMENT_NEW(&pNode->data, objType, params...);

			return pNode;
		}
		template<class ...ConstructorParams> ObjectPoolNode<objType>* allocAsync(ConstructorParams... params)
		{
			const LockGuard<CriticalSection> lock(_hCS);

			ObjectPoolNode<objType>* pNode = nullptr;

			if (pCurrentFreeNod)
			{
				pNode = pCurrentFreeNod;
				pCurrentFreeNod = pCurrentFreeNod->pNext;
			}
			else
			{
				createNewBuffer();

				pNode = alloc(params...);
			}

			CL_PLACEMENT_NEW(&pNode->data, objType, params...);

			return pNode;
		}
		void free(ObjectPoolNode<objType>* pNode)
		{
			CL_PLACEMENT_DELETE(&pNode->data);

			pNode->pNext = pCurrentFreeNod;
			pCurrentFreeNod = pNode;
		}
		void freeAsync(ObjectPoolNode<objType>* pNode)
		{ 
			CL_PLACEMENT_DELETE(&pNode->data);

			const LockGuard<CriticalSection> lock(_hCS);
			pNode->pNext = pCurrentFreeNod;
			pCurrentFreeNod = pNode;
		}
		~ObjectPoolDynamic()
		{
			for (size_t i = 0; i < mDataPtr.size(); i++)
			{
				CL_FREE(mDataPtr[i]);
			} 
		}
	private:
		ObjectPoolDynamic(const ObjectPoolDynamic<objType, nInitialObjects>&) = delete;
		ObjectPoolDynamic<objType, nInitialObjects> operator = (const ObjectPoolDynamic<objType, nInitialObjects>&) = delete;
		 
		size_t nCurrentBlocSize = nInitialObjects;

		ObjectPoolNode<objType>* pCurrentFreeNod = nullptr;
		std::vector<ObjectPoolNode<objType>*> mDataPtr;
		CriticalSection _hCS;

		void createNewBuffer()
		{
			assert(!pCurrentFreeNod);

			auto pData = reinterpret_cast<ObjectPoolNode<objType>*>(CL_MALLOC(sizeof(ObjectPoolNode<objType>) * nCurrentBlocSize));
			for (size_t i = 0; i < nCurrentBlocSize - 1; i++)
			{
				pData[i].pNext = &pData[i + 1];
			}

			pData[nCurrentBlocSize - 1].pNext = nullptr;
			mDataPtr.push_back(pData);
			pCurrentFreeNod = &pData[0];

			nCurrentBlocSize *= 2;
		}
	};
};