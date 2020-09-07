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

		virtual DelegateBase* createCopy() const override
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

		void call() override
		{
			(_pListener->*_pMethod)();
		}

		virtual DelegateBase* createCopy() const override
		{
			return new Delegat<EventListener>(_pListener, _pMethod);
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

			return *this;
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

	template<> class AbstractDelegat<> final
	{
	public:

		AbstractDelegat(const DelegateBase<>& delegat)
		{
			std::cout << "AbstractDelegat(const DelegateBase<>& delegat)" << std::endl;
			_pDelegat = delegat.createCopy();
		}

		AbstractDelegat(const AbstractDelegat<>& delegat)
		{
			std::cout << "AbstractDelegat(const AbstractDelegat<>& delegat)" << std::endl;
			_pDelegat = delegat._pDelegat->createCopy();
		}

		AbstractDelegat& operator = (const AbstractDelegat<>& delegat)
		{
			std::cout << "AbstractDelegat& operator = (const AbstractDelegat<>& delegat)" << std::endl;

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

		~AbstractDelegat()
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