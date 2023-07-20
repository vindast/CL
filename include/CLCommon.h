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

	//CRC32 from https://habr.com/ru/articles/38622/
	unsigned long Crc32(const unsigned char* Buff, size_t Length);
}