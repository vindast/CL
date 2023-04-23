#pragma once
#include <assert.h>
#include "CLObjects/CLCriticalSection.h"
#include "CLMemory.h"

namespace CL
{
	template<class listItem>class List; 

	template<class listItem>struct ListNode
	{
		ListNode()
		{

		}
		ListNode(listItem item)
		{
			this->item = item;
		}

		listItem item;
		ListNode<listItem>* previos = nullptr;
		ListNode<listItem>* next    = nullptr;
	};

	template<class listItem> class ListIterator
	{
		friend class List<listItem>;
	public:
		ListIterator()
		{

		}
		ListIterator(const ListIterator<listItem>& iterator)
		{
			*this = iterator;
		}
		ListIterator<listItem>& operator = (const ListIterator<listItem>& iterator)
		{
			_currentNode = iterator._currentNode;
			_owner = iterator._owner;
			return *this;
		}	 
		ListIterator<listItem>& operator++(int)
		{
			assert(_currentNode);
			 
			_currentNode = _currentNode->next;

			return *this;
		}
		ListIterator<listItem>& operator--(int)
		{
			assert(_currentNode);

			_currentNode = _currentNode->previos;

			return *this;
		}
		const ListNode<listItem>* current_node() const
		{
			return _currentNode;
		}
		listItem& operator ()()   
		{
			return _currentNode->item;
		}
		const listItem& operator ()() const
		{
			return _currentNode->item;
		}
		const List<listItem>* getOwner() const
		{
			return _owner;
		}
		friend ListIterator<listItem>& operator++(ListIterator<listItem>& it) 
		{
			it._currentNode = it._currentNode->next;

			return it;
		}
		friend listItem& operator*(ListIterator<listItem>& it)
		{
			return it._currentNode->item;
		}
	private:
		List<listItem>* _owner = nullptr;
		ListNode<listItem>* _currentNode = nullptr;
	};
 
	template<class listItem> bool operator != (const ListIterator<listItem>& iterator1, const ListIterator<listItem>& iterator2)
	{
		return iterator1.current_node() != iterator2.current_node();
	}

	template<class listItem>class ListAtomicIterator
	{
		friend class List<listItem>;
	public:
		ListAtomicIterator()
		{

		} 
		ListAtomicIterator(const ListAtomicIterator<listItem>& iterator)
		{
			*this = iterator;
		} 
		ListAtomicIterator<listItem>& operator = (const ListAtomicIterator<listItem>& iterator)
		{
			_owner = iterator._owner;
			_currentNode = iterator._currentNode;
			return *this;
		} 
		bool getNext(listItem& item)
		{
			LockGuard<CriticalSection> lock(_hCS);

			if (_currentNode)
			{
				item = _currentNode->item;

				_currentNode = _currentNode->next;

				return true;
			}
			else
			{
				return false;
			}
		} 
		bool getNext(listItem*& item)
		{
			LockGuard<CriticalSection> lock(_hCS);

			if (_currentNode)
			{
				item = &_currentNode->item;

				_currentNode = _currentNode->next;

				return true;
			}
			else
			{
				return false;
			}
		} 
	private:
		CriticalSection _hCS;
		List<listItem>* _owner = nullptr;
		ListNode<listItem>* _currentNode = nullptr;
	};

	

	template<class listItem>class List
	{
	public:
		List()
		{

		}

		List(const List<listItem>& list)
		{
			*this = list;
		}

		List<listItem>& operator = (const List<listItem>& list)
		{
			clear();

			ListNode<listItem>* node = list._first;

			while (node)
			{
				push_back(node->item);
				node = node->next;
			}

			return *this;
		}

		void push_front(listItem item)
		{
			ListNode<listItem>* node = CL_NEW(ListNode<listItem>);
			node->item = item;

			if (_first == nullptr)
			{
				_first = node;
				_last  = node;
			}
			else
			{ 
				_first->previos = node;
				node->next = _first;
				_first = node;
			}

			_size++;
		}

		void push_back(listItem item)
		{
			ListNode<listItem>* node = CL_NEW( ListNode<listItem>);
			node->item = item;

			if (_first == nullptr)
			{
				_first = node;
				_last = node;
			}
			else
			{
				//_last->previos = _last;
				auto tmpLast = _last;

				_last->next = node;
				_last = node;
				_last->previos = tmpLast;
			}

			_size++;
		}

		ListIterator<listItem> insert(ListIterator<listItem> iterator, listItem item)
		{
			assert(iterator._owner == this);
			assert(iterator._currentNode);

			ListNode<listItem>* node = CL_NEW( ListNode<listItem>, item);

			_size++;
			 
			if (iterator._currentNode == _last)
			{
				_last = node;

				node->previos = iterator._currentNode;

				iterator._currentNode->next = node;
			}
			else
			{
				node->previos = iterator._currentNode;
				node->next    = iterator._currentNode->next;

				iterator._currentNode->next->previos = node;
				iterator._currentNode->next = node; 
			}

			ListIterator<listItem> newIterator;
			newIterator._owner       = this;
			newIterator._currentNode = node;

			return newIterator;
		}

		ListAtomicIterator<listItem>  beginAtomic()
		{
			ListAtomicIterator<listItem> it;
			it._currentNode = _first;
			it._owner = this;

			return it;
		}

		ListIterator<listItem> begin()
		{
			ListIterator<listItem> iterator;
			iterator._currentNode = _first;
			iterator._owner = this;
			return iterator;
		}

		ListIterator<listItem> rbegin()
		{
			ListIterator<listItem> iterator;
			iterator._currentNode = _last;
			iterator._owner = this;
			return iterator;
		}
		 
		ListIterator<listItem> end()
		{
			ListIterator<listItem> iterator;
			iterator._currentNode = nullptr;
			iterator._owner = this;
			return iterator;
		}

		ListIterator<listItem> erase(ListIterator<listItem>& iterator)
		{ 
			assert(iterator._owner == this);
			assert(iterator._currentNode); 
			assert(_size);
		

			//std::cout << "erase start" << std::endl;

			ListNode<listItem>* node = iterator._currentNode;

			iterator._owner = nullptr;
			iterator._currentNode = nullptr;
			 
			if (node != _first && node != _last)
			{ 
				node->previos->next = node->next;
				node->next->previos = node->previos;
				 
				//std::cout << "node != _first && node != _last" << std::endl;
			}
			else
			{
				//std::cout << "this case (node != _first :" << (node != _first) <<", node != _last :" << (node != _last) << ")"<< std::endl;

				if (node == _first)
				{
				//	std::cout << "node == _first" << std::endl;

					_first = node->next;
					
					if (_first)
					{
						_first->previos = nullptr;
					} 
				}

				if (node == _last)
				{
				//	std::cout << "node == _last" << std::endl;

					_last = node->previos;
					
 
					if (_last)
					{
						_last->next = nullptr;
					}

				}
			}
			

			ListIterator<listItem> outIterator;
			outIterator._owner       = this;
			outIterator._currentNode = node->next;

			
			_size--;

			CL_DELETE( node);

			//std::cout << "erase end" << std::endl;

			return outIterator;
		}

		size_t size() const
		{
			return _size;
		}

		~List()
		{
			clear();
		}

		void clear()
		{
			if (_first)
			{
				assert(_size);
				ListNode<listItem>* node = _first;

				while (node)
				{
					_first = node;
					node = node->next;

					CL_DELETE( _first);
				}

				_first = nullptr;
				_last  = nullptr;

				_size = 0;
			}
		}

		const ListNode<listItem>* getLastItem() const
		{
			return _last;
		}

		ListNode<listItem>* getLastItem()
		{
			return _last;
		}

		const ListNode<listItem>* getFirstItem() const
		{
			return _first;
		}

		ListNode<listItem>* getFirstItem()
		{
			return _first;
		}

	private:
		size_t _size = 0;
		ListNode<listItem>* _first = nullptr;
		ListNode<listItem>* _last  = nullptr;
	};

};