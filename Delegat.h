#pragma once

namespace CL
{ 
	template <class... Arguments> class DelegateBase abstract
	{
	public:

		virtual void call(Arguments... argumetns) = 0;

		virtual DelegateBase* createCopy() const = 0;

	private:
	};

	template<class EventListener, class... Arguments> class Delegat final : public DelegateBase<Arguments...>
	{
		typedef void (EventListener::* Metod)(Arguments...);
	public:

		Delegat(EventListener* pListener, Metod pMethod) :
			_pListener(pListener), _pMethod(pMethod)
		{

		}

		void call(Arguments... argumetns) override
		{
			(_pListener->*_pMethod)(argumetns...);
		}

		virtual DelegateBase* createCopy() const
		{
			return new Delegat<EventListener, Arguments...>(_pListener, _pMethod);
		}

	private:
		EventListener* _pListener;
		Metod _pMethod;
	};  

	template<class EventListener> class Delegat<EventListener, void>  final
	{
		typedef void (EventListener::* Metod)();
	public:

		Delegat(EventListener* pListener, Metod pMethod) :
			_pListener(pListener), _pMethod(pMethod)
		{

		}

		void call() 
		{
			(_pListener->*_pMethod)();
		}

		virtual DelegateBase* createCopy() const
		{
			return new Delegat<EventListener, void>(_pListener, _pMethod);
		}

	private:
		EventListener* _pListener;
		Metod _pMethod;
	};

	template<class... Arguments> class AbstractDelegat final
	{
	public:

		AbstractDelegat(const DelegateBase<Arguments...>& delegat)
		{
			_pDelegat = delegat.createCopy();
		}

		AbstractDelegat(const AbstractDelegat<Arguments...>& delegat)
		{
			_pDelegat = delegat._pDelegat->createCopy();
		}

		AbstractDelegat<Arguments...>& operator = (const AbstractDelegat<Arguments...>& delegat)
		{
			if (_pDelegat)
			{
				delete _pDelegat;
			}

			_pDelegat = delegat._pDelegat->createCopy();
		}

		void call(Arguments... argumetns) 
		{
			_pDelegat->call(argumetns...);
		}

		~AbstractDelegat()
		{
			if (_pDelegat)
			{
				delete _pDelegat;
			}
		}

	private:
		DelegateBase<Arguments...>* _pDelegat;
	};
};