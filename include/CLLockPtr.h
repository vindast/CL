#pragma once

namespace CL
{
	template<class PtrType, class LockType>
	class LockPtr
	{
	public:
		LockPtr(PtrType& Ptr, LockType& Lockable) :
			_Ptr(Ptr), _Lockable(Lockable)
		{
			_Lockable.lock();
		}
		operator PtrType& () const { return _Ptr; }
		PtrType& Get() { return _Ptr; }
		const PtrType& Get() const { return _Ptr; }
		~LockPtr()
		{
			_Lockable.unlock();
		}
	private:
		PtrType& _Ptr;
		LockType& _Lockable;
	};
}