#pragma once

namespace CL
{
	template<class Object> class UniquePtr final
	{
	public:
  
		UniquePtr(Object* pObject = nullptr) noexcept :
			_pObject(pObject)
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

		static UniquePtr<Object>&& move(UniquePtr<Object>& ptr) noexcept
		{
		//	UniquePtr<Object> tmp(ptr._pObject);

		//	ptr._pObject = nullptr;

		//	return tmp;

			return static_cast<UniquePtr<Object>&&>(ptr);
		}
		 
		bool valid() const noexcept
		{
			return _pObject;
		}

		~UniquePtr()
		{
			freeObject();
		}
 
		Object* operator -> () noexcept
		{
			return _pObject;
		}

		const Object* operator -> () const noexcept
		{
			return _pObject;
		}



	private:
		Object* _pObject;

		UniquePtr(const UniquePtr<Object>& ptr) = delete;

		UniquePtr& operator = (const UniquePtr<Object>& ptr) = delete;

		/*UniquePtr& operator = (Object* pObject)
		{
			freeObject();

			_pObject = pObject;

			return *this;
		}*/
		 
		void freeObject()
		{
			if (_pObject)
			{
				delete _pObject;
				_pObject = nullptr;
			}
		}
	};
};