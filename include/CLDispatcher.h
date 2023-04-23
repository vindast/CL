#pragma once
#include <vector>
#include "Delegate.h"

namespace CL
{
	template<class... EventParams>
	class Dispatcher final
	{
		typedef Dispatcher<EventParams...> DispatcherT;
		typedef Function<void, EventParams...> EventCallback;
		DispatcherT(const DispatcherT&) = delete;
		DispatcherT& operator = (const DispatcherT&) = delete;
	public:
		DispatcherT() = default;
		template<class ClassName>
		void Subscribe(ClassName&& arg)
		{
			_mListeners.push_back(EventCallback(arg));
		}
		template<
			class ClassName,
			typename Method = typename void(ClassName::*)(EventParams...),
			typename = typename std::enable_if<std::is_constructible<Delegate<ClassName, void, EventParams...>, ClassName*, Method>::value>::type
		>
		void Subscribe(ClassName* obj, Method method)
		{
			_mListeners.push_back(EventCallback(obj, method));
		}
		template<
			class ClassName,
			typename Method = typename void(ClassName::*)(EventParams...) const,
			typename = typename std::enable_if<std::is_constructible<ConstDelegate<ClassName, void, EventParams...>, ClassName*, Method>::value>::type
		>
		void Subscribe(const ClassName* obj, Method method)
		{
			_mListeners.push_back(ConstDelegate<ClassName, void, EventParams...>(obj, method));
		//	_mListeners.push_back(EventCallback(obj, method));
		}
		template<
			class ClassName,
			typename Method = typename void(ClassName::*)(EventParams...),
			typename = typename std::enable_if<std::is_constructible<Delegate<ClassName, void, EventParams...>, ClassName*, Method>::value>::type
		>
		void Unsubscribe(ClassName* obj, Method method)
		{
			for (size_t i = 0; i < _mListeners.size(); )
			{
				auto pDelegat = TypeId::StaticCast<Delegate<ClassName, void, EventParams...>>(_mListeners[i].GetBaseDelegate());

				if (pDelegat && pDelegat->IsEqual(obj, method))
				{
					_mListeners.erase(_mListeners.begin() + i);
				}
				else
				{
					i++;
				}
			}
		}
		template<
			class ClassName,
			typename Method = typename void(ClassName::*)(EventParams...) const,
			typename = typename std::enable_if<std::is_constructible<ConstDelegate<ClassName, void, EventParams...>, ClassName*, Method>::value>::type
		>
		void Unsubscribe(const ClassName* obj, Method method)
		{
			for (size_t i = 0; i < _mListeners.size(); )
			{
				auto pDelegat = TypeId::StaticCast<ConstDelegate<ClassName, void, EventParams...>>(_mListeners[i].GetBaseDelegate());

				if (pDelegat && pDelegat->IsEqual(obj, method))
				{
					_mListeners.erase(_mListeners.begin() + i);
				}
				else
				{
					i++;
				}
			}
		}
		template<class ClassName, typename = typename std::enable_if<std::is_constructible<FunctionPtr<void, EventParams...>, ClassName>::value>::type>
		void Unsubscribe(ClassName obj)
		{  
			for (size_t i = 0; i < _mListeners.size(); )
			{
				auto pDelegat = TypeId::StaticCast<FunctionPtr<void, EventParams...>>(_mListeners[i].GetBaseDelegate());

				if (pDelegat && pDelegat->IsEqual(obj)) 
				{
					_mListeners.erase(_mListeners.begin() + i);
				}
				else
				{
					i++;
				}
			}
		}
		template<class ClassName,
			typename = typename std::enable_if<!std::is_same<EventCallback, ClassName>::value>::type,
			typename = typename std::enable_if<!std::is_constructible<FunctionPtr<void, EventParams...>, ClassName>::value>::type,
			typename = typename std::enable_if<std::is_constructible<LambdaPtr<void, ClassName, EventParams...>, ClassName>::value>::type
		>
		void Unsubscribe(ClassName obj)
		{
			for (size_t i = 0; i < _mListeners.size(); )
			{ 
				if (TypeId::StaticCast<LambdaPtr<void, ClassName, EventParams...>>(_mListeners[i].GetBaseDelegate()))
				{
					_mListeners.erase(_mListeners.begin() + i);
				}
				else
				{
					i++;
				}
			}
		} 
		void Unsubscribe(const EventCallback& callback)
		{
			for (size_t i = 0; i < _mListeners.size(); )
			{
				if (_mListeners[i] == callback)
				{
					_mListeners.erase(_mListeners.begin() + i);
				}
				else
				{
					i++;
				}
			}
		}
		void Notify(EventParams&&... Params)
		{
			for (EventCallback& callback : _mListeners)
			{
				callback.call(Params...);
			}
		}
		~Dispatcher()
		{

		}
	private:
		std::vector<EventCallback> _mListeners;
	};
}