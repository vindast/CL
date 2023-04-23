#include "CLHashString.h"

CL::HashString::HashString() : _hash(0)
{

}

CL::HashString::HashString(const char* str) : _str(str), _hash(StringHash(str))
{

}

CL::HashString::HashString(const String& str) : _str(str), _hash(StringHash(str.GetData()))
{

}

CL::HashString::HashString(String&& str) : _str(str), _hash(StringHash(str.GetData()))
{

}

CL::HashString::HashString(const HashString& hashStr) : _str(hashStr._str), _hash(hashStr._hash)
{

}

CL::HashString::HashString(HashString&& hashStr) : _str(CL::Move(hashStr._str)), _hash(hashStr._hash)
{

}

constexpr size_t CL::StringHash(const char* pStr)
{
	size_t hash = 0, m = 1;
	const int k = 31, mod = int(1e9 + 7);

	while (*pStr != '\0')
	{
		int x = (int)(*pStr - 'a' + 1);
		hash = (hash + m * x) % mod;
		m = (m * k) % mod;
		pStr++;
	}

	return hash;
}

CL::HashStringView::HashStringView() : _pHashString(nullptr)
{

}

CL::HashStringView::HashStringView(const HashString* pHashString) : _pHashString(pHashString)
{

}

CL::HashStringView::HashStringView(const HashString& HashString) : _pHashString(&HashString)
{

}
