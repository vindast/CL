#pragma once
#include <assert.h>
#include <map>
#include <string>
#include "list.h"
#include "UniquePtr.h"
#include "Delegate.h"

namespace CL
{
	class ResourceHandleBase
	{
	public:
		virtual void unsubscribe() = 0;
	};

	class ResourceBase
	{
	public:
		ResourceBase()
		{

		}
		size_t getHandlesCount() const
		{
			return _handles.size();
		}
		virtual ~ResourceBase()
		{
			for (auto pHandle : _handles)
			{
				pHandle->unsubscribe();
			}
		}

		List<ResourceHandleBase*> _handles;
	private:
		ResourceBase(const ResourceBase&) = delete;
		ResourceBase& operator = (const ResourceBase&) = delete;
	};

	template<class ResourceType> class Resource : public ResourceBase
	{
		typedef void (ResourceType::* SetNewNameMethod)(const std::string&);
	public:
		Resource(ResourceType* pResource) :
			_pResource(pResource)
		{

		}
		ResourceType* operator -> ()
		{
			return _pResource.data();
		}
		const ResourceType* operator -> () const
		{
			return _pResource.data();
		}
		ResourceType* data()
		{
			return _pResource.data();
		}
		const ResourceType* data() const
		{
			return _pResource.data();
		}
		void setNewName(const std::string& sName)
		{
			if (_pSetNameCallback.valid())
			{
				_pSetNameCallback.data()->call(sName);
			}
		}
		void createCallback(SetNewNameMethod pMethod)
		{
			_pSetNameCallback = new Delegate<ResourceType, void, const std::string&>(_pResource.data(), pMethod);
		}
	private:
		UniquePtr<Delegate<ResourceType, void, const std::string&>> _pSetNameCallback;
		UniquePtr<ResourceType> _pResource;
	};

	template<class ObjType> class ResourceHandle final : public ResourceHandleBase
	{
	public:
		ResourceHandle()
		{

		}
		template<class ResourceType> ResourceHandle(Resource<ResourceType>& resource)
		{
			setNewResource(resource);
		}
		template<class ResourceType> ResourceHandle(const ResourceHandle<ResourceType>& hResource)
		{
			if (hResource._pResource)
			{
				setNewResource(hResource);
			}
		}
		template<class ResourceType> ResourceHandle<ObjType>& operator = (const ResourceHandle<ResourceType>& hResource)
		{
			unsubscribe();
			if (hResource._pResource)
			{
				setNewResource(hResource);
			}

			return *this;
		}
		template<class ResourceType> ResourceHandle<ObjType>& operator = (Resource<ResourceType>& resource)
		{
			unsubscribe();
			setNewResource(resource);
			return *this;
		}
		void unsubscribe() override
		{
			if (_pResource)
			{
				_pResource->_handles.erase(_it);
				_pResource = nullptr;
				_pObj = nullptr;
			}
		}
		bool isValid() const
		{
			return _pResource;
		}
		const ObjType* operator -> () const
		{
			assert(_pObj);
			return _pObj;
		}
		ObjType* operator -> ()
		{
			assert(_pObj);
			return _pObj;
		}

		ObjType* _pObj = nullptr;
		ResourceBase* _pResource = nullptr;
	private:
		ListIterator<ResourceHandleBase*> _it;

		template<class ResourceType> void setNewResource(Resource<ResourceType>& resource)
		{
			resource._handles.push_back(this);
			_it = resource._handles.rbegin();
			_pResource = &resource;
			_pObj = resource.data();
		} 
		template<class ResourceType> void setNewResource(const ResourceHandle<ResourceType>& hResource)
		{
			_pResource = hResource._pResource;
			hResource._pResource->_handles.push_back(this);
			_it = _pResource->_handles.rbegin(); 
			_pObj = hResource._pObj;
		}
	};

	template<class ResourceType> class ResourceDB
	{
		typedef void (ResourceType::* SetNewNameMethod)(const std::string&);
	public:
		ResourceDB()
		{

		}
		Resource<ResourceType>* regResource(const std::string& sName, ResourceType* pResource)
		{
			Resource<ResourceType>* pDBResource = nullptr;

			auto it = resources.find(sName);

			if (it == resources.end())
			{ 
				pDBResource = new Resource<ResourceType>(pResource);
				
				resources.insert(std::make_pair(sName, pDBResource));
			}

			return pDBResource;
		}
		Resource<ResourceType>* regResource(const std::string& sName, ResourceType* pResource, SetNewNameMethod pMethod)
		{
			Resource<ResourceType>* pDBResource = nullptr;

			auto it = resources.find(sName);

			if (it == resources.end())
			{
				pDBResource = new Resource<ResourceType>(pResource);
				pDBResource->createCallback(pMethod);
				resources.insert(std::make_pair(sName, pDBResource));
			}

			return pDBResource;
		}
		bool erase(const std::string& sName)
		{
			bool bErased = false;

			auto it = resources.find(sName);

			if (it != resources.end())
			{
				bErased = true;
				resources.erase(it);
			}

			return bErased;
		}
		bool rename(const std::string& sOldName, const std::string& sNewName)
		{
			bool bRenamed = false;
			auto itOldName = resources.find(sOldName);
			auto itNewName = resources.find(sNewName);

			if (itOldName != resources.end() && itNewName == resources.end())
			{
				bRenamed = true;
				itOldName->second->setNewName(sNewName);

				resources.insert(std::make_pair(sNewName, move(itOldName->second)));
				resources.erase(itOldName);
			}

			return bRenamed;
		}
		bool exist(const std::string& sName) const
		{
			auto it = resources.find(sName);
			return it != resources.end();
		}
		Resource<ResourceType>* find(const std::string& sName)
		{
			auto it = resources.find(sName);

			assert(it != resources.end());

			return it->second.data();
		}

		std::map<std::string, UniquePtr<Resource<ResourceType>>> resources;
	private:
		ResourceDB(const ResourceDB&) = delete;
		ResourceDB& operator = (const ResourceDB&) = delete;
	};
};