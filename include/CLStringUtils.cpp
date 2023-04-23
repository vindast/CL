#include "CLString.h"
#include "CLStringUtils.h"

#define CL_STRING_LOCAL_BUFFER_REALISATION

namespace CL
{
	String ToString(unsigned int n) noexcept
	{
		String str;

#ifdef CL_STRING_LOCAL_BUFFER_REALISATION 
		char buff[24];
		char* pStr = &buff[11];
#else
		char* pStr = &str._sbStr.mStr[11];
#endif

		* pStr = '\0';

		do
		{
			*--pStr = static_cast<char>(int('0') + n % 10);
			n /= 10;
		} while (n);

#ifdef CL_STRING_LOCAL_BUFFER_REALISATION
		memcpy(str._sbStr.mStr, pStr, &buff[12] - pStr);
		str._nLength = &buff[11] - pStr;
#else
		str._nLength = &str._sbStr.mStr[11] - pStr;
		memcpy(str._sbStr.mStr, pStr, str._nLength);
#endif

		return str;
	}

	String ToString(int n) noexcept
	{
		String str;

#ifdef CL_STRING_LOCAL_BUFFER_REALISATION 
		char buff[24];
		char* pStr = &buff[11];
#else
		char* pStr = &str._sbStr.mStr[11];
#endif

		* pStr = '\0';

		if (n < 0)
		{
			n = -n;

			do
			{
				*--pStr = static_cast<char>(int('0') + n % 10);
				n /= 10;
			} while (n);

			*--pStr = '-';
		}
		else
		{
			do
			{
				*--pStr = static_cast<char>(int('0') + n % 10);
				n /= 10;
			} while (n);
		}

#ifdef CL_STRING_LOCAL_BUFFER_REALISATION
		memcpy(str._sbStr.mStr, pStr, &buff[12] - pStr);
		str._nLength = &buff[11] - pStr;
#else
		str._nLength = &str._sbStr.mStr[11] - pStr;
		memcpy(str._sbStr.mStr, pStr, str._nLength);
#endif

		return str;
	}

	String FormatString(const char* fmt, ...)
	{
		char buf[CL_STR_MAX_FORMAT_CAPACITY];

		va_list args;
		va_start(args, fmt);

		int w = vsnprintf(buf, CL_STR_MAX_FORMAT_CAPACITY, fmt, args);
		if (w == -1 || w >= (int)CL_STR_MAX_FORMAT_CAPACITY)
			w = (int)CL_STR_MAX_FORMAT_CAPACITY - 1;
		buf[w] = 0;

		va_end(args);

		return String(buf, w);
	}
}