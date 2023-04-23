#pragma once

namespace CL
{
	__forceinline size_t BytesFromMB(size_t MB)
	{
		return MB * 1024 * 1024;
	}
}