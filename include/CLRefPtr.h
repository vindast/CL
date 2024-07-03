#pragma once 
#include <CLCommon.h>
#include <CLAssert.h>
#include <CLMemory.h>
#include <atomic>

namespace CL
{
	template<class Object> class RefPtr;
	class ObserverPtr;

	class RefPtrContanerBase
	{  
	public:
		explicit RefPtrContanerBase() = default;
		virtual ~RefPtrContanerBase() = default;  

		std::atomic_size_t _RefCount = 0;
	private:
		RefPtrContanerBase(const RefPtrContanerBase&) = delete;
		RefPtrContanerBase& operator = (const RefPtrContanerBase&) = delete; 
	};

	template<class Object>
	class RefPtrContainer final : public RefPtrContanerBase
	{ 
	public:
		template<class... Args>
		RefPtrContainer(Args&&... args):
			_Object(args...)
		{

		} 
		~RefPtrContainer() = default; 

		Object _Object;
	};

	template<class Object>
	class RefPtr final
	{
		friend class ObserverPtr;
	public:
		RefPtr() = default;
		RefPtr(const RefPtr<Object>& ptr) noexcept
		{
			CL_SCOPE_LOCK_GUARD(GetRefPtrCriticalSection());
			SetNewCounter(ptr._pContainer);
		}
		RefPtr(RefPtr<Object>&& ptr) noexcept
		{
			CL_SCOPE_LOCK_GUARD(GetRefPtrCriticalSection());
			_pContainer = ptr._pContainer;
			ptr._pContainer = nullptr;
		}
		RefPtr<Object>& operator = (const RefPtr<Object>& ptr)
		{
			if (this != &ptr)
			{
				CL_SCOPE_LOCK_GUARD(GetRefPtrCriticalSection());
				SetNewCounter(ptr._pContainer);
			}

			return *this;
		}
		RefPtr<Object>& operator = (RefPtr<Object>&& ptr)
		{
			CL_SCOPE_LOCK_GUARD(GetRefPtrCriticalSection());
			Free();
			_pContainer = ptr._pContainer;
			ptr._pContainer = nullptr;
			return *this;
		}
		template<class ... Args>
		static RefPtr<Object> MakeRefPtr(Args... args)
		{
			RefPtr<Object> ptr;
			ptr._pContainer = CL_NEW( RefPtrContainer<Object>, args...); 
			ptr._pContainer->_RefCount = 1;
			return ptr;
		}
		Object& operator* ()
		{
			CL_ASSERT(IsValid());
			return _pContainer->_Object;
		}
		const Object& operator* () const
		{
			CL_ASSERT(IsValid());
			return _pContainer->_Object;
		}
		Object* operator -> ()
		{
			CL_ASSERT(IsValid());
			return &_pContainer->_Object;
		}
		const Object* operator -> () const
		{ 
			CL_ASSERT(IsValid());
			return &_pContainer->_Object;
		}
		void Free()
		{
			if (!_pContainer)
			{
				return;
			}

			CL_SCOPE_LOCK_GUARD(GetRefPtrCriticalSection());

			if ((--_pContainer->_RefCount) == 0)
			{
				CL_DELETE(_pContainer);
			}

			_pContainer = nullptr;
		}
		Object* Data()
		{
			return IsValid() ? &_pContainer->_Object : nullptr;
		}
		const Object* Data() const
		{
			return IsValid() ? &_pContainer->_Object : nullptr;
		}
		size_t GetRefCount() const
		{ 
			return _pContainer->_RefCount;
		} 
		bool IsValid() const
		{
			return _pContainer;
		}
		operator bool() const
		{
			return IsValid();
		}
		~RefPtr()
		{
			Free();
		}
	private:
		RefPtrContainer<Object>* _pContainer = nullptr;

		void SetNewCounter(RefPtrContainer<Object>* pContainer)
		{
			if (pContainer == _pContainer)
			{
				return;
			}

			Free();

			if (pContainer)
			{
				_pContainer = pContainer;
				_pContainer->_RefCount++;
			}
		}
	};
	 
	class ObserverPtr final
	{
	public:
		ObserverPtr() = default;
		ObserverPtr(const ObserverPtr& ptr) noexcept
		{
			CL_SCOPE_LOCK_GUARD(GetRefPtrCriticalSection());
			SetNewCounter(ptr._pContainer);
		}
		ObserverPtr(ObserverPtr&& ptr) noexcept
		{
			CL_SCOPE_LOCK_GUARD(GetRefPtrCriticalSection());
			_pContainer = ptr._pContainer;
			ptr._pContainer = nullptr;
		}
		ObserverPtr& operator = (const ObserverPtr& ptr)
		{
			if (this != &ptr)
			{
				CL_SCOPE_LOCK_GUARD(GetRefPtrCriticalSection());
				SetNewCounter(ptr._pContainer);
			}

			return *this;
		}
		ObserverPtr& operator = (ObserverPtr&& ptr)
		{
			CL_SCOPE_LOCK_GUARD(GetRefPtrCriticalSection());
			SetNewCounter(ptr._pContainer);
			return *this;
		}
		template<class Object>
		ObserverPtr(const RefPtr<Object>& ptr) noexcept
		{
			CL_SCOPE_LOCK_GUARD(GetRefPtrCriticalSection());
			SetNewCounter(ptr._pContainer);
		}
		template<class Object>
		ObserverPtr(RefPtr<Object>&& ptr) noexcept
		{
			CL_SCOPE_LOCK_GUARD(GetRefPtrCriticalSection());
			_pContainer = ptr._pContainer;
			ptr._pContainer = nullptr;
		}
		template<class Object>
		ObserverPtr& operator = (const RefPtr<Object>& ptr)
		{
			SetNewCounter(ptr._pContainer);
			return *this;
		}
		template<class Object>
		ObserverPtr& operator = (RefPtr<Object>&& ptr)
		{
			Free();
			_pContainer = ptr._pContainer;
			ptr._pContainer = nullptr;
			return *this;
		} 
		void Free()
		{
			if (!_pContainer)
			{
				return;
			}

			CL_SCOPE_LOCK_GUARD(GetRefPtrCriticalSection());

			if ((--_pContainer->_RefCount) == 0)
			{
				CL_DELETE(_pContainer);
			}

			_pContainer = nullptr;
		}
		bool IsValid() const { return _pContainer && _pContainer->_RefCount; }
		operator bool() const { return IsValid(); }
		~ObserverPtr()
		{
			Free();
		}
	private:
		void SetNewCounter(RefPtrContanerBase* pContainer)
		{
			if (pContainer == _pContainer)
			{
				return;
			}

			Free();

			if (pContainer)
			{
				_pContainer = pContainer;
				_pContainer->_RefCount++;
			}
		}

		RefPtrContanerBase* _pContainer = nullptr;
	};

	template<class Object>
	bool operator == (const RefPtr<Object>& A, const RefPtr<Object>& B)
	{ 
		return A.Data() == B.Data();
	}

	template<class Object>
	bool operator != (const RefPtr<Object>& A, const RefPtr<Object>& B)
	{
		return A.Data() != B.Data();
	}

	template<class Object>
	bool operator == (const ObserverPtr& A, const RefPtr<Object>& B)
	{
		return A._pContainer == B.Data();
	}

	template<class Object>
	bool operator != (const ObserverPtr& A, const RefPtr<Object>& B)
	{
		return A._pContainer != B.Data();
	}

	template<class Object>
	bool operator == (const RefPtr<Object&> A, const ObserverPtr& B)
	{
		return A.Data() == B._pContainer;
	}

	template<class Object>
	bool operator != (const RefPtr<Object&> A, const ObserverPtr& B)
	{
		return A.Data() != B._pContainer;
	}

	template<class Object>
	bool operator < (const RefPtr<Object&> A, const ObserverPtr& B)
	{
		return A.Data() < B._pContainer;
	}
}

template<class T>
struct std::hash<CL::RefPtr<T>>
{
	size_t operator()(const CL::RefPtr<T>& key) const
	{
		return reinterpret_cast<size_t>(key.Data());
	}
};