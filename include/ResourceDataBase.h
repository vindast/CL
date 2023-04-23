#pragma once
#include <map>
#include <string>
#include "list.h"


namespace CL
{  
	template<class ResourceType> class ResourceDataBaseHandle;
	template<class ResourceType> class ResourceDataBaseNativeHandle;
	template<class ResourceType> class ResourcesDataBase;

	template<class ResourceType> class NativeResource
	{
		friend class ResourceDataBaseNativeHandle<ResourceType>;
		friend class ResourceDataBaseHandle<ResourceType>;
		friend class ResourcesDataBase<ResourceType>;
	public:
		NativeResource(const std::string& name, ResourceType& resource, ResourcesDataBase<ResourceType>* db) :
			_resource(resource),
			_name(name),
			_db(db)
		{

		} 
		ResourceType& getResource()
		{
			return _resource;
		} 
		const ResourceType& getResource() const
		{
			return _resource;
		} 
		const std::string& getName() const
		{
			return _name;
		} 
		size_t getHandlesCount() const
		{
			return _handels.size();
		} 
	private:
		List<ResourceDataBaseHandle<ResourceType>*> _handels;
		List<ResourceDataBaseNativeHandle<ResourceType>*> _nativeHandels;
		ResourcesDataBase<ResourceType>* _db;

		std::string _name;
		ResourceType _resource;
		 
		NativeResource(const NativeResource&) = delete;
		NativeResource& operator = (const NativeResource&) = delete;
	};

	template<class ResourceType> class ResourceDataBaseNativeHandle
	{
		friend class ResourcesDataBase<ResourceType>;
	public:
		ResourceDataBaseNativeHandle()
		{

		}
		ResourceDataBaseNativeHandle(const ResourceDataBaseNativeHandle<ResourceType>& handle)
		{
			*this = handle;
		}
		ResourceDataBaseNativeHandle(NativeResource<ResourceType>& resurce)
		{
			*this = resurce;
		} 
		ResourceDataBaseNativeHandle<ResourceType>& operator = (NativeResource<ResourceType>& resurce)
		{
			unsubscribe();

			_resource = &resurce;
			_resource->_nativeHandels.push_back(this);
			_handleIt = _resource->_nativeHandels.rbegin();

			return *this;
		} 
		ResourceDataBaseNativeHandle<ResourceType>& operator = (const ResourceDataBaseNativeHandle<ResourceType>& handle)
		{
			unsubscribe();
			if (handle._resource)
			{
				_resource = handle._resource;
				_resource->_nativeHandels.push_back(this);
				_handleIt = _resource->_nativeHandels.rbegin();

			}
			 
			return *this;
		}
		void replaceResource(ResourceType resource)
		{
			_resource->_resource = resource;
		}
		ResourceType& getResource()
		{
			assert(_resource);
			return _resource->getResource();
		}
		NativeResource<ResourceType>* getNativeResource()
		{
			assert(_resource);
			return _resource;
		}
		bool isValid()
		{
			return _resource;
		}
		void closeHandle()
		{
			unsubscribe();
		}
		~ResourceDataBaseNativeHandle()
		{
			unsubscribe();
		}
	private:
		NativeResource<ResourceType>* _resource = nullptr;
		ListIterator<ResourceDataBaseNativeHandle<ResourceType>*> _handleIt;

		void unsubscribe()
		{
			if (_resource)
			{
				_resource->_nativeHandels.erase(_handleIt);
				_resource = nullptr;
			}
		}
	};

	template<class ResourceType> class ResourceDataBaseHandle
	{
		friend class ResourcesDataBase<ResourceType>;
	public:
		ResourceDataBaseHandle()
		{

		} 
		ResourceDataBaseHandle(NativeResource<ResourceType>& resource)
		{
			*this = resource;
		} 
		ResourceDataBaseHandle(const ResourceDataBaseHandle<ResourceType>& handle)
		{
			*this = handle;
		} 
		ResourceDataBaseHandle<ResourceType>& operator = (NativeResource<ResourceType>& resource)
		{
			unsubscribe();

			_nativeResource = &resource;
			_nativeResource->_handels.push_back(this);
			_handleIt = _nativeResource->_handels.rbegin();

			return *this;
		} 
		ResourceDataBaseHandle<ResourceType>& operator = (const ResourceDataBaseHandle<ResourceType>& handle)
		{
			unsubscribe();

			if (handle._nativeResource)
			{
				_nativeResource = handle._nativeResource;
				_nativeResource->_handels.push_back(this); 
				_handleIt = _nativeResource->_handels.rbegin();
			}

			return *this;
		} 
		bool isValid() const
		{
			return _nativeResource;
		} 
		const ResourceType& getResource() const
		{
			assert(_nativeResource);
			return _nativeResource->getResource();
		} 
		const ResourceType& operator ->() const
		{
			assert(_nativeResource);
			return _nativeResource->getResource();
		}
		const std::string& getName() const
		{
			assert(_nativeResource);
			return _nativeResource->getName();
		} 
		void closeHandle()
		{
			unsubscribe();
		} 
		~ResourceDataBaseHandle()
		{
			unsubscribe();
		} 
	private:
		NativeResource<ResourceType>* _nativeResource = nullptr;
		ListIterator<ResourceDataBaseHandle<ResourceType>*> _handleIt;
	
		void unsubscribe()
		{
			if (_nativeResource)
			{
				_nativeResource->_handels.erase(_handleIt);
				_nativeResource = nullptr;
			}
		}
	};

	template<class ResourceType> class ResourcesDataBase
	{
	public:
		ResourceDataBaseHandle<ResourceType> getResourceHandle(const std::string& name)
		{
			ResourceDataBaseHandle<ResourceType> handle;

			auto it = _mResource.find(name);

			if (it != _mResource.end())
			{
				handle = *it->second;
			}

			return handle;
		}
		ResourceDataBaseNativeHandle<ResourceType> getResourceNativeHandle(const std::string& name)
		{
			ResourceDataBaseNativeHandle<ResourceType> handle;

			auto it = _mResource.find(name);

			if (it != _mResource.end())
			{
				handle = *it->second;
			}

			return handle;
		}
		bool registerResource(const std::string& name, ResourceType& data)
		{
			auto it = _mResource.find(name);

			if (it != _mResource.end())
			{
				return false;
			}

			NativeResource<ResourceType>* resource = CL_NEW( NativeResource<ResourceType>, name, data, this);

			_mResource.insert(std::make_pair(name, resource));

			return true;
		}
		bool exist(const std::string& name) const
		{
			auto it = _mResource.find(name);

			return it != _mResource.end();
		}
		bool exist(const std::string& name) 
		{
			auto it = _mResource.find(name);

			return it != _mResource.end();
		}
		bool erase(ResourceDataBaseNativeHandle<ResourceType>& handle)
		{
			if (!handle.isValid() || handle._resource->_db != this)
			{
				return false;
			}

			auto it = _mResource.find(handle.getNativeResource()->getName());

			bool exist = it != _mResource.end();

			if (exist)
			{
				resourceDeleteHandles(it->second);

				CL_DELETE( it->second);
				_mResource.erase(it);
			}

			return exist;
		}
		bool rename(ResourceDataBaseNativeHandle<ResourceType>& handle, const std::string& newName)
		{
			if (!handle.isValid() || handle._resource->_db != this)
			{
				return false;
			}

			auto it = _mResource.find(handle.getNativeResource()->getName());

			bool exist = it != _mResource.end();

			if (exist)
			{
				auto data = it->second;
				it->second->_name = newName;
				_mResource.erase(it);
				_mResource.insert(std::make_pair(newName, data));
			}

			return exist; 
		}
		void clear()
		{
			for (auto it : _mResource)
			{
				resourceDeleteHandles(it.second);
				CL_DELETE( it.second);
			}

			_mResource.clear();
		}
		~ResourcesDataBase()
		{
			clear();
		}

		std::map<std::string, NativeResource<ResourceType>*> _mResource;
	private:
		 
		void resourceDeleteHandles(NativeResource<ResourceType>* resource)
		{
			for (auto it = resource->_handels.begin(); it != resource->_handels.end(); it++)
			{
				it()->closeHandle();
			}

			for (auto it = resource->_nativeHandels.begin(); it != resource->_nativeHandels.end(); it++)
			{
				it()->closeHandle();
			}
		} 
	};

	template<class resoursePtr> std::string uniqueName(const std::string& sStartName, const ResourcesDataBase<resoursePtr>& resourceMap)
	{
		std::string sName = sStartName;
		size_t iId = 0;

		while (resourceMap._mResource.find(sName) != resourceMap._mResource.end())
		{
			sName = sStartName + "_" + std::to_string(iId++);
		}

		return sName;
	}
};