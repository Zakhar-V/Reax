#pragma once

#include "Container.hpp"

namespace Reax
{
	//!\addtogroup String
	//!\{

	//----------------------------------------------------------------------------//
	// String
	//----------------------------------------------------------------------------//

	//! Multibyte string.
	class String
	{
	public:
		//!
		typedef ArrayIterator<char> Iterator;
		//!
		typedef ArrayIterator<const char> ConstIterator;

		//!
		String(void) = default;
		//!
		~String(void) { if (m_data) Deallocate(m_data); }
		//!
		String(const String& _other) { Append(_other); }
		//!
		String(String&& _temp) : m_length(_temp.m_length), m_capacity(_temp.m_capacity), m_data(_temp.m_data)
		{
			_temp.m_length = 0;
			_temp.m_capacity = 0;
			_temp.m_data = nullptr;
		}
		//!
		String(const char* _str, int _length = -1) { Append(_str, _length); }
		//!
		String(const char* _start, const char* _end) { Append(_start, _end); }
		//!
		String(char _ch) { Append(_ch); }
		//!
		String(uint _count, char _ch) { Append(_count, _ch); }
		//!
		String(const char* _str1, int _length1, const char* _str2, int _length2);

		//!
		String& operator = (const String& _rhs);
		//!
		String& operator = (String&& _rhs);
		//!
		String& operator = (const char* _str);
		//!
		String& operator = (char _ch) { return Clear().Append(_ch); }

		//!
		char& operator [] (int _index) { ASSERT((uint)_index < m_length); return m_data[_index]; }
		//!
		char operator [] (int _index) const { ASSERT((uint)_index < m_length); return m_data[_index]; }
		//!
		char& At(uint _index) { ASSERT(_index < m_length); return m_data[_index]; }
		//!
		char At(uint _index) const { ASSERT(_index < m_length); return m_data[_index]; }

		//!
		operator const char* (void) const { return m_data; }
		//!
		char* Data(uint _offset = 0) { ASSERT(_offset <= m_length); return m_data + _offset; }
		//!
		const char* CStr(uint _offset = 0) const { ASSERT(_offset <= m_length); return m_data + _offset; }

		//!
		Iterator Begin(void) { return m_data; }
		//!
		ConstIterator Begin(void) const { return m_data; }
		//!
		Iterator End(void) { return m_data + m_length; }
		//!
		ConstIterator End(void) const { return m_data + m_length; }


		//!
		bool IsEmpty(void) const { return m_length == 0; }
		//!
		bool NonEmpty(void) const { return m_length != 0; }
		//!
		uint Length(void) const { return m_length; }
		//!
		uint Size(void) const { return m_length; }
		//!
		uint Capacity(void) const { return m_capacity; }
		//!
		String& Reserve(uint _maxLength);
		//!
		String& Resize(uint _newLength, char _ch = 0);
		//!
		String& Clear(void) { return Resize(0); }


		//! 
		String& operator += (const String& _rhs) { return Append(_rhs); }
		//! 
		String& operator += (const char* _rhs) { return Append(_rhs); }
		//! 
		String& operator += (char _rhs) { return Append(_rhs); }
		//! 
		String operator + (const String& _rhs) const { return String(m_data, m_length, _rhs.m_data, _rhs.m_length); }
		//! 
		String operator + (const char* _rhs) const { return String(m_data, m_length, _rhs, -1); }
		//! 
		String operator + (char _rhs) const { return String(m_data, m_length, &_rhs, 1); }
		//!
		friend String operator + (const char* _lhs, const String& _rhs) { return String(_lhs, -1, _rhs.m_data, _rhs.m_length); }
		//!
		friend String operator + (char _lhs, const String& _rhs) { return String(&_lhs, 1, _rhs.m_data, _rhs.m_length); }

		//!
		String& Append(const String& _str) { return Append(_str.m_data, _str.m_length); }
		//!
		String& Append(const char* _str, int _length = -1);
		//!
		String& Append(const char* _start, const char* _end) { return Append(_start, (uint)(_end - _start)); }
		//!
		String& Append(char _ch);
		//!
		String& Append(uint _count, char _ch);

		//!
		bool operator == (const String& _rhs) const { return m_length == _rhs.m_length && Compare(m_data, _rhs.m_data) == 0; }
		//!
		bool operator == (const char* _rhs) const { return Compare(m_data, _rhs) == 0; }
		//!
		bool operator != (const String& _rhs) const { return !(*this == _rhs); }
		//!
		bool operator != (const char* _rhs) const { return !(*this == _rhs); }
		//!
		bool operator < (const char* _rhs) const { return Compare(m_data, _rhs) < 0; }
		//!
		bool operator <= (const char* _rhs) const { return Compare(m_data, _rhs) <= 0; }
		//!
		bool operator > (const char* _rhs) const { return Compare(m_data, _rhs) > 0; }
		//!
		bool operator >= (const char* _rhs) const { return Compare(m_data, _rhs) >= 0; }

		//!
		String SubStr(uint _offset, int _length = -1) const;
		//!
		String Copy(void) const { return *this; }
		//!
		String& MakeLower(void) { Lower(m_data, m_length); return *this; }
		//!
		String& MakeUpper(void) { Upper(m_data, m_length); return *this; }
		//!
		String Lower(void) const { Copy().MakeLower(); }
		//!
		String Upper(void) const { Copy().MakeUpper(); }
		//!
		uint Hash(uint _hash = 0) const { return Hash(m_data, _hash); }
		//!
		uint IHash(uint _hash = 0) const { return IHash(m_data, _hash); }


		//!
		static uint Length(const char* _str, int _length = -1) { return _str ? (uint)(_length < 0 ? strlen(_str) : _length) : 0; }

		//!
		static constexpr bool IsAlpha(char _ch) { return (_ch >= 'A' && _ch <= 'Z') || (_ch >= 'a' && _ch <= 'z') || ((uint8)_ch >= 0xc0); }
		//!
		static constexpr char Lower(char _ch) { return IsAlpha(_ch) ? (_ch | 0x20) : _ch; }
		//!
		static constexpr char Upper(char _ch) { return IsAlpha(_ch) ? (_ch & ~0x20) : _ch; }
		//!
		static constexpr uint ConstHash(const char* _str, uint _hash = 0) { return *_str ? ConstHash(_str + 1, *_str + (_hash << 6) + (_hash << 16) - _hash) : _hash; }
		//!
		static constexpr uint ConstIHash(const char* _str, uint _hash = 0) { return *_str ? ConstIHash(_str + 1, Lower(*_str) + (_hash << 6) + (_hash << 16) - _hash) : _hash; }

		//!
		static uint Hash(const char* _str, uint _hash = 0);
		//!
		static uint IHash(const char* _str, uint _hash = 0);
		//!
		static char* Lower(char* _str, int _length = -1);
		//!
		static char* Upper(char* _str, int _length = -1);

		//!
		static int Compare(const char* _lhs, const char* _rhs, bool _ignoreCase = false);

		//!
		static String Format(const char* _fmt, ...);
		//!
		static String FormatV(const char* _fmt, va_list _args);

		//!
		static bool Match(const char* _str, const char* _pattern, bool _ignoreCase = true);
		//!
		static const char* Find(const char* _str1, const char* _str2, bool _ignoreCase = false);
		//!
		static char* Find(char* _str1, const char* _str2, bool _ignoreCase = false);
		//!
		static void Split(const char* _str, const char* _delimiters, Array<String>& _dst);

		static const String Empty;

	private:
		const char* operator * (void) const = delete;
		template <class T> String operator - (const T&) = delete;
		template <class T> friend String operator - (const T&, const String&) = delete;

	protected:
		uint m_length = 0;
		uint m_capacity = 0;
		char* m_data = nullptr;
	};

	//!
	inline uint MakeHash(const String& _value) { return _value.Hash(); }

	//!
	inline auto begin(String& _str)->decltype(_str.Begin()) { return _str.Begin(); }
	//!
	inline auto begin(const String& _str)->decltype(_str.Begin()) { return _str.Begin(); }
	//!
	inline auto end(String& _str)->decltype(_str.End()) { return _str.End(); }
	//!
	inline auto end(const String& _str)->decltype(_str.End()) { return _str.End(); }

	//----------------------------------------------------------------------------//
	// WString
	//----------------------------------------------------------------------------//

	//! Unicode string.
	class RX_API WString
	{
	public:


	protected:
		uint m_length = 0;
		uint m_capacity = 0;
		wchar_t* m_data = nullptr;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	//!\} String
}