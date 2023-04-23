#pragma once
#include "list.h"
#include "CLMemory.h"
#include <iostream>
#include <string>
#include <map>

#define _STRINGIZE_HELPER_(x) #x



namespace CL
{
	 
	class EventSignalDescriptorBase
	{
	public:

		EventSignalDescriptorBase(std::string name):
			_name(name)
		{

		}

		virtual ~EventSignalDescriptorBase()
		{

		}

		std::string _name;
	}; 

	template<class Listener, class... Arguments> class EventSignalDescriptor : public EventSignalDescriptorBase
	{
		typedef void (Listener::* Method)(Arguments...);
	public:
		EventSignalDescriptor(Listener* pListener, Method pMethod, std::string name) :
			EventSignalDescriptorBase(name),
			_pListener(pListener), _pMethod(pMethod) 
		{

		}


		Listener* _pListener;
		Method _pMethod; 
	};
	 
	class EventLisnerHolder
	{
	public:

		void addNewListener(EventSignalDescriptorBase* pDesc)
		{
			assert(listeners.find(pDesc->_name) == listeners.end());

			listeners.insert(std::make_pair(pDesc->_name, pDesc));
		}


		std::map<std::string, EventSignalDescriptorBase*> listeners;

		~EventLisnerHolder()
		{
			for (auto pDesc : listeners)
			{
				CL_DELETE(pDesc.second);
			}
		}
	};

};