#pragma once
#include <type_traits>
#include "CLAssert.h"
#ifdef _DEBUG
#include <iostream>
#endif
namespace CL
{ 
	class TypeId
	{
		template<class FinalClass>
		friend class TypeIdWrapper;
	public: 
		size_t GetTypeId() const noexcept
		{
			return _ObjTypeId;
		}
		template<class A, class B>  
		static A* DynamicCast(B* pObj) noexcept
		{
			if (pObj)
			{
				if (A::TypeIndex() == pObj->GetTypeId())
				{
					return static_cast<A*>(pObj);
				}
			}

			return nullptr;
		}
		template<class A, class B>
		static const A* DynamicCast(const B* pObj) noexcept
		{
			if (pObj)
			{
				if (A::TypeIndex() == pObj->GetTypeId())
				{
					return static_cast<const A*>(pObj);
				}
			}

			return nullptr;
		}
		virtual ~TypeId() = default;
	private:  
		static size_t CreateNewTypeIndex();

		size_t _ObjTypeId;
	};

	template<class FinalClass>
	class TypeIdWrapper : public virtual TypeId
	{
	public:   
		TypeIdWrapper() noexcept
		{
			static_assert(std::is_final<FinalClass>::value);
			_TypeId = TypeIndex();
			_ObjTypeId = _TypeId;
		} 
		static size_t TypeIndex() noexcept
		{
			if (!_TypeId)
			{ 
				_TypeId = TypeId::CreateNewTypeIndex(); 
/*#ifdef _DEBUG
				std::cout << typeid(FinalClass).name() << ", id = " << _TypeId << std::endl;
#endif*/
			}
			return _TypeId;
		}
		static const char* GetTypeName() noexcept
		{
			return typeid(FinalClass).name();
		}
 	private:
		static size_t _TypeId;
	}; 

	template<class FinalClass>
	size_t TypeIdWrapper<FinalClass>::_TypeId = 0;
}