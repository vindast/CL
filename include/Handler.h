#pragma once
#include "list.h"
 
namespace CL
{
	template<class ObjType> class Handle;
	template<class ObjType> class Handler;

	template<class ObjType> class HandleBase
	{
		friend class Handler<ObjType>;
	protected:
		Handler<ObjType>* _pParrent = nullptr;

		ListIterator<HandleBase<ObjType>*> _handleIt;
	};

	template<class ObjType> class Handler
	{
		friend class Handle<ObjType>;
	public:

		Handler(ObjType obj) :
			_obj(obj)
		{

		}

		const ObjType getObj() const
		{
			return _obj;
		}

		ObjType getObj()
		{
			return _obj;
		}

		~Handler()
		{
			for (auto pHandle : _subscribers)
			{
				pHandle->_pParrent = nullptr;
			}
		}

	private:
		ObjType _obj;

		CL::List<HandleBase<ObjType>*> _subscribers;

		Handler<ObjType>(const Handler<ObjType>&) = delete;
		Handler<ObjType> operator = (const Handler<ObjType>&) = delete;

	};


	template<class ObjType> class Handle : public HandleBase<ObjType>
	{
	public:

		Handle()
		{

		}

		bool isValid() const
		{
			return _pParrent;
		}

		Handle<ObjType>& operator << (Handler<ObjType>& handler)
		{
			free();

			_pParrent = &handler;
			_pParrent->_subscribers.push_front(this);
			_handleIt = _pParrent->_subscribers.begin();

			return *this;
		}

		Handle<ObjType>& operator = (Handler<ObjType>& handler)
		{
			free();

			_pParrent = &handler;
			_pParrent->_subscribers.push_front(this);
			_handleIt = _pParrent->_subscribers.begin();

			return *this;
		}

		void free()
		{
			if (_pParrent)
			{
				_pParrent->_subscribers.erase(_handleIt);
			}

			_pParrent = nullptr;
		}

		~Handle()
		{
			free();
		}

		ObjType getObj()
		{
			return _pParrent->getObj();
		}

		 

		Handle<ObjType>& operator = (Handle<ObjType>& handle)
		{
			free();

			if (handle._pParrent)
			{
				_pParrent = handle._pParrent;
				_pParrent->_subscribers.push_front(this);
				_handleIt = _pParrent->_subscribers.begin();
			}
			
			return *this;
		}
		 
		Handle<ObjType>(Handle<ObjType>& handle)
		{
			*this = handle;
		}
		 
	private:


		//Handle<ObjType>(const Handle<ObjType>&) = delete;
		//Handle<ObjType>& operator = (const Handle<ObjType>&) = delete;
	};


};
