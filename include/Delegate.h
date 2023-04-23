#pragma once
#include "CLTypeId.h"
#include "UniquePtr.h" 
#include "CLCommon.h"

namespace CL
{  
	template <class ReturnValueType, class... Arguments> class DelegateBase abstract : public virtual TypeId
	{ 
		typedef DelegateBase<ReturnValueType, Arguments...> TDelegateBase;
		TDelegateBase& operator = (const TDelegateBase&) = delete;
		TDelegateBase(const TDelegateBase&) = delete;
	public:
		TDelegateBase() = default;
		virtual ReturnValueType call(Arguments... arguments) = 0;
		virtual TDelegateBase* createCopy() const = 0;
		virtual bool IsEqual(const TDelegateBase* pDelegate) const = 0;
		virtual ~DelegateBase() = default;
	private:
	};

	template<class ClassName, class ReturnValueType, class... Arguments> 
	class Delegate final : 
		public DelegateBase<ReturnValueType, Arguments...>,
		public TypeIdWrapper<Delegate<ClassName, ReturnValueType, Arguments...>>
	{
		typedef DelegateBase<ReturnValueType, Arguments...> TDelegateBase;
		typedef Delegate<ClassName, ReturnValueType, Arguments...> TDelegate;
		typedef ReturnValueType(ClassName::* Method)(Arguments...);
	public:
		TDelegate(ClassName* pListener, Method pMethod) :
			_pListener(pListener), _pMethod(pMethod)
		{

		}
		TDelegate(const TDelegate& otherDelegate):
			_pListener(otherDelegate._pListener), _pMethod(otherDelegate._pMethod)
		{

		}
		TDelegate(TDelegate&& otherDelegate) :
			_pListener(otherDelegate._pListener), _pMethod(otherDelegate._pMethod)
		{
			otherDelegate._pListener = nullptr;
			otherDelegate._pMethod = nullptr;
		} 
		TDelegate& operator = (TDelegate&& otherDelegate)
		{
			_pListener = otherDelegate._pListener;
			_pMethod = otherDelegate._pMethod;

			otherDelegate._pListener = nullptr;
			otherDelegate._pMethod = nullptr;

			return *this;
		}
		TDelegate& operator = (const TDelegate& otherDelegate)
		{
			_pListener = otherDelegate._pListener;
			_pMethod = otherDelegate._pMethod; 

			return *this;
		}
		ReturnValueType call(Arguments... arguments) override
		{
			return (_pListener->*_pMethod)(arguments...);
		}
		TDelegate* createCopy() const override
		{
			return CL_NEW( TDelegate, _pListener, _pMethod);
		}
		bool IsEqual(ClassName* pListener, Method pMethod) const
		{
			return _pListener == pListener && pMethod == _pMethod;
		}
		bool IsEqual(const TDelegateBase* pDelegate) const override
		{
			const TDelegate* pObj = TypeId::DynamicCast<TDelegate>(pDelegate);
			return pObj && pObj->_pListener == _pListener && pObj->_pMethod == _pMethod;
		}
	private:
		ClassName* _pListener;
		Method _pMethod;
	};   

	template<class ClassName, class ReturnValueType, class... Arguments> 
	class ConstDelegate final : 
		public DelegateBase<ReturnValueType, Arguments...>,
		public TypeIdWrapper<ConstDelegate<ClassName, ReturnValueType, Arguments...>>
	{
		typedef DelegateBase<ReturnValueType, Arguments...> TDelegateBase;
		typedef ConstDelegate<ClassName, ReturnValueType, Arguments...> TDelegate;
		typedef ReturnValueType(ClassName::* Method)(Arguments...) const;
	public:
		TDelegate(const ClassName* pListener, Method pMethod) :
			_pListener(pListener), _pMethod(pMethod)
		{

		} 
		TDelegate(const TDelegate& otherDelegate) :
			_pListener(otherDelegate._pListener), _pMethod(otherDelegate._pMethod)
		{

		}
		TDelegate(TDelegate&& otherDelegate) :
			_pListener(otherDelegate._pListener), _pMethod(otherDelegate._pMethod)
		{
			otherDelegate._pListener = nullptr;
			otherDelegate._pMethod   = nullptr;
		}
		TDelegate& operator = (TDelegate&& otherDelegate)
		{
			_pListener = otherDelegate._pListener;
			_pMethod = otherDelegate._pMethod;

			otherDelegate._pListener = nullptr;
			otherDelegate._pMethod = nullptr;

			return *this;
		}
		TDelegate& operator = (const TDelegate& otherDelegate)
		{
			_pListener = otherDelegate._pListener;
			_pMethod = otherDelegate._pMethod;

			return *this;
		}
		ReturnValueType call(Arguments... arguments) override
		{
			return (_pListener->*_pMethod)(arguments...);
		}
		TDelegate* createCopy() const override
		{
			return CL_NEW( TDelegate, _pListener, _pMethod);
		}
		bool IsEqual(const ClassName* pListener, Method pMethod) const
		{
			return _pListener == pListener && pMethod == _pMethod;
		}
		bool IsEqual(const TDelegateBase* pDelegate) const override
		{
			const TDelegate* pObj = TypeId::DynamicCast<TDelegate>(pDelegate);
			return pObj && pObj->_pListener == _pListener && pObj->_pMethod == _pMethod;
		}
	private:
		const ClassName* _pListener;
		Method _pMethod;
	};

	template<class ReturnValueType, class... Arguments> 
	class FunctionPtr final :
		public DelegateBase<ReturnValueType, Arguments...>,
		public TypeIdWrapper<FunctionPtr<ReturnValueType, Arguments...>>
	{
		typedef FunctionPtr<ReturnValueType, Arguments...> TFunctionPtr;
		typedef DelegateBase<ReturnValueType, Arguments...> TDelegateBase;
		typedef ReturnValueType(*_function_ptr_)(Arguments...);
	public:  
		TFunctionPtr(_function_ptr_ pFunction) :
			_pFunction(pFunction)
		{

		}
		ReturnValueType call(Arguments... arguments) override
		{ 
			return _pFunction(arguments...);
		}
		TFunctionPtr* createCopy() const override
		{
			return CL_NEW( TFunctionPtr, _pFunction);
		}
		bool IsEqual(_function_ptr_ pFucntion) const
		{
			return _pFunction == pFucntion;
		}
		bool IsEqual(const TDelegateBase* pDelegate) const override
		{
			const TFunctionPtr* pObj = TypeId::DynamicCast<TFunctionPtr>(pDelegate);
			return pObj && pObj->_pFunction == _pFunction;
		}
	private:
		_function_ptr_ _pFunction;
	};  

	template<
		class ReturnValueType,
		typename LambdaType, 
		class... Arguments 
	> 
	class LambdaPtr final : 
		public DelegateBase<ReturnValueType, Arguments...>,
		public TypeIdWrapper<LambdaPtr<ReturnValueType, LambdaType, Arguments...>>
	{
		typedef DelegateBase<ReturnValueType, Arguments...> TDelegateBase;
	public: 
		typedef LambdaPtr<ReturnValueType, LambdaType, Arguments...> LambdaT;
		template<
			typename = std::enable_if_t<
			!std::is_base_of<TDelegateBase, std::remove_reference<LambdaType>::type>::value 
			&& !std::is_same<TDelegateBase, std::remove_reference<LambdaType>::type>::value 
			&& std::is_object<LambdaType>::value
			&& !std::is_function<LambdaType>::value
			&& std::is_invocable<LambdaType, Arguments...>::value
			>
		>
		LambdaT(LambdaType pFunction) :
			_pFunction(pFunction)
		{

		}  
		ReturnValueType call(Arguments... arguments) override
		{
			return _pFunction(arguments...);
		} 
		LambdaT* createCopy() const override
		{
			return CL_NEW( LambdaT, _pFunction);
		}
		bool IsEqual(LambdaType pFunction) const
		{
			return true;
		}
		bool IsEqual(const TDelegateBase* pDelegate) const override
		{ 
			return TypeId::DynamicCast<LambdaT>(pDelegate);
		}
	private:  
		LambdaType _pFunction;
	};  

	template<class ReturnValueType, class... Arguments> class Function final
	{
		typedef Function<ReturnValueType, Arguments...> TFunction;
		typedef DelegateBase<ReturnValueType, Arguments...> TDelegateBase;
		typedef ReturnValueType(*_function_ptr_)(Arguments...);
	public:
		TFunction() = default; 
		TFunction(const TFunction& otherFunction)
		{
			_pDelegate = UniquePtr<TDelegateBase>::MakeUniqueFromHere(otherFunction._pDelegate->createCopy());
		}
		TFunction(TFunction&& otherFunction)
		{
			_pDelegate = Move(otherFunction._pDelegate);
		}
		TFunction(const TDelegateBase& delegat)
		{
			_pDelegate = UniquePtr<TDelegateBase>::MakeUniqueFromHere(delegat.createCopy());
		}
		TFunction(TDelegateBase&& delegat)
		{
			_pDelegate = UniquePtr<TDelegateBase>::MakeUniqueFromHere(delegat.createCopy());
		}
		TFunction(_function_ptr_ pFunction)
		{
			_pDelegate = UniquePtr<TDelegateBase>::MakeUnique<FunctionPtr<ReturnValueType, Arguments...>>(pFunction); 
		}
		template<
			typename LambdaType, 
			typename Garbage = void,
			typename = std::enable_if_t<
			!std::is_base_of<TDelegateBase, LambdaType>::value
			&& !std::is_same<TDelegateBase, LambdaType>::value
			&& !std::is_same<TFunction, LambdaType>::value
			&& !std::is_reference<LambdaType>::value
			&& !std::is_rvalue_reference<LambdaType>::value
			>, 
			typename = std::enable_if_t<std::is_constructible<LambdaPtr<ReturnValueType, LambdaType, Arguments...>, LambdaType>::value>,
			typename = LambdaPtr<ReturnValueType, LambdaType, Arguments...>::LambdaT
		>
		TFunction(LambdaType lambda)
		{ 
			_pDelegate = UniquePtr<TDelegateBase>::MakeUnique<LambdaPtr<ReturnValueType, LambdaType, Arguments...>>(lambda);
		}
		template<
			class ClassName, 
			typename Method = typename ReturnValueType(ClassName::*)(Arguments...),
			typename = std::enable_if<std::is_constructible<Delegate<ClassName, ReturnValueType, Arguments...>, ClassName*, Method>::value>::type
		>
		TFunction(ClassName* pListener, Method pMethod)
		{ 
			_pDelegate = UniquePtr<TDelegateBase>::MakeUnique<Delegate<ClassName, ReturnValueType, Arguments...>>(pListener, pMethod);
		}
		template<
			class ClassName, 
			typename Method = typename ReturnValueType(ClassName::*)(Arguments...) const,
			typename = std::enable_if_t<std::is_constructible<ConstDelegate<ClassName, ReturnValueType, Arguments...>, ClassName*, Method>::value>
		>
		TFunction(const ClassName* pListener, Method pMethod)
		{ 
			_pDelegate = UniquePtr<TDelegateBase>::MakeUnique<ConstDelegate<ClassName, ReturnValueType, Arguments...>>(pListener, pMethod);
		}  
		TFunction& operator = (const TFunction& otherFunction)
		{  
			_pDelegate = otherFunction._pDelegate->createCopy();
			return *this;
		}
		TFunction& operator = (TFunction&& otherFunction)
		{
			_pDelegate = Move(otherFunction._pDelegate);
			return *this;
		}
		ReturnValueType call(Arguments... arguments)
		{
			return _pDelegate->call(Arguments(arguments)...);
		}
		bool operator == (const TFunction& OtherFucntion)
		{
			return _pDelegate->IsEqual(OtherFucntion.GetBaseDelegate());
		}
		const TDelegateBase* GetBaseDelegate() const
		{
			return _pDelegate.data();
		}
		bool IsValid() const
		{
			return _pDelegate.IsValid();
		}
		TDelegateBase* GetBaseDelegate()
		{
			return _pDelegate.data();
		}
	private:
		UniquePtr<TDelegateBase> _pDelegate;
	}; 
};