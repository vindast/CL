#pragma once
#include "Assert.h"
#include "CLMemory.h"

namespace CL
{
	template<class Object> class UniquePtr final
	{
	public: 
		UniquePtr() noexcept :
			_pObject(nullptr)
		{

		} 
		UniquePtr(UniquePtr<Object>&& ptr) noexcept :
			_pObject(ptr._pObject)
		{
			ptr._pObject = nullptr;
		} 
		UniquePtr& operator = (UniquePtr<Object>&& ptr) noexcept
		{
			freeObject();

			_pObject = ptr._pObject;
			ptr._pObject = nullptr;

			return *this;
		}
		template<class SubClass>
		static UniquePtr<Object> MakeUniqueFromHere(SubClass* pObject)
		{
			UniquePtr<Object> pObj;
			pObj._pObject = pObject;
			return pObj;
		}
		template<class SubClass, class... Args>
		static UniquePtr<Object> MakeUnique(Args&&... args)
		{
			UniquePtr<Object> pObj;
			pObj._pObject = CL_NEW( SubClass, args...);
			return pObj;
		}
		template<class... Args>
		static UniquePtr<Object> MakeUnique(Args&&... args)
		{
			UniquePtr<Object> pObj;
			pObj._pObject = CL_NEW( Object, args...);
			return pObj;
		}
		bool IsValid() const noexcept
		{
			return _pObject;
		}
		bool valid() const noexcept
		{
			return IsValid();
		} 
		const Object* Data() const noexcept
		{
			return _pObject;
		}
		Object* Data() noexcept
		{
			return _pObject;
		}
		const Object* data() const noexcept
		{
			return Data();
		} 
		Object* data() noexcept
		{
			return Data();
		} 
		Object* operator -> () noexcept
		{
			return _pObject;
		} 
		const Object* operator -> () const noexcept
		{
			return _pObject;
		} 
		Object& operator * () noexcept
		{
			return *_pObject;
		} 
		const Object& operator * () const noexcept
		{
			return *_pObject;
		} 
		void Free()
		{
			if (_pObject)
			{
				CL_DELETE( _pObject);
				_pObject = nullptr;
			}
		}
		void freeObject()
		{
			Free();
		}
		~UniquePtr()
		{
			Free();
		}
	private:
		Object* _pObject;

		UniquePtr(const UniquePtr<Object>& ptr) = delete; 
		UniquePtr& operator = (const UniquePtr<Object>& ptr) = delete;
	};
};