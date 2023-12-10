#pragma once
#include "CLRefPtr.h"
#include "CLPool2.h"

namespace CL
{
	template<class ObjType>
	class ListForwardIterator
	{
		typedef ListForwardIterator<ObjType> Iterator;
		typedef TwoLinkedNode<ObjType> ListNode;
		template<class t> 
		friend class List;
	public:
		Iterator& operator++(int)
		{
			CL_DEBUG_ASSERT(_pCurrentNode);
			_pCurrentNode = _pCurrentNode->pNext;
			return *this;
		}
		Iterator& operator++()
		{
			CL_DEBUG_ASSERT(_pCurrentNode);
			_pCurrentNode = _pCurrentNode->pNext;
			return *this;
		}
		ObjType& operator*()
		{
			CL_DEBUG_ASSERT(_pCurrentNode);
			return _pCurrentNode->Obj;
		}
		ObjType& operator ()()
		{
			CL_DEBUG_ASSERT(_pCurrentNode);
			return _pCurrentNode->Obj;
		}
		auto& operator ->()
		{
			CL_DEBUG_ASSERT(_pCurrentNode);
			return *_pCurrentNode->Obj;
		}
		operator bool () const
		{
			return _pCurrentNode;
		}
		bool operator == (const Iterator& other) const
		{
			return _pCurrentNode == other._pCurrentNode;
		}
		bool operator != (const Iterator& other) const
		{
			return _pCurrentNode != other._pCurrentNode;
		}
	private:
#ifdef _DEBUG
		List<ObjType>* _pList = nullptr;
#endif

		ListNode* _pCurrentNode = nullptr;
	};

	template<class ObjType>
	class ListConstForwardIterator
	{
		typedef ListConstForwardIterator<ObjType> Iterator;
		typedef TwoLinkedNode<ObjType> ListNode;
		template<class t>
		friend class List;
	public:
		Iterator& operator++(int)
		{
			CL_DEBUG_ASSERT(_pCurrentNode);
			_pCurrentNode = _pCurrentNode->pNext;
			return *this;
		}
		Iterator& operator++()
		{
			CL_DEBUG_ASSERT(_pCurrentNode);
			_pCurrentNode = _pCurrentNode->pNext;
			return *this;
		}
		const ObjType& operator*()
		{
			CL_DEBUG_ASSERT(_pCurrentNode);
			return _pCurrentNode->Obj;
		}
		const ObjType& operator ()()
		{
			CL_DEBUG_ASSERT(_pCurrentNode);
			return _pCurrentNode->Obj;
		}
		const ObjType& operator ->()
		{
			CL_DEBUG_ASSERT(_pCurrentNode);
			return _pCurrentNode->Obj;
		}
		operator bool() const
		{
			return _pCurrentNode;
		}
		bool operator == (const Iterator& other) const
		{
			return _pCurrentNode == other._pCurrentNode;
		}
		bool operator != (const Iterator& other) const
		{
			return _pCurrentNode != other._pCurrentNode;
		}
	private:
#ifdef _DEBUG
		const List<ObjType>* _pList = nullptr;
#endif

		ListNode* _pCurrentNode = nullptr;
	};

	template<class ObjType>
	class List final
	{
	public:
		typedef ListForwardIterator<ObjType> ForwardIterator;
		typedef ListConstForwardIterator<ObjType> ConstForwardIterator;
		typedef TwoLinkedNode<ObjType> ListNode;

		List(CL::RefPtr<Pool<ListNode>>& pAllocator = CL::RefPtr<Pool<ListNode>>::MakeRefPtr(256)):
			_pAllocator(pAllocator), _pFirst(nullptr), _pLast(nullptr), _nElementsCount(0)
		{

		}
		List(const List& Other)
		{
			if (Other.IsEmpty())
			{
				return;
			}

			_pAllocator = CL::RefPtr<Pool<ListNode>>::MakeRefPtr(Other._pAllocator->GetElementsInBlock());

			for (const auto& it : Other)
			{
				PushBack(it);
			}
		}
		List(List&& Other)
		{
			_pAllocator = Other._pAllocator;
			_pFirst = Other._pFirst;
			_pLast = Other._pLast;
			_nElementsCount = Other._nElementsCount;

			Other._pAllocator.Free();
			Other._pFirst = nullptr;
			Other._pLast = nullptr;
			Other._nElementsCount = 0;
		}
		List& operator = (const List& Other)
		{
			if (Other.IsEmpty())
			{
				Clear();
				return;
			}

			_pAllocator = CL::RefPtr<Pool<ListNode>>::MakeRefPtr(Other._pAllocator->GetElementsInBlock());

			for (const auto& it : Other)
			{
				PushBack(it);
			}

			return *this;
		}
		List& operator = (List&& Other)
		{
			_pAllocator = Other._pAllocator;
			_pFirst = Other._pFirst;
			_pLast = Other._pLast;
			_nElementsCount = Other._nElementsCount;

			Other._pAllocator.Free();
			Other._pFirst = nullptr;
			Other._pLast = nullptr;
			Other._nElementsCount = 0;

			return *this;
		}
		ForwardIterator begin()
		{
			ForwardIterator it;
			it._pCurrentNode = _pFirst;
#ifdef _DEBUG
			it._pList = this;
#endif
			return it;
		}
		ForwardIterator end()
		{
			return ForwardIterator();
		}
		ConstForwardIterator begin() const
		{
			ConstForwardIterator it;
			it._pCurrentNode = _pFirst;
#ifdef _DEBUG
			it._pList = this;
#endif
			return it;
		}
		ConstForwardIterator end() const
		{
			return ConstForwardIterator();
		}
		ConstForwardIterator ConstBegin() const
		{
			ConstForwardIterator it;
			it._pCurrentNode = _pFirst;
#ifdef _DEBUG
			it._pList = this;
#endif
			return it;
		}
		ConstForwardIterator ConstEnd() const
		{
			return ConstForwardIterator();
		}
		ForwardIterator First()
		{
			ForwardIterator it;
			it._pCurrentNode = _pFirst;
#ifdef _DEBUG
			it._pList = this;
#endif
			return it;
		}
		ForwardIterator Last()
		{
			ForwardIterator it;
			it._pCurrentNode = _pLast;
#ifdef _DEBUG
			it._pList = this;
#endif
			return it;
		}
		ConstForwardIterator First() const
		{
			ConstForwardIterator it;
			it._pCurrentNode = _pFirst;
#ifdef _DEBUG
			it._pList = this;
#endif
			return it;
		}
		ConstForwardIterator Last() const
		{
			ConstForwardIterator it;
			it._pCurrentNode = _pLast;
#ifdef _DEBUG
			it._pList = this;
#endif
			return it;
		}
		void PushFront(const ObjType& obj)
		{
			ListNode* pNewNode = _pAllocator->Alloc(obj);

			if (_pFirst)
			{
				_pFirst->pPrevios = pNewNode;
				pNewNode->pNext = _pFirst;
				_pFirst = pNewNode;
			}
			else
			{
				_pFirst = pNewNode;
				_pLast = pNewNode;
			}

			_nElementsCount++;
		}
		void EmplaceFront(ObjType&& obj)
		{
			ListNode* pNewNode = _pAllocator->Alloc(Forward(obj));

			if (_pFirst)
			{
				_pFirst->pPrevios = pNewNode;
				pNewNode->pNext = _pFirst;
				_pFirst = pNewNode;
			}
			else
			{
				_pFirst = pNewNode;
				_pLast = pNewNode;
			}

			_nElementsCount++;
		}
		void PushBack(const ObjType& obj)
		{
			ListNode* pNewNode = _pAllocator->Alloc(obj);

			if (_pLast)
			{
				_pLast->pNext = pNewNode;
				pNewNode->pPrevios = _pLast;
				_pLast = pNewNode;
			}
			else
			{
				_pFirst = pNewNode;
				_pLast = pNewNode;
			}

			_nElementsCount++;
		}
		void EmplaceBack(ObjType&& obj)
		{
			ListNode* pNewNode = _pAllocator->Alloc(Forward(obj));

			if (_pLast)
			{
				_pLast->pNext = pNewNode;
				pNewNode->pPrevios = _pLast;
				_pLast = pNewNode;
			}
			else
			{
				_pFirst = pNewNode;
				_pLast = pNewNode;
			}

			_nElementsCount++;
		}
		void PopBack()
		{
			Erase(Last());
		}
		ForwardIterator Erase(const ForwardIterator& It)
		{
			CL_DEBUG_ASSERT(It._pList == this);
			CL_ASSERT(It._pCurrentNode);

			if (It._pCurrentNode != _pFirst && It._pCurrentNode != _pLast)
			{
				It._pCurrentNode->pPrevios->pNext = It._pCurrentNode->pNext;
				It._pCurrentNode->pNext->pPrevios = It._pCurrentNode->pPrevios;
			}
			else
			{
				if (It._pCurrentNode == _pFirst)
				{
					_pFirst = It._pCurrentNode->pNext;
					if (_pFirst)
					{
						_pFirst->pPrevios = nullptr;
					}
				}

				if (It._pCurrentNode == _pLast)
				{
					_pLast = It._pCurrentNode->pPrevios;

					if (_pLast)
					{
						_pLast->pNext = nullptr;
					}
				}
			}

			ForwardIterator NewIt;
			NewIt._pCurrentNode = It._pCurrentNode->pNext;
#ifdef _DEBUG
			NewIt._pList = this;
#endif // _DEBUG

			_pAllocator->Free(It._pCurrentNode);
			_nElementsCount--;

			return NewIt;
		}
		ConstForwardIterator Erase(const ConstForwardIterator& It)
		{
			CL_DEBUG_ASSERT(It._pList == this);
			CL_ASSERT(It._pCurrentNode);

			if (It._pCurrentNode != _pFirst && It._pCurrentNode != _pLast)
			{
				It._pCurrentNode->pPrevios->pNext = It._pCurrentNode->pNext;
				It._pCurrentNode->pNext->pPrevios = It._pCurrentNode->pPrevios;
			}
			else
			{
				if (It._pCurrentNode == _pFirst)
				{
					_pFirst = It._pCurrentNode->pNext;
					if (_pFirst)
					{
						_pFirst->pPrevios = nullptr;
					}
				}

				if (It._pCurrentNode == _pLast)
				{
					_pLast = It._pCurrentNode->pPrevios;

					if (_pLast)
					{
						_pLast->pNext = nullptr;
					}
				}
			}

			ConstForwardIterator NewIt;
			NewIt._pCurrentNode = It._pCurrentNode->pNext;
#ifdef _DEBUG
			NewIt._pList = this;
#endif // _DEBUG

			_pAllocator->Free(It._pCurrentNode);
			_nElementsCount--;

			return NewIt;
		}
		size_t size() const { return _nElementsCount; }
		size_t GetElementsCount() const { return _nElementsCount; }
		bool IsEmpty() const { return _nElementsCount == 0; }
		void Clear()
		{
			while (_pFirst)
			{
				ListNode* pTMP = _pFirst->pNext;
				_pAllocator->Free(_pFirst);
				_pFirst = pTMP;
			}

			_pFirst = nullptr;
			_pLast  = nullptr;
			_nElementsCount = 0;
		}
		~List()
		{
			Clear();
		}
	private:

		CL::RefPtr<Pool<ListNode>> _pAllocator;
		ListNode* _pFirst;
		ListNode* _pLast;
		size_t _nElementsCount;
	};
}