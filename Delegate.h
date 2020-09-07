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

	template <> class DelegateBase<> 
	{
	public:

		virtual void call() = 0;

		virtual DelegateBase* createCopy() const = 0;

	private:
	};

	template<class EventListener, class... Arguments> class Delegate final : public DelegateBase<Arguments...>
	{
		typedef void (EventListener::* Metod)(Arguments...);
	public:

		Delegate(EventListener* pListener, Metod pMethod) :
			_pListener(pListener), _pMethod(pMethod)
		{

		}

		void call(Arguments... argumetns) override
		{
			(_pListener->*_pMethod)(argumetns...);
		}

		DelegateBase* createCopy() const override
		{
			return new Delegate<EventListener, Arguments...>(_pListener, _pMethod);
		}

	private:
		EventListener* _pListener;
		Metod _pMethod;
	};  

	template<class EventListener> class Delegate<EventListener, void>  final
	{
		typedef void (EventListener::* Metod)();
	public:

		Delegate(EventListener* pListener, Metod pMethod) :
			_pListener(pListener), _pMethod(pMethod)
		{

		}

		void call() override
		{
			(_pListener->*_pMethod)();
		}

		DelegateBase* createCopy() const override
		{
			return new Delegate<EventListener>(_pListener, _pMethod);
		}

	private:
		EventListener* _pListener;
		Metod _pMethod;
	};

	template<class... Arguments> class AbstracteDelegate final
	{
	public:

		AbstracteDelegate(const DelegateBase<Arguments...>& delegat)
		{
			_pDelegat = delegat.createCopy();
		}

		AbstracteDelegate(const AbstracteDelegate<Arguments...>& delegat)
		{
			_pDelegat = delegat._pDelegat->createCopy();
		}

		AbstracteDelegate<Arguments...>& operator = (const AbstracteDelegate<Arguments...>& delegat)
		{
			if (_pDelegat)
			{
				delete _pDelegat;
			}

			_pDelegat = delegat._pDelegat->createCopy();

			return *this;
		}

		void call(Arguments... argumetns) 
		{
			_pDelegat->call(argumetns...);
		}

		~AbstracteDelegate()
		{
			if (_pDelegat)
			{
				delete _pDelegat;
			}
		}

	private:
		DelegateBase<Arguments...>* _pDelegat;
	};

	template<> class AbstracteDelegate<> final
	{
	public:

		AbstracteDelegate(const DelegateBase<>& delegat)
		{ 
			_pDelegat = delegat.createCopy();
		}

		AbstracteDelegate(const AbstracteDelegate<>& delegat)
		{ 
			_pDelegat = delegat._pDelegat->createCopy();
		}

		AbstracteDelegate& operator = (const AbstracteDelegate<>& delegat)
		{ 

			if (_pDelegat)
			{
				delete _pDelegat;
			}

			_pDelegat = delegat._pDelegat->createCopy();

			return *this;
		}

		void call()
		{
			_pDelegat->call();
		}

		~AbstracteDelegate()
		{
			if (_pDelegat)
			{
				delete _pDelegat;
			}
		}

	private:
		DelegateBase<>* _pDelegat;
	};
};