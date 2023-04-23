#pragma once
#include "CLMacroHelper.h"

namespace CL
{
	template<class Object>
	CL_NO_DISCARD constexpr Object&& Move(Object& Obj) noexcept
	{
		return static_cast<Object&&>(Obj);
	}

	template<class Object>
	CL_NO_DISCARD constexpr Object&& Forward(Object&& Obj) noexcept
	{
		return (static_cast<Object&&>(Obj));
	}

	template<class Object>
	CL_NO_DISCARD constexpr Object&& Forward(Object& Obj) noexcept
	{
		return (static_cast<Object&&>(Obj));
	}
}