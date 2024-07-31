#pragma once
#include <CLAssert.h>
#include <CLMemory.h>

#ifndef CL_STR_BIN_ALIGNMENT
#define CL_STR_BIN_ALIGNMENT 4
#endif

#define CL_STRING_TERMINATOR '\0'
#define CL_STRING_CAPACITY(Length) CL_ALIGN_BIN_2((Length) + 1, CL_STR_BIN_ALIGNMENT)

namespace CL
{
	struct StringData
	{
		char* _pStr = nullptr;
		size_t _nCapacity = 0;
		size_t stride;
	};

	struct SmallStringBuffer
	{
		char mStr[sizeof(StringData)];
	};

// #TODO conversion to char* operator

	class String final
	{
		friend static String operator+(const String& lefth, const String& right);
		friend static String operator+(String&& left, String&& right);
		friend String ToString(int n) noexcept;
		friend String ToString(unsigned int n) noexcept;
	public:
		String() noexcept;
		String(char C) noexcept;
		String& operator = (char C)
		{
			Clear();

			_sbStr.mStr[0] = C;
			_sbStr.mStr[1] = CL_STRING_TERMINATOR;
			_nLength = 1;

			return *this;
		}
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
		String(const char* pStr, size_t NumCharacters);
		String(const char* pStr);
		String& operator = (const char* pStr)
		{
			size_t Length = strlen(pStr);
			ClearHeapFromNewSize(Length);
			_nLength = Length;

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

			return *this;
		}
		String(const String& otherStr);
		String& operator = (const String& otherStr)
		{
			ClearHeapFromNewSize(otherStr._nLength);
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

			return *this;
		}
		String(String&& otherStr) noexcept;
		String& operator = (String&& otherStr) noexcept
		{
			ClearHeapFromNewSize(otherStr._nLength);
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

			return *this;
		}
		String& operator += (String&& sourceStr)
		{
			if (!sourceStr._nLength)
			{
				return *this;
			}

			size_t NewLength = _nLength + sourceStr.GetLength();

			if (NewLength < sizeof(_sbStr))
			{
				memcpy(&_sbStr.mStr[_nLength], sourceStr.GetData(), sourceStr._nLength + 1);
			}
			else
			{
				size_t NewCapacity = CL_STRING_CAPACITY(NewLength);

				if (GetCapacity() < NewCapacity)
				{
					if (sourceStr.GetCapacity() >= NewCapacity)
					{
						memcpy(&sourceStr._heapStr._pStr[_nLength], sourceStr._heapStr._pStr, sourceStr.GetLength() + 1);
						memcpy(sourceStr._heapStr._pStr, GetData(), _nLength);

						if (!IsSmallBufferUsed())
						{
							CL_FREE(_heapStr._pStr);
						}

						_heapStr = sourceStr._heapStr;

						sourceStr._heapStr._nCapacity = 0;
						sourceStr._heapStr._pStr = 0;
						sourceStr._nLength = 0;
					}
					else
					{
						char* pNewBuffer = (char*)CL_MALLOC(NewCapacity);
						memcpy(pNewBuffer, GetData(), _nLength);
						memcpy(&pNewBuffer[_nLength], sourceStr.GetData(), sourceStr.GetLength() + 1);

						if (!IsSmallBufferUsed())
						{
							CL_FREE(_heapStr._pStr);
						}

						_heapStr._pStr = pNewBuffer;
						_heapStr._nCapacity = NewCapacity;
					}
				}
				else
				{
					memcpy(&_heapStr._pStr[_nLength], sourceStr.GetData(), sourceStr.GetLength() + 1);
				}
			}

			_nLength = NewLength;

			return *this;
		}
		String& operator += (const String& sourceStr)
		{
			if (!sourceStr._nLength)
			{
				return *this;
			}

			size_t NewLength = _nLength + sourceStr.GetLength();

			if (NewLength < sizeof(_sbStr))
			{
				memcpy(&_sbStr.mStr[_nLength], sourceStr.GetData(), sourceStr._nLength + 1);
			}
			else
			{
				size_t NewCapacity = CL_STRING_CAPACITY(NewLength);

				if (GetCapacity() < NewCapacity)
				{
					char* pNewBuffer = (char*)CL_MALLOC(NewCapacity);
					memcpy(pNewBuffer, GetData(), _nLength);
					memcpy(&pNewBuffer[_nLength], sourceStr.GetData(), sourceStr.GetLength() + 1);

					if (!IsSmallBufferUsed())
					{
						CL_FREE(_heapStr._pStr);
					}

					_heapStr._pStr = pNewBuffer;
					_heapStr._nCapacity = NewCapacity;
				}
				else
				{
					memcpy(&_heapStr._pStr[_nLength], sourceStr.GetData(), sourceStr.GetLength() + 1);
				}
			}

			_nLength = NewLength;

			return *this;
		}
		String& operator += (const char* SourceStr)
		{
			if (!SourceStr)
			{
				return *this;
			}

			size_t SourceLength = strlen(SourceStr);
			size_t NewLength = _nLength + SourceLength;

			if (NewLength < sizeof(_sbStr))
			{
				memcpy(&_sbStr.mStr[_nLength], SourceStr, SourceLength + 1);
			}
			else
			{
				size_t NewCapacity = CL_STRING_CAPACITY(NewLength);

				if (GetCapacity() < NewCapacity)
				{
					char* pNewBuffer = (char*)CL_MALLOC(NewCapacity);
					memcpy(pNewBuffer, GetData(), _nLength);
					memcpy(&pNewBuffer[_nLength], SourceStr, SourceLength + 1);

					if (!IsSmallBufferUsed())
					{
						CL_FREE(_heapStr._pStr);
					}

					_heapStr._pStr = pNewBuffer;
					_heapStr._nCapacity = NewCapacity;
				}
				else
				{
					memcpy(&_heapStr._pStr[_nLength], SourceStr, SourceLength + 1);
				}
			}

			_nLength = NewLength;

			return *this;
		}
		String& operator += (char C)
		{
			size_t SourceLength = 1;
			size_t NewLength = _nLength + SourceLength;

			if (NewLength < sizeof(_sbStr))
			{
				_sbStr.mStr[_nLength] = C;
				_sbStr.mStr[_nLength + 1] = CL_STRING_TERMINATOR;
			}
			else
			{
				size_t NewCapacity = CL_STRING_CAPACITY(NewLength);

				if (GetCapacity() < NewCapacity)
				{
					char* pNewBuffer = (char*)CL_MALLOC(NewCapacity);
					memcpy(pNewBuffer, GetData(), _nLength);
					pNewBuffer[_nLength] = C;
					pNewBuffer[_nLength + 1] = CL_STRING_TERMINATOR;

					if (!IsSmallBufferUsed())
					{
						CL_FREE(_heapStr._pStr);
					}

					_heapStr._pStr = pNewBuffer;
					_heapStr._nCapacity = NewCapacity;
				}
				else
				{
					_heapStr._pStr[_nLength] = C;
					_heapStr._pStr[_nLength + 1] = CL_STRING_TERMINATOR;
				}
			}

			_nLength = NewLength;

			return *this;
		}
		bool operator == (char C) const
		{
			return _nLength == 1 ? _sbStr.mStr[0] == C : false;
		}
		bool operator != (char C) const
		{
			return _nLength == 1 ? _sbStr.mStr[0] != C : true;
		}
		bool operator == (const char* pStr) const
		{
			size_t StrLength = strlen(pStr);
			return StrLength == _nLength ? memcmp(pStr, GetData(), _nLength) == 0: false;
		}
		bool operator != (const char* pStr) const
		{
			size_t StrLength = strlen(pStr);
			return StrLength == _nLength ? memcmp(pStr, GetData(), _nLength) != 0 : true;
		}
		bool operator == (const String& str) const
		{
			return str._nLength == _nLength ? memcmp(str.GetData(), GetData(), _nLength) == 0 : false;
		}
		bool operator != (const String& str) const
		{
			return str._nLength != _nLength ? true : memcmp(str.GetData(), GetData(), _nLength) != 0;
		}
		char operator[](size_t Index) const
		{
			CL_DEBUG_ASSERT(Index < _nLength);
			return IsSmallBufferUsed() ? _sbStr.mStr[Index] : _heapStr._pStr[Index];
		}
		char& operator[](size_t Index)
		{
			CL_DEBUG_ASSERT(Index < _nLength);
			return IsSmallBufferUsed() ? _sbStr.mStr[Index] : _heapStr._pStr[Index];
		}
		void Resize(size_t NewSize, bool bDoCopy = true, bool bFill = true, char FillSymbol = ' ');
		size_t FindFirst(const char* pStrToSearch, size_t Offset = 0) const;
		size_t FindFirst(const CL::String& StrToSearch, size_t Offset = 0) const;
		size_t FindLast(const char* pStrToSearch, size_t Offset = 0) const;
		size_t FindLast(const CL::String& StrToSearch, size_t Offset = 0) const;
		size_t FindLast(char SimbolToFind, size_t Offset = 0) const;
		size_t FindFirst(char c, size_t Offset = 0) const;
		String Substring(size_t Position, size_t FragmentLength = NullPos()) const noexcept;
		bool IsSmallBufferUsed() const noexcept { return _nLength < sizeof(_sbStr); }
		char* GetData() noexcept { return IsSmallBufferUsed() ? _sbStr.mStr : _heapStr._pStr; }
		const char* GetData() const noexcept { return IsSmallBufferUsed() ? _sbStr.mStr : _heapStr._pStr; }
		char* CStr() noexcept { return IsSmallBufferUsed() ? _sbStr.mStr : _heapStr._pStr; }
		const char* CStr() const noexcept { return IsSmallBufferUsed() ? _sbStr.mStr : _heapStr._pStr; }
		size_t GetLength() const noexcept { return _nLength; }
		size_t GetCapacity() const noexcept { return IsSmallBufferUsed() ? sizeof(_sbStr) : _heapStr._nCapacity; }
		void Clear();
		void clear();
		bool IsEmpty() const { return _nLength == 0; }
		constexpr static size_t NullPos() { return SIZE_MAX; }
		~String();
	private:
		__forceinline void ClearHeapFromNewSize(size_t NewLength) noexcept;

		size_t _nLength = 0;

		union
		{
			StringData _heapStr;
			SmallStringBuffer _sbStr;
		};
	};

	static String operator+(const String& left, const String& right)
	{
		if (!left._nLength)
		{
			return right;
		}

		if (!right._nLength)
		{
			return left;
		}

		String str;
		str._nLength = left._nLength + right._nLength;

		if (str._nLength < sizeof(str._sbStr))
		{
			memcpy(str._sbStr.mStr, left.GetData(), left._nLength);
			memcpy(&str._sbStr.mStr[left._nLength], right.GetData(), right._nLength);
			str._sbStr.mStr[str._nLength] = CL_STRING_TERMINATOR;
		}
		else
		{
			size_t NewCapacity = CL_STRING_CAPACITY(str._nLength);
			str._heapStr._nCapacity = NewCapacity;
			str._heapStr._pStr = (char*)CL_MALLOC(NewCapacity);
			memcpy(str._heapStr._pStr, left.GetData(), left._nLength);
			memcpy(&str._heapStr._pStr[left._nLength], right.GetData(), right._nLength);
			str._heapStr._pStr[str._nLength] = CL_STRING_TERMINATOR;
		}

		return str;
	}

	static String operator+(String&& left, String&& right)
	{
		left += right;
		return left;
	}
}

static std::ostream& operator << (std::ostream& os, const CL::String& Str)
{
	if (!Str.IsEmpty())
	{
		os << Str.CStr();
	}

	return os;
}