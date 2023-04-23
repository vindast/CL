#pragma once
#include <CLMacroHelper.h>

#define CL_SCOPE_LOCK_GUARD(lockObj)\
CL::LockGuard CL_MAKE_CHAR_PTR_SUMM(cl_macro_scope_lock_guard_,__LINE__)(lockObj);

namespace CL
{
	template<class LockObj> class LockGuard
	{
	public:
		LockGuard(LockObj& obj) :
			_obj(obj)
		{
			obj.lock();
		}
		~LockGuard()
		{
			_obj.unlock();
		}
	private:
		LockGuard(const LockGuard&) = delete;
		LockGuard& operator = (const LockGuard&) = delete;

		LockObj& _obj;
	};
}