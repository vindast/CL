#pragma once 
#include <CLCommon.h>
#include <CLAssert.h>
#include <CLMemory.h>
#include <atomic>

namespace CL
{
	template<class Object> class RefPtr;
	class ObserverPtr;

	class RefPtrCounterBase
	{  
	public:
		explicit RefPtrCounterBase() = default;
		virtual ~RefPtrCounterBase() = default;  

		std::atomic_size_t _RefCount = 0;
	private:
		RefPtrCounterBase(const RefPtrCounterBase&) = delete;
		RefPtrCounterBase& operator = (const RefPtrCounterBase&) = delete; 
	};

	template<class Object>
	class RefPtrCounter final : public RefPtrCounterBase
	{ 
	public:
		template<class... Args>
		RefPtrCounter(Args&&... args):
			_Object(args...)
		{

		} 
		~RefPtrCounter() = default; 

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
			SetNewCounter(ptr._pCounter);
		}
		RefPtr(RefPtr<Object>&& ptr) noexcept
		{
			_pCounter = ptr._pCounter;
			ptr._pCounter = nullptr;
		}
		RefPtr<Object>& operator = (const RefPtr<Object>& ptr)
		{
			if (this != &ptr)
			{
				SetNewCounter(ptr._pCounter);
			}

			return *this;
		}
		RefPtr<Object>& operator = (RefPtr<Object>&& ptr)
		{
			Free();
			_pCounter = ptr._pCounter;
			ptr._pCounter = nullptr;
			return *this;
		}
		template<class ... Args>
		static RefPtr<Object> MakeRefPtr(Args... args)
		{
			RefPtr<Object> ptr;
			ptr._pCounter = CL_NEW( RefPtrCounter<Object>, args...); 
			ptr._pCounter->_RefCount = 1;
			return ptr;
		}
		Object& operator* ()
		{
			CL_ASSERT(IsValid());
			return _pCounter->_Object;
		}
		const Object& operator* () const
		{
			CL_ASSERT(IsValid());
			return _pCounter->_Object;
		}
		Object* operator -> ()
		{
			CL_ASSERT(IsValid());
			return &_pCounter->_Object;
		}
		const Object* operator -> () const
		{ 
			CL_ASSERT(IsValid());
			return &_pCounter->_Object;
		}
		void Free()
		{
			if (!_pCounter)
			{
				return;
			}

			size_t count = --_pCounter->_RefCount;

			if (!_pCounter->_RefCount && !count)
			{
				CL_DELETE(_pCounter);
			}

			_pCounter = nullptr;
		}
		Object* Data()
		{
			return IsValid() ? &_pCounter->_Object : nullptr;
		}
		const Object* Data() const
		{
			return IsValid() ? &_pCounter->_Object : nullptr;
		}
		size_t GetRefCount() const
		{ 
			return _pCounter->_RefCount;
		} 
		bool IsValid() const
		{
			return _pCounter;
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
		RefPtrCounter<Object>* _pCounter = nullptr;

		void SetNewCounter(RefPtrCounter<Object>* pCounter)
		{
			if (pCounter == _pCounter)
			{
				return;
			}

			Free();

			if (pCounter)
			{
				_pCounter = pCounter;
				_pCounter->_RefCount++;
			}
		}
	};
	 
	class ObserverPtr final
	{
	public:
		ObserverPtr() = default;
		ObserverPtr(const ObserverPtr& ptr) noexcept
		{
			SetNewCounter(ptr._pCounter);
		}
		ObserverPtr(ObserverPtr&& ptr) noexcept
		{
			_pCounter = ptr._pCounter;
			ptr._pCounter = nullptr;
		}
		ObserverPtr& operator = (const ObserverPtr& ptr)
		{
			if (this != &ptr)
			{
				SetNewCounter(ptr._pCounter);
			}

			return *this;
		}
		ObserverPtr& operator = (ObserverPtr&& ptr)
		{ 
			SetNewCounter(ptr._pCounter);
			return *this;
		}
		template<class Object>
		ObserverPtr(const RefPtr<Object>& ptr) noexcept
		{
			SetNewCounter(ptr._pCounter);
		}
		template<class Object>
		ObserverPtr(RefPtr<Object>&& ptr) noexcept
		{
			_pCounter = ptr._pCounter;
			ptr._pCounter = nullptr;
		}
		template<class Object>
		ObserverPtr& operator = (const RefPtr<Object>& ptr)
		{
			SetNewCounter(ptr._pCounter);
			return *this;
		}
		template<class Object>
		ObserverPtr& operator = (RefPtr<Object>&& ptr)
		{
			Free();
			_pCounter = ptr._pCounter;
			ptr._pCounter = nullptr;
			return *this;
		} 
		void Free()
		{
			if (!_pCounter)
			{
				return;
			}

			size_t count = --_pCounter->_RefCount;

			if (!_pCounter->_RefCount && !count)
			{
				CL_DELETE(_pCounter);
			}

			_pCounter = nullptr;
		}
		bool IsValid() const { return _pCounter && _pCounter->_RefCount; }
		operator bool() const { return IsValid(); }
		~ObserverPtr()
		{
			Free();
		}
	private:
		void SetNewCounter(RefPtrCounterBase* pCounter)
		{
			if (pCounter == _pCounter)
			{
				return;
			}

			Free();

			if (pCounter)
			{
				_pCounter = pCounter;
				_pCounter->_RefCount++;
			}
		}

		RefPtrCounterBase* _pCounter = nullptr;
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
		return A._pCounter == B.Data();
	}

	template<class Object>
	bool operator != (const ObserverPtr& A, const RefPtr<Object>& B)
	{
		return A._pCounter != B.Data();
	}

	template<class Object>
	bool operator == (const RefPtr<Object&> A, const ObserverPtr& B)
	{
		return A.Data() == B._pCounter;
	}

	template<class Object>
	bool operator != (const RefPtr<Object&> A, const ObserverPtr& B)
	{
		return A.Data() != B._pCounter;
	}

	template<class Object>
	bool operator < (const RefPtr<Object&> A, const ObserverPtr& B)
	{
		return A.Data() < B._pCounter;
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