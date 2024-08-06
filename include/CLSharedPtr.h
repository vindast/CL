#pragma once
#include "CLAssert.h"
#include "CLMemory.h"
#include <atomic>
#include <type_traits>
 
namespace CL
{
	template<class Object>
	struct SharedPtrControlSection
	{
		SharedPtrControlSection(Object* pInObject) : 
			pObject(pInObject), nStrongRef(1), nWeakRef(0)
		{
			CL_DEBUG_ASSERT(pObject);
		}
		~SharedPtrControlSection()
		{
			CL_ASSERT(!pObject);
		}

		Object* pObject;
		std::atomic_size_t nStrongRef;
		std::atomic_size_t nWeakRef;
	};

	template<class Object>
	class SharedPtr final
	{
		template<class T>
		friend class WeakPtr;

		template<class T>
		friend class SharedPtr;

		typedef SharedPtrControlSection<Object> TConstrolSection;
		typedef SharedPtr<Object> TSharedPtr;
	public:
		SharedPtr() :
			_pControlSection(nullptr)
		{

		}
		TSharedPtr(const TSharedPtr& other)
		{
			_pControlSection = other._pControlSection;

			if (_pControlSection)
			{
				_pControlSection->nStrongRef++;
			}
		}
		TSharedPtr(TSharedPtr&& other)
		{
			_pControlSection = other._pControlSection;
			other._pControlSection = nullptr;
		}
		template<class T2>
		TSharedPtr(const SharedPtr<T2>& other)
		{
			static_assert(std::is_base_of<Object, T2>::value);
			_pControlSection = reinterpret_cast<TConstrolSection*>(other._pControlSection);

			if (_pControlSection)
			{
				_pControlSection->nStrongRef++;
			}
		}
		template<class T2>
		TSharedPtr(SharedPtr<T2>&& other)
		{
			static_assert(std::is_base_of<Object, T2>::value);
			_pControlSection = reinterpret_cast<TConstrolSection*>(other._pControlSection);
			other._pControlSection = nullptr;
		}
		TSharedPtr& operator = (const TSharedPtr& other)
		{
			if (this != &other)
			{
				Free();
				_pControlSection = other._pControlSection;

				if (_pControlSection)
				{
					_pControlSection->nStrongRef++;
				}
			}

			return *this;
		}
		TSharedPtr& operator = (TSharedPtr&& other)
		{
			Free();
			_pControlSection = other._pControlSection; 
			other._pControlSection = nullptr;
			return *this;
		}
		template<class T2>
		TSharedPtr& operator = (const SharedPtr<T2>& other)
		{
			static_assert(std::is_base_of<Object, T2>::value);

			if (this != &other)
			{
				Free();
				_pControlSection = reinterpret_cast<TConstrolSection*>(other._pControlSection);

				if (_pControlSection)
				{
					_pControlSection->nStrongRef++;
				}
			}

			return *this;
		}
		template<class T2>
		TSharedPtr& operator = (SharedPtr<T2>&& other)
		{
			static_assert(std::is_base_of<Object, T2>::value);
			Free();
			_pControlSection = reinterpret_cast<TConstrolSection*>(other._pControlSection);
			other._pControlSection = nullptr;
			return *this;
		}
		template<class T2>
		SharedPtr<T2> ReinterpretCast()
		{
			SharedPtr<T2> Ptr;
			if (_pControlSection)
			{
				Ptr._pControlSection = reinterpret_cast<SharedPtr<T2>::TConstrolSection*>(_pControlSection);
				Ptr._pControlSection->nStrongRef++;
			}
			return Ptr;
		}
		static TSharedPtr MakeSharedFromHere(Object* pObject)
		{
			TSharedPtr ptr;
			ptr._pControlSection = CL_NEW( TConstrolSection, pObject);
			return ptr;
		}
		template<class RealObjectClass, class ...Args >
		static TSharedPtr MakeSharedFromHere(Args&&... args)
		{
			TSharedPtr ptr;
			ptr._pControlSection = CL_NEW( TConstrolSection, CL_NEW(RealObjectClass, args...));
			return ptr;
		}
		template<class ...Args>
		static TSharedPtr MakeShared(Args&&... args)
		{
			TSharedPtr ptr;
			ptr._pControlSection = CL_NEW(TConstrolSection, CL_NEW(Object, args...));
			return ptr;
		}
		bool operator != (const TSharedPtr& other) const
		{
			return _pControlSection != other._pControlSection;
		}
		bool operator == (const TSharedPtr& other) const
		{
			return _pControlSection == other._pControlSection;
		}
		operator bool () const { return _pControlSection && _pControlSection->pObject; }
		bool IsValid() const { return _pControlSection && _pControlSection->pObject; }
		const Object* operator -> () const 
		{
			CL_ASSERT(IsValid());
			return _pControlSection->pObject;
		}
		Object* operator -> ()
		{
			CL_ASSERT(IsValid());
			return _pControlSection->pObject;
		}
		const Object* Data() const
		{
			CL_ASSERT(IsValid());
			return _pControlSection->pObject;
		}
		Object* Data()
		{
			CL_ASSERT(IsValid());
			return _pControlSection->pObject;
		}
		void Free()
		{
			if (!_pControlSection)
			{
				return;
			}

			CL_DEBUG_ASSERT(_pControlSection->nStrongRef);

			size_t strongRef = --_pControlSection->nStrongRef;
			size_t weakRef   = _pControlSection->nWeakRef;

			if (!strongRef && !_pControlSection->nStrongRef)
			{
				CL_DELETE(_pControlSection->pObject);
				_pControlSection->pObject = nullptr;
			
				if (!weakRef && !_pControlSection->nWeakRef)
				{
					CL_DELETE(_pControlSection);
				}
			}

			_pControlSection = nullptr;
		}
		size_t GetStrongRefCount() const { return _pControlSection ? _pControlSection->nStrongRef : 0; }
		~SharedPtr()
		{
			Free();
		}
	private:
		TConstrolSection* _pControlSection;
	};

	template<class Object>
	class WeakPtr
	{
		typedef SharedPtrControlSection<Object> TConstrolSection;
		typedef SharedPtr<Object> TSharedPtr;
		typedef WeakPtr<Object> TWeakPtr;
	public:
		WeakPtr() : _pControlSection(nullptr)
		{

		}
		TWeakPtr(const TWeakPtr& other)
		{
			if (other._pControlSection && other._pControlSection->nStrongRef)
			{
				_pControlSection = other._pControlSection;
				_pControlSection->nWeakRef++;
			}
			else
			{
				_pControlSection = nullptr;
			}
		}
		TWeakPtr& operator = (const TWeakPtr& other)
		{
			Free();

			if (other._pControlSection && other._pControlSection->nStrongRef)
			{
				_pControlSection = other._pControlSection;
				_pControlSection->nWeakRef++;
			}
			else
			{
				_pControlSection = nullptr;
			}

			return *this;
		}
		TWeakPtr(TWeakPtr&& other)
		{
			if (other._pControlSection && other._pControlSection->nStrongRef)
			{
				_pControlSection = other._pControlSection;
				other._pControlSection = nullptr;
			}
			else
			{
				_pControlSection = nullptr;
			}
		}
		TWeakPtr& operator = (TWeakPtr&& other)
		{
			Free();

			if (other._pControlSection && other._pControlSection->nStrongRef)
			{
				_pControlSection = other._pControlSection;
				other._pControlSection = nullptr;
			}

			return *this;
		}
		TWeakPtr(const TSharedPtr& other)
		{
			if (other._pControlSection && other._pControlSection->nStrongRef)
			{
				_pControlSection = other._pControlSection;
				_pControlSection->nWeakRef++;
			}
			else
			{
				_pControlSection = nullptr;
			}
		}
		TWeakPtr& operator = (const TSharedPtr& other)
		{
			Free();

			if (other._pControlSection && other._pControlSection->nStrongRef)
			{
				_pControlSection = other._pControlSection;
				_pControlSection->nWeakRef++;
			}
			else
			{
				_pControlSection = nullptr;
			}

			return *this;
		}
		bool IsValid() const { return _pControlSection && _pControlSection->pObject; }
		void Free()
		{
			if (!_pControlSection)
			{
				return;
			}

			size_t strongRef = _pControlSection->nStrongRef;
			size_t weakRef   = --_pControlSection->nWeakRef;

			if (!strongRef && !_pControlSection->nStrongRef && !weakRef && !_pControlSection->nWeakRef)
			{
				CL_DEBUG_ASSERT(!_pControlSection->pObject)
				CL_DELETE(_pControlSection);
			}

			_pControlSection = nullptr;
		}
		const Object* operator -> () const
		{
			CL_ASSERT(IsValid());
			return _pControlSection->pObject;
		}
		Object* operator -> ()
		{
			CL_ASSERT(IsValid());
			return _pControlSection->pObject;
		}
		const Object* Data() const
		{
			CL_ASSERT(IsValid());
			return _pControlSection->pObject;
		}
		Object* Data()
		{
			CL_ASSERT(IsValid());
			return _pControlSection->pObject;
		}
		~WeakPtr()
		{
			Free();
		}
	private:
		TConstrolSection* _pControlSection;
	};
}

template<class Obj>
struct std::hash<CL::SharedPtr<Obj>>
{
	std::size_t operator()(const CL::SharedPtr<Obj>& key) const
	{
		return size_t(key.Data());
	}
};