#pragma once 
#include "ObjectPool.h"

namespace CL
{


	template<class listItem, size_t nAlocatorBaseSize = 128>class ListPoolled;

	template<class listItem>struct ListPoolledNode
	{
		ListPoolledNode()
		{

		}

		ListPoolledNode(listItem item)
		{
			this->item = item;
		}

		ListPoolledNode<listItem>& operator = (const ListPoolledNode<listItem>& node)
		{
			item = node.item;

			return*this;
		}

		listItem item;
		CL::ObjectPoolNode<ListPoolledNode<listItem>>* previos = nullptr;
		CL::ObjectPoolNode<ListPoolledNode<listItem>>* next = nullptr;
	};
	 
	template<class listItem, size_t nAlocatorBaseSize> class ListPoolledIterator
	{
		friend class ListPoolled<listItem, nAlocatorBaseSize>;
	public:
		ListPoolledIterator()
		{

		}

		ListPoolledIterator(const ListPoolledIterator<listItem, nAlocatorBaseSize>& iterator)
		{
			*this = iterator;
		}

		ListPoolledIterator<listItem, nAlocatorBaseSize>& operator = (const ListPoolledIterator<listItem, nAlocatorBaseSize>& iterator)
		{
			_currentNode = iterator._currentNode;
			_owner = iterator._owner;
			return *this;
		}

		ListPoolledIterator<listItem, nAlocatorBaseSize>& operator++(int)
		{
			assert(_currentNode);

			_currentNode = _currentNode->data.next;

			return *this;
		}
		 
		ListPoolledIterator<listItem, nAlocatorBaseSize>& operator--(int)
		{
			assert(_currentNode);

			_currentNode = _currentNode->data.previos;

			return *this;
		}
		 
		const CL::ObjectPoolNode<ListPoolledNode<listItem>>* current_node() const
		{
			return _currentNode;
		}

		listItem& operator ()()
		{
			return _currentNode->data.item;
		}

		const listItem& operator ()() const
		{
			return _currentNode->item;
		}

		const ListPoolled<listItem, nAlocatorBaseSize>* getOwner() const
		{
			return _owner;
		}

		friend ListPoolledIterator<listItem, nAlocatorBaseSize>& operator++(ListPoolledIterator<listItem, nAlocatorBaseSize>& it)
		{
			it._currentNode = it._currentNode->data.next;

			return it;
		}

		friend listItem& operator*(ListPoolledIterator<listItem, nAlocatorBaseSize>& it)
		{
			return it._currentNode->data.item;
		}

	private:
		ListPoolled<listItem, nAlocatorBaseSize>* _owner = nullptr;
		CL::ObjectPoolNode<ListPoolledNode<listItem>>* _currentNode = nullptr;
	};

	template<class listItem, size_t nAlocatorBaseSize> bool operator != (const ListPoolledIterator<listItem, nAlocatorBaseSize>& iterator1, const ListPoolledIterator<listItem, nAlocatorBaseSize>& iterator2)
	{
		return iterator1.current_node() != iterator2.current_node();
	}

 
	template<class listItem, size_t nAlocatorBaseSize>class ListPoolled
	{
	public:
		ListPoolled(ObjectPoolDynamic<ListPoolledNode<listItem>, nAlocatorBaseSize>* pAlocator, bool bAsync):
			_pAlocator(pAlocator), _bAsync(bAsync)
		{

		}

		ListPoolled(const ListPoolled<listItem, nAlocatorBaseSize>& list)
		{
			*this = list;
		}

		ListPoolled<listItem, nAlocatorBaseSize>& operator = (const ListPoolled<listItem, nAlocatorBaseSize>& list)
		{
			clear();

			_pAlocator = list._pAlocator;
			_bAsync = list._bAsync;

			auto node = list._first;

			while (node)
			{
				push_back(node->data.item);
				node = node->data.next;
			}

			return *this;
		}

		void push_front(listItem item)
		{
			CL::ObjectPoolNode<ListPoolledNode<listItem>>* node = nullptr;

			if (_bAsync)
			{
				node = _pAlocator->allocAsync();
			}
			else
			{
				node = _pAlocator->alloc();
			}


			node->data.item = item;

			if (_first == nullptr)
			{
				_first = node;
				_last = node;
			}
			else
			{
				_first->previos = node;
				node->data.next = _first;
				_first = node;
			}

			_size++;
		}

		void push_back(listItem item)
		{
			CL::ObjectPoolNode<ListPoolledNode<listItem>>* node = nullptr;

			if (_bAsync)
			{
				node = _pAlocator->allocAsync();
			}
			else
			{
				node = _pAlocator->alloc();
			}

			node->data.item = item;

			if (_first == nullptr)
			{
				_first = node;
				_last = node;
			}
			else
			{ 
				auto tmpLast = _last;

				_last->data.next = node;
				_last = node;
				_last->data.previos = tmpLast;
			}

			_size++;
		}

		ListPoolledIterator<listItem, nAlocatorBaseSize> insert(ListPoolledIterator<listItem, nAlocatorBaseSize> iterator, listItem item)
		{
			assert(iterator._owner == this);
			assert(iterator._currentNode);

			CL::ObjectPoolNode<ListPoolledNode<listItem>>* node = nullptr;

			if (_bAsync)
			{
				node = _pAlocator->allocAsync();
			}
			else
			{
				node = _pAlocator->alloc();
			}

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
				node->next = iterator._currentNode->next;

				iterator._currentNode->next->previos = node;
				iterator._currentNode->next = node;
			}

			ListPoolledIterator<listItem, nAlocatorBaseSize> newIterator;
			newIterator._owner = this;
			newIterator._currentNode = node;

			return newIterator;
		}

		/*ListAtomicIterator<listItem>  beginAtomic()
		{
			ListAtomicIterator<listItem> it;
			it._currentNode = _first;
			it._owner = this;

			return it;
		}*/

		ListPoolledIterator<listItem, nAlocatorBaseSize> begin()
		{
			ListPoolledIterator<listItem, nAlocatorBaseSize> iterator;
			iterator._currentNode = _first;
			iterator._owner = this;
			return iterator;
		}

		ListPoolledIterator<listItem, nAlocatorBaseSize> rbegin()
		{
			ListPoolledIterator<listItem, nAlocatorBaseSize> iterator;
			iterator._currentNode = _last;
			iterator._owner = this;
			return iterator;
		}

		ListPoolledIterator<listItem, nAlocatorBaseSize> end()
		{
			ListPoolledIterator<listItem, nAlocatorBaseSize> iterator;
			iterator._currentNode = nullptr;
			iterator._owner = this;
			return iterator;
		}

		ListPoolledIterator<listItem, nAlocatorBaseSize> erase(ListPoolledIterator<listItem, nAlocatorBaseSize>& iterator)
		{
			assert(iterator._owner == this);
			assert(iterator._currentNode);
			assert(_size);

		//	std::cout << "size =" << _size << ", iterator._owner == this = " << (iterator._owner == this) << ", iterator._currentNode = " << iterator._currentNode << std::endl;


			//std::cout << "erase start" << std::endl;

			auto node = iterator._currentNode;

			iterator._owner = nullptr;
			iterator._currentNode = nullptr;

			if (node != _first && node != _last)
			{
				node->data.previos->data.next = node->data.next;
				node->data.next->data.previos = node->data.previos;

				//std::cout << "node != _first && node != _last" << std::endl;
			}
			else
			{
				//std::cout << "this case (node != _first :" << (node != _first) <<", node != _last :" << (node != _last) << ")"<< std::endl;

				if (node == _first)
				{
					//	std::cout << "node == _first" << std::endl;

					_first = node->data.next;

					if (_first)
					{
						_first->data.previos = nullptr;
					}
				}

				if (node == _last)
				{
					//	std::cout << "node == _last" << std::endl;

					_last = node->data.previos;


					if (_last)
					{
						_last->data.next = nullptr;
					}

				}
			}


			ListPoolledIterator<listItem, nAlocatorBaseSize> outIterator;
			outIterator._owner = this;
			outIterator._currentNode = node->data.next;


			_size--;


			

			if (_bAsync)
			{
				_pAlocator->freeAsync(node);
			}
			else
			{
				_pAlocator->free(node);
			}


			//std::cout << "erase end" << std::endl;

			return outIterator;
		}

		size_t size() const
		{
			return _size;
		}

		~ListPoolled()
		{
			clear();
		}

		void clear()
		{
			if (_first)
			{
				assert(_size);
				auto node = _first;

				while (node)
				{
					_first = node;
					node = node->data.next;

					if (_bAsync)
					{
						_pAlocator->freeAsync(_first);
					}
					else
					{
						_pAlocator->free(_first);
					}
				}

				_first = nullptr;
				_last = nullptr;

				_size = 0;
			}
		}

		const auto getLastItem() const
		{
			return _last;
		}

		auto getLastItem()
		{
			return _last;
		}

		auto getFirstItem() const
		{
			return _first;
		}

		auto getFirstItem()
		{
			return _first;
		}

	private:
		size_t _size = 0;
		CL::ObjectPoolNode<ListPoolledNode<listItem>>* _first = nullptr;
		CL::ObjectPoolNode<ListPoolledNode<listItem>>* _last = nullptr;

		ObjectPoolDynamic<ListPoolledNode<listItem>, nAlocatorBaseSize>* _pAlocator;
		bool _bAsync;
	};
};