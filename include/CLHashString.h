#pragma once
#include <CLString.h>
#include <CLCommon.h>
#include <iostream>

namespace CL
{
	constexpr size_t StringHash(const char* pStr);

	class HashString final 
	{
		friend class HashStringView;
	public:
		HashString();
		HashString(const char* str);
		HashString(const String& str);
		HashString(String&& str);
		HashString(const HashString& hashStr);
		HashString(HashString&& hashStr);
		HashString& operator = (const String& str)
		{
			_hash = StringHash(str.GetData());
			_str = str;
			return *this;
		}
		HashString& operator = (String&& str)
		{
			_hash = StringHash(str.GetData());
			_str = CL::Move(str);
			return *this;
		}
		HashString& operator = (const HashString& str)
		{
			_hash = StringHash(str._str.GetData());
			_str = str._str;
			return *this;
		}
		HashString& operator = (HashString&& str)
		{
			_hash = StringHash(str._str.GetData());
			_str = CL::Move(str._str);
			return *this;
		}
		HashString& operator = (const char* str)
		{
			_str = str;
			_hash = StringHash(str);
			return *this;
		}
		const String& GetString() const { return _str; }
		size_t GetHash() const { return _hash; }
		bool IsValid() const { return _str.GetLength(); }
		bool operator == (const HashString& other) const
		{
			return _hash == other._hash && _str == other._str;
		}
		bool operator != (const HashString& other) const
		{
			return _hash != other._hash || _str != other._str;
		}
	private:
		String _str;
		size_t _hash;
	};

	class HashStringView final
	{
	public:
		HashStringView();
		HashStringView(const HashString* pHashString);
		HashStringView(const HashString& HashString);
		bool IsValid() const { return _pHashString && _pHashString->IsValid(); }
		size_t GetHash() const
		{
			CL_DEBUG_ASSERT(_pHashString);
			return _pHashString->GetHash();
		}
		bool operator == (const HashString& Other) const
		{
			CL_DEBUG_ASSERT(_pHashString);
			return _pHashString->_hash == Other._hash && _pHashString->_str == Other._str;
		}
		bool operator == (const HashStringView& Other) const
		{
			CL_DEBUG_ASSERT(_pHashString);
			CL_DEBUG_ASSERT(Other._pHashString);
			return _pHashString->_hash == Other._pHashString->_hash && _pHashString->_str == Other._pHashString->_str;
		}
	private:
		const HashString* _pHashString;
	};
}

template<>
struct std::hash<CL::HashString>
{
	std::size_t operator()(const CL::HashString& key) const
	{
		return key.GetHash();
	}
};

template<>
struct std::hash<CL::HashStringView>
{
	std::size_t operator()(const CL::HashStringView& key) const
	{
		return key.GetHash();
	}
};

static std::ostream& operator << (std::ostream& os, const CL::HashString& hs)
{
	for (size_t i = 0; i < hs.GetString().GetLength(); i++)
	{
		os << hs.GetString()[i];
	}

	return os;
}

