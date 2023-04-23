#pragma once
#include "EventHolder.h"

namespace CL
{   
//------------------------------------------предварительное обьявление--------------------------------------
	template<class... Arguments> class EventHandlerBase;
	template<class... Arguments> class Event;
//----------------------------------------------------------------------------------------------------------
	template<class EventSender, class... Arguments> class EventBase abstract
	{
		friend class EventHandlerBase<EventSender, Arguments>;
	public:
		
	protected:
		
		virtual void unregHandle(CL::ListIterator<EventHandlerBase<EventSender, Arguments...>*> it) = 0;
	};

	template<class... Arguments> class EventHandlerBase abstract
	{
		friend class Event<Arguments...>;
	public:

		virtual void call(Arguments... argumetns) = 0;

		virtual ~EventHandlerBase()
		{
			unregister();
		}
		 
		void unregister()
		{
			if (_pEvent)
			{
				_pEvent->unregHandle(it);
				_pEvent = nullptr;
			}
		}

	private:
		CL::ListIterator<EventHandlerBase<Arguments...>*> it;
		EventBase<Arguments...>* _pEvent = nullptr;

	};

	template<class EventListener, class... Arguments> class EventHandler final : public EventHandlerBase<Arguments...>
	{
		typedef void (EventListener::*Method)(Arguments...);
	public:

		EventHandler(EventListener* pListener, Method pMethod) :
			_pListener(pListener), _pMethod(pMethod)
		{

		}

		void call(Arguments... argumetns) override
		{
			(_pListener->*_pMethod)(argumetns...);
		}

	private:
		EventListener* _pListener;
		Method _pMethod;
	};

	template<class... Arguments> class Event final : public EventBase<Arguments...>
	{
	public:
		Event()
		{

		}

		void registerListener(EventHandlerBase<Arguments...>* pHandle)
		{
			pHandle->unregister();
			pHandle->_pEvent = this;
			_pHandlers.push_back(pHandle);
			pHandle->it = _pHandlers.rbegin();
		}

		void call(Arguments... argemetns)
		{
			for (auto p : _pHandlers)
			{
				p->call(argemetns...);
			}
		} 
 
		~Event()
		{
			for (auto pHandle : _pHandlers)
			{
				pHandle->_pEvent = nullptr;
			}
		}


	private:
		CL::List<EventHandlerBase<Arguments...>*> _pHandlers;

		void unregHandle(CL::ListIterator<EventHandlerBase<Arguments...>*> it) override
		{
			_pHandlers.erase(it);
		}
	};  
};