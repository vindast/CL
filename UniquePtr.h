#pragma once

namespace CL
{
	template<class Object> class UniquePtr final
	{
	public:

		UniquePtr() : _pObject(nullptr)
		{

		}

		UniquePtr(Object* pObject) :
			_pObject(pObject)
		{

		}

		UniquePtr(UniquePtr<Object>& ptr) :
			_pObject(ptr._pObject)
		{
			ptr._pObject = nullptr;
		} 

		UniquePtr& operator = (UniquePtr<Object>& ptr)
		{
			freeObject();

			_pObject = ptr._pObject;
			ptr._pObject = nullptr;

			return *this;
		}

		UniquePtr& operator = (Object* pObject)
		{
			freeObject();

			_pObject = pObject;

			return *this;
		}
		 
		bool valid() const
		{
			return _pObject;
		}

		~UniquePtr()
		{
			freeObject();
		}

		/*Object* operator ()()
		{
			return _pObject;
		}

		const Object* operator ()() const
		{
			return _pObject;
		}*/

		Object* operator -> ()
		{
			return _pObject;
		}

		const Object* operator -> () const
		{
			return _pObject;
		}

	private:
		Object* _pObject;

		/*UniquePtr(const UniquePtr<Object>& ptr) = delete;

		UniquePtr& operator = (const UniquePtr<Object>& ptr) = delete;*/

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