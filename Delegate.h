#pragma once
#include "UniquePtr.h"

namespace CL
{ 
	template <class... Arguments> class DelegateBase abstract
	{
	public:

		virtual void call(Arguments... arguments) = 0;

		virtual DelegateBase* createCopy() const = 0;

		virtual ~DelegateBase()
		{

		}

	private:
	};

	template<class EventListener, class... Arguments> class Delegate final : public DelegateBase<Arguments...>
	{
		typedef void (EventListener::* Method)(Arguments...);
	public:

		Delegate(EventListener* pListener, Method pMethod) :
			_pListener(pListener), _pMethod(pMethod)
		{

		}

		void call(Arguments... arguments) override
		{
			(_pListener->*_pMethod)(arguments...);
		}

		DelegateBase* createCopy() const override
		{
			return new Delegate<EventListener, Arguments...>(_pListener, _pMethod);
		}

	private:
		EventListener* _pListener;
		Method _pMethod;
	};   

	template<class... Arguments> class AbstractDelegate final
	{
	public:

		AbstractDelegate(const DelegateBase<Arguments...>& delegat)
		{
			_upDelegate = delegat.createCopy();
		}

		AbstractDelegate(const AbstractDelegate<Arguments...>& delegat)
		{
			_upDelegate = delegat._upDelegate->createCopy();
		}

		AbstractDelegate<Arguments...>& operator = (const AbstractDelegate<Arguments...>& delegat)
		{  
			_upDelegate = delegat._upDelegate->createCopy();

			return *this;
		}

		void call(Arguments... arguments) 
		{
			_upDelegate->call(arguments...);
		}
		 

	private:
		UniquePtr<DelegateBase<Arguments...>> _upDelegate;
	};

};