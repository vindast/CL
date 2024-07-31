#include "CLString.h"


/*template<size_t N>
String(const char(&mStr)[N])
{
printf("const char(&mStr)[N]\n");
_nLength = N - 1;

if (_nLength < sizeof(SmallStringBuffer))
{
memcpy(_sbStr.mStr, mStr, _nLength + 1);
}
else
{
_heapStr._nCapacity = CL_ALIGN(_nLength, CL_STR_ALIGNMENT);
_heapStr._pStr = (char*)CL_MALLOC(_heapStr._nCapacity);
memcpy(_heapStr._pStr, mStr, _nLength + 1);
}
}
template<size_t N>
String& operator = (const char(&mStr)[N])
{
printf("String& operator = (const char(&mStr)[N])\n");
ClearHeapFromNewSize(N - 1);
_nLength = N - 1;

if (_nLength < sizeof(SmallStringBuffer))
{
memcpy(_sbStr.mStr, mStr, _nLength + 1);
}
else
{
_heapStr._nCapacity = CL_ALIGN(_nLength, CL_STR_ALIGNMENT);
_heapStr._pStr = (char*)CL_MALLOC(_heapStr._nCapacity);
memcpy(_heapStr._pStr, mStr, _nLength + 1);
}

return *this;
}*/

CL::String::String() noexcept
{
	_sbStr.mStr[0] = CL_STRING_TERMINATOR;
}

CL::String::String(char C) noexcept
{
	_sbStr.mStr[0] = C;
	_sbStr.mStr[1] = CL_STRING_TERMINATOR;
	_nLength = 1;
}

CL::String::String(const char* pStr, size_t NumCharacters) :
	_nLength(NumCharacters)
{
	if (IsSmallBufferUsed())
	{
		memcpy(_sbStr.mStr, pStr, _nLength);
		_sbStr.mStr[_nLength] = CL_STRING_TERMINATOR;
	}
	else
	{
		_heapStr._nCapacity = CL_STRING_CAPACITY(_nLength);
		_heapStr._pStr = (char*)CL_MALLOC(_heapStr._nCapacity);
		memcpy(_heapStr._pStr, pStr, _nLength);
		_heapStr._pStr[_nLength] = CL_STRING_TERMINATOR;
	}
}

CL::String::String(const char* pStr)
{
	_nLength = strlen(pStr);

	if (IsSmallBufferUsed())
	{
		memcpy(_sbStr.mStr, pStr, _nLength + 1);
	}
	else
	{
		_heapStr._nCapacity = CL_STRING_CAPACITY(_nLength);
		_heapStr._pStr = (char*)CL_MALLOC(_heapStr._nCapacity);
		memcpy(_heapStr._pStr, pStr, _nLength + 1);
	}
}

CL::String::String(const String& otherStr)
{
	_nLength = otherStr._nLength;

	if (IsSmallBufferUsed())
	{
		memcpy(_sbStr.mStr, otherStr._sbStr.mStr, _nLength + 1);
	}
	else
	{
		_heapStr._nCapacity = otherStr._heapStr._nCapacity;
		_heapStr._pStr = (char*)CL_MALLOC(_heapStr._nCapacity);
		memcpy(_heapStr._pStr, otherStr._heapStr._pStr, _nLength + 1);
	}
}

CL::String::String(String&& otherStr) noexcept
{
	_nLength = otherStr._nLength;

	if (IsSmallBufferUsed())
	{
		memcpy(_sbStr.mStr, otherStr._sbStr.mStr, _nLength + 1);
	}
	else
	{
		_heapStr._nCapacity = otherStr._heapStr._nCapacity;
		_heapStr._pStr = otherStr._heapStr._pStr;

		otherStr._nLength = 0;
		otherStr._heapStr._nCapacity = 0;
		otherStr._heapStr._pStr = nullptr;
	}
}

void CL::String::Resize(size_t NewSize, bool bDoCopy, bool bFill, char FillSymbol)
{
	size_t OldLength = _nLength;

	if (NewSize < GetCapacity())
	{
		_nLength = NewSize;
		GetData()[_nLength] = CL_STRING_TERMINATOR;

		if (bFill)
		{
			int i = 0x0000000;
			i |= int(FillSymbol) << 24;
			i |= int(FillSymbol) << 16;
			i |= int(FillSymbol) << 8;
			i |= int(FillSymbol);

			memset(GetData(), i, NewSize);
		}
	}
	else
	{
		size_t NewCapacity = CL_STRING_CAPACITY(NewSize + 1);

		char* pNewBuffer = (char*)CL_MALLOC(NewCapacity);

		if (bDoCopy)
		{
			memcpy(pNewBuffer, GetData(), _nLength);

			if (bFill)
			{
				int i = 0x0000000;
				i |= int(FillSymbol) << 24;
				i |= int(FillSymbol) << 16;
				i |= int(FillSymbol) << 8;
				i |= int(FillSymbol);

				memset(&pNewBuffer[_nLength], i, NewSize - _nLength);
			}
		}
		else if (bFill)
		{
			int i = 0x0000000;
			i |= int(FillSymbol) << 24;
			i |= int(FillSymbol) << 16;
			i |= int(FillSymbol) << 8;
			i |= int(FillSymbol);

			memset(pNewBuffer, i, NewSize);
		}

		pNewBuffer[NewSize] = CL_STRING_TERMINATOR;

		if (!IsSmallBufferUsed())
		{
			CL_FREE(_heapStr._pStr);
		}

		_heapStr._pStr = pNewBuffer;
		_heapStr._nCapacity = NewCapacity;
		_nLength = NewSize;
	}
}

size_t CL::String::FindFirst(const char* pStrToSearch, size_t Offset) const
{
	if (Offset < _nLength && pStrToSearch)
	{
		const char* pSearchSource = pStrToSearch;
		const char* pTarget = GetData() + Offset;
		const char* pLast = GetData() + _nLength;

		while (pTarget != pLast)
		{
			const char* pLocalCmpString = pTarget;

			while (*pLocalCmpString == *pSearchSource && pLocalCmpString != pLast)
			{
				pSearchSource++;
				pLocalCmpString++;

				if (*pSearchSource == CL_STRING_TERMINATOR)
				{
					return pTarget - GetData();
				}
			}

			pSearchSource = pStrToSearch;
			pTarget++;
		}
	}

	return NullPos();
}

size_t CL::String::FindFirst(const CL::String& StrToSearch, size_t Offset) const
{
	if (Offset < _nLength && !StrToSearch.IsEmpty())
	{
		const char* pStrToSearch = StrToSearch.CStr();
		const char* pSearchSource = pStrToSearch;
		const char* pTarget = GetData() + Offset;
		const char* pLast = GetData() + _nLength;

		while (pTarget != pLast)
		{
			const char* pLocalCmpString = pTarget;

			while (*pLocalCmpString == *pSearchSource && pLocalCmpString != pLast)
			{
				pSearchSource++;
				pLocalCmpString++;

				if (*pSearchSource == CL_STRING_TERMINATOR)
				{
					return pTarget - GetData();
				}
			}

			pSearchSource = pStrToSearch;
			pTarget++;
		}
	}

	return NullPos();
}

size_t CL::String::FindLast(const char* pStrToSearch, size_t Offset) const
{
	if (Offset < _nLength && pStrToSearch)
	{
		const char* pSearchSource = pStrToSearch;
		const char* pTarget = GetData() + _nLength - Offset - 1;
		const char* pLast = GetData();

		while (pTarget != pLast)
		{
			const char* pLocalCmpString = pTarget;

			while (*pLocalCmpString == *pSearchSource && pLocalCmpString != pLast)
			{
				pSearchSource++;
				pLocalCmpString++;

				if (*pSearchSource == CL_STRING_TERMINATOR)
				{
					return pTarget - GetData();
				}
			}

			pSearchSource = pStrToSearch;
			pTarget--;
		}
	}

	return NullPos();
}

size_t CL::String::FindLast(const CL::String& StrToSearch, size_t Offset) const
{
	if (Offset < _nLength && !StrToSearch.IsEmpty())
	{
		const char* pStrToSearch = StrToSearch.CStr();
		const char* pSearchSource = pStrToSearch;
		const char* pTarget = GetData() + _nLength - Offset - 1;
		const char* pLast = GetData();

		while (pTarget != pLast)
		{
			const char* pLocalCmpString = pTarget;

			while (*pLocalCmpString == *pSearchSource && pLocalCmpString != pLast)
			{
				pSearchSource++;
				pLocalCmpString++;

				if (*pSearchSource == CL_STRING_TERMINATOR)
				{
					return pTarget - GetData();
				}
			}

			pSearchSource = pStrToSearch;
			pTarget--;
		}
	}

	return NullPos();
}

size_t CL::String::FindLast(char SimbolToFind, size_t Offset) const
{
	if (Offset < _nLength)
	{
		const char* pFirst = GetData();
		const char* pTarget = pFirst + _nLength - Offset - 1;

		if (pFirst > pTarget)
		{
			return NullPos();
		}

		while (pTarget != pFirst)
		{
			if (*pTarget == SimbolToFind)
			{
				return pTarget - GetData();
			}

			pTarget--;
		}
	}

	return NullPos();
}

size_t CL::String::FindFirst(char c, size_t Offset) const
{
	if (Offset < _nLength)
	{
		const char* pTarget = GetData() + Offset;

		while (*pTarget)
		{
			if (*pTarget == c)
			{
				return pTarget - GetData();
			}

			pTarget++;
		}
	}

	return NullPos();
}

CL::String CL::String::Substring(size_t Position, size_t FragmentLength) const noexcept
{
	String Str;

	if (Position < _nLength)
	{
		Str = String(GetData() + Position, CL_MIN(FragmentLength, _nLength - Position));
	}

	return Str;
}

void CL::String::Clear()
{
	ClearHeapFromNewSize(0);
	_sbStr.mStr[0] = CL_STRING_TERMINATOR;
	_nLength = 0;
}

void CL::String::clear()
{
	Clear();
}

CL::String::~String()
{
	ClearHeapFromNewSize(0);
}

void CL::String::ClearHeapFromNewSize(size_t NewLength) noexcept
{
	if (IsSmallBufferUsed())
	{
		return;
	}

	if (NewLength < sizeof(_sbStr) || NewLength >= _heapStr._nCapacity)
	{
		CL_FREE(_heapStr._pStr);
		_heapStr._pStr = nullptr;
	}
}
