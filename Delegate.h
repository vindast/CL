#pragma once

namespace CL
{ 
	template <class... Arguments> class DelegateBase abstract
	{
	public:

		virtual void call(Arguments... arguments) = 0;

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

	/*template<class EventListener> class Delegate<EventListener, void>  final
	{
		typedef void (EventListener::* Method)();
	public:

		Delegate(EventListener* pListener, Method pMethod) :
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
		Method _pMethod;
	};*/

	template<class... Arguments> class AbstractDelegate final
	{
	public:

		AbstractDelegate(const DelegateBase<Arguments...>& delegat)
		{
			_pDelegate = delegat.createCopy();
		}

		AbstractDelegate(const AbstractDelegate<Arguments...>& delegat)
		{
			_pDelegate = delegat._pDelegate->createCopy();
		}

		AbstractDelegate<Arguments...>& operator = (const AbstractDelegate<Arguments...>& delegat)
		{
			delete _pDelegate;

			_pDelegate = delegat._pDelegate->createCopy();

			return *this;
		}

		void call(Arguments... arguments) 
		{
			_pDelegate->call(arguments...);
		}

		~AbstractDelegate()
		{
			delete _pDelegate;
		}

	private:
		DelegateBase<Arguments...>* _pDelegate;
	};

	/*template<> class AbstractDelegate<> final
	{
	public:

		AbstractDelegate(const DelegateBase<>& delegat)
		{ 
			_pDelegate = delegat.createCopy();
		}

		AbstractDelegate(const AbstractDelegate<>& delegat)
		{ 
			_pDelegate = delegat._pDelegate->createCopy();
		}

		AbstractDelegate& operator = (const AbstractDelegate<>& delegat)
		{  
			delete _pDelegate;

			_pDelegate = delegat._pDelegate->createCopy();

			return *this;
		}

		void call()
		{
			_pDelegate->call();
		}

		~AbstractDelegate()
		{
			delete _pDelegate;
		}

	private:
		DelegateBase<>* _pDelegate;
	};*/
};