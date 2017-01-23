#pragma once

//----------------------------------------------------------------------------//
// Preprocessor, Compiler and Platform
//----------------------------------------------------------------------------//

#define _QUOTE( x ) #x
#define _QUOTE_IN_PLACE( x ) _QUOTE( x )
#define __FILELINE__ __FILE__"(" _QUOTE_IN_PLACE(__LINE__) ")"

#ifdef _MSC_VER
#	define _PRAGMA(x) __pragma(x)
#	define _PRAGMA_MESSAGE(x) _PRAGMA(message(__FILELINE__ " : " x))
#   define DEPRECATED __declspec(deprecated)
#   define PACK __pragma(pack( push, 1))
#   define PACKED
#   define UNPACK ;__pragma(pack(pop))
#	define THREAD_LOCAL __declspec(thread)
#	define NOINLINE __declspec(noinline)
#   ifndef _CRT_SECURE_NO_WARNINGS
#       define _CRT_SECURE_NO_WARNINGS // CRT unsafe
#   endif
#   ifndef _CRT_NONSTDC_NO_WARNINGS
#       define _CRT_NONSTDC_NO_WARNINGS // The POSIX name
#   endif
#	pragma warning(disable : 4251) // dll interface
#	pragma warning(disable : 4275) // dll interface
#	pragma warning(disable : 4201) // unnamed union
#	pragma warning(disable : 4100) // unused arg
//#	pragma warning(disable : 4244) // loss of data
//#	pragma warning(disable : 4996)	// deprecated

#elif defined(__GNUC__)
#	define __FUNCTION__ __func__ //!<\note in GCC it's local variable. Cannot be concatenated with constant string.
#	define _PRAGMA(x) _Pragma(#x)
#	define _PRAGMA_MESSAGE(x) _PRAGMA(message(_QUOTE(x)))
#   define DEPRECATED __attribute__((deprecated))
#   define PACK
#   define PACKED __attribute__((packed))
#   define UNPACK
#   define THREAD_LOCAL __thread
#	define NOINLINE __attribute__((noinline))
#	define abstract =0
#else
#	warning "unknown compiler"
#	define _PRAGMA(x)
#	define _PRAGMA_MESSAGE(x)
#   define DEPRECATED
#   define PACK
#   define PACKED
#   define UNPACK
#   define THREAD_LOCAL
#	define NOINLINE
#endif

#define COMPILER_MESSAGE(_prefix, _message) _PRAGMA_MESSAGE(_prefix ": " _message )
#define COMPILER_MESSAGE_EX(_prefix, _source, _message) COMPILER_MESSAGE(_prefix, _source " : " _message)
#define WARNING_EX(_source, _message) COMPILER_MESSAGE_EX("Warning", _source, _message)
#define WARNING(_message) WARNING_EX(__FUNCTION__, _message)
#define FIXME_EX(_source, _message) COMPILER_MESSAGE_EX("FixMe", _source, _message)
#define FIXME(_message) FIXME_EX(__FUNCTION__, _message)
#define TODO_EX(_source, _message) COMPILER_MESSAGE_EX("ToDo", _source, _message)
#define TODO(_message) TODO_EX(__FUNCTION__, _message)
#define NOT_IMPLEMENTED_YET() FIXME("Not implemented yet")
#define NOT_IMPLEMENTED_YET_EX(_source) FIXME_EX(_source, "Not implemented yet")

//----------------------------------------------------------------------------//
// Settings
//----------------------------------------------------------------------------//

#define RX_STATIC_LIB // define it if you use the engine as static library.

//----------------------------------------------------------------------------//
// Import/Export
//----------------------------------------------------------------------------//

#ifdef RX_EXPORT_SYMBOLS
#	define RX_API __declspec(dllexport)
#elif !defined(RX_STATIC_LIB)
#	define RX_API __declspec(dllimport)
#else
#	define RX_API
#endif

//----------------------------------------------------------------------------//
// Includes
//----------------------------------------------------------------------------//

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <time.h>
#include <new>
#include <atomic>

namespace Reax
{
	//----------------------------------------------------------------------------//
	// Types
	//----------------------------------------------------------------------------//

	typedef int8_t int8;
	typedef uint8_t uint8;
	typedef int16_t int16;
	typedef uint16_t uint16;
	typedef int32_t int32;
	typedef uint32_t uint32;
	typedef int64_t int64;
	typedef uint64_t uint64;
	typedef unsigned int uint;

	//----------------------------------------------------------------------------//
	// Assert
	//----------------------------------------------------------------------------//

#ifdef _DEBUG
#	define ASSERT(cond, ...) (cond) ? (void)0 : Reax::Assert("Assertion failed", __FUNCTION__, __FILE__, __LINE__, #cond, ##__VA_ARGS__)
#else
#	define ASSERT(cond, ...)
#endif

	RX_API void Assert(const char* _type, const char* _func, const char* _file, int _line, const char* _desc, const char* _msg = nullptr, ...);

	//----------------------------------------------------------------------------//
	// Move semantics
	//----------------------------------------------------------------------------//

	template <class T> struct BaseType { typedef T Type; };
	template <class T> struct BaseType<T&> { typedef T Type; };
	template <class T> struct BaseType<T&&> { typedef T Type; };

	//! Get a rvalue reference.
	template <class T> inline constexpr typename T&& Forward(typename BaseType<T>::Type& _ref) { return static_cast<T&&>(_ref); }
	//! Get a rvalue reference.
	template <class T> inline constexpr typename T&& Forward(typename BaseType<T>::Type&& _ref) { return static_cast<T&&>(_ref); }
	//! Get a rvalue reference.
	template <class T> inline typename BaseType<T>::Type&& Move(T&& _ref) { return static_cast<BaseType<T>::Type&&>(_ref); }

	//! Swap elements.
	template <class T> void Swap(T& _a, T& _b)
	{
		T _c = Move(_a);
		_a = Move(_b);
		_b = Move(_c);
	}

	//! Destroy element.
	template <class T> void Destroy(T* _dst)
	{
		_dst->~T();
	}

	//! Destroy elements.
	template <class T> T* DestroyRange(T* _dst, T* _end)
	{
		for (; _dst < _end; ++_dst)
			Destroy(_dst++);
		return _dst;
	}

	//! Construct element.
	template <class T, class... Args> static void Construct(T* _dst, Args&&... _args)
	{
		new(_dst) T(Forward<Args>(_args)...);
	}

	//! Construct elements.
	template <class T, class... Args> static T* ConstructRange(T* _dst, T* _end, Args&&... _args)
	{
		while (_dst < _end)
			new(_dst++) T(_args...);
		return _dst;
	}

	//! Move elements to uninitialized memory and destroy old elements.
	template <class T> T* MoveAndDestroyRange(T* _dst, T* _src, T* _end)
	{
		for (T* src = _src; _src < _end;)
			Construct(_dst++, Move(*src++));
		while (_src < _end)
			Destroy(_src++);
		return _dst;
	}

	//! Move elements to uninitialized memory.
	template <class T> T* MoveRange(T* _dst, T* _src, T* _end)
	{
		while (_src < _end)
			Construct(_dst++, Move(*_src++));
		return _dst;
	}

	//! Copy elements to uninitialized memory.
	template <class T> T* CopyRange(T* _dst, const T* _src, const T* _end)
	{
		while (_src < _end)
			Construct(_dst++, *_src++);
		return _dst;
	}

	//! Copy value of elements.
	template <class T> T* AssignRange(T* _dst, const T* _src, const T* _end)
	{
		while (_src < _end)
			*_dst++ = *_src++;
		return _dst;
	}

	//! Move value of elements.
	template <class T> T* AssignMoveRange(T* _dst, T* _src, T* _end)
	{
		while (_src < _end)
			*_dst++ = Move(*_src++);
		return _dst;
	}

	//----------------------------------------------------------------------------//
	// Allocator utilities
	//----------------------------------------------------------------------------//

	//! Get new memory block for element.
	template <class T> T* AllocateBlock(uint _size) { return reinterpret_cast<T*>(new uint8[_size]); }
	//! Get new memory block for elements.
	template <class T> T* Allocate(uint _count) { return reinterpret_cast<T*>(new uint8[_count * sizeof(T)]); }
	//! Delete memory block.
	template <class T> void Deallocate(T* _ptr) { delete[] reinterpret_cast<uint8*>(_ptr); }
	//!	Grow size.
	inline uint GrowTo(uint _currentSize, uint _newSize)
	{
		if (_currentSize)
		{
			size_t _size = _currentSize;
			while (_size < _newSize)
				_size += (_size + 1) >> 1;
			return _size;
		}
		return _newSize;
	}

	//----------------------------------------------------------------------------//
	// NonCopyable
	//----------------------------------------------------------------------------//

	class NonCopyable
	{
	public:
		NonCopyable(void) { }
		~NonCopyable(void) { }

	private:
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator = (const NonCopyable&) = delete;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	template <class T> class Singleton
	{

	};

	//----------------------------------------------------------------------------//
	// ArrayIterator
	//----------------------------------------------------------------------------//

	//! Iterator with random access.
	template <class T> class ArrayIterator
	{
	public:
		//!
		ArrayIterator(void) { }
		//!
		explicit ArrayIterator(T* _ptr) : m_ptr(_ptr) { }
		//!	Implicit conversion from non-const iterator to const iterator.
		operator const typename ArrayIterator<const T>(void) const { return ArrayIterator<const T>(m_ptr); }

		//!	Get address.
		T* operator & (void) { return m_ptr; }
		//!	Get address.
		T* operator -> (void) { ASSERT(m_ptr != nullptr); return m_ptr; }
		//!	Get reference.
		T& operator * (void) { ASSERT(m_ptr != nullptr); return *m_ptr; }

		//!	Go to next element.
		ArrayIterator& operator ++ (void) { ++m_ptr; return *this; }
		//!	Go to next element.
		ArrayIterator operator ++ (int) { ArrayIterator _next = *this; ++m_ptr; return _next; }
		//!	Go to prev element.
		ArrayIterator& operator -- (void) { --m_ptr; return *this; }
		//!	Go to prev element.
		ArrayIterator operator -- (int) { ArrayIterator _prev = *this; --m_ptr; return _prev; }
		//!
		ArrayIterator& operator += (ptrdiff_t _offset) { m_ptr += _offset; return *this; }
		//!
		ArrayIterator& operator -= (ptrdiff_t _offset) { m_ptr -= _offset; return *this; }

		//!
		ArrayIterator operator + (ptrdiff_t _offset) const { return ArrayIterator(m_ptr + _offset); }
		//!
		ArrayIterator operator - (ptrdiff_t _offset) const { return ArrayIterator(m_ptr - _offset); }
		//!
		ptrdiff_t operator - (const ArrayIterator& _rhs) const { return m_ptr - _rhs.m_ptr; }
		//!
		ptrdiff_t operator - (const T* _rhs) const { return m_ptr - _rhs; }

		//!
		bool operator == (const ArrayIterator& _rhs) const { return m_ptr == _rhs.m_ptr; }
		//!
		bool operator != (const ArrayIterator& _rhs) const { return m_ptr != _rhs.m_ptr; }
		//!
		bool operator < (const ArrayIterator& _rhs) const { return m_ptr < _rhs.m_ptr; }
		//!
		bool operator <= (const ArrayIterator& _rhs) const { return m_ptr <= _rhs.m_ptr; }
		//!
		bool operator > (const ArrayIterator& _rhs) const { return m_ptr > _rhs.m_ptr; }
		//!
		bool operator >= (const ArrayIterator& _rhs) const { return m_ptr >= _rhs.m_ptr; }

	protected:
		//!	Pointer.
		T* m_ptr = nullptr;
	};

	//----------------------------------------------------------------------------//
	// Array
	//----------------------------------------------------------------------------//

	//! Dynamic array.
	template <class T> class Array
	{
	public:
		typedef ArrayIterator<T> Iterator;
		typedef ArrayIterator<const T> ConstIterator;

		//!
		Array(void) = default;
		//!
		~Array(void) { Free(); }
		//!
		Array(const Array& _rhs) { Push(_rhs.m_data, _rhs.m_size); }
		//!
		Array(Array&& _rhs) :
			m_size(_rhs.m_size),
			m_caps(_rhs.m_caps),
			m_data(_rhs.m_data)
		{
			_rhs.m_size = 0;
			_rhs.m_caps = 0;
			_rhs.m_data = nullptr;
		}
		//!
		Array& operator = (const Array& _rhs)
		{
			if (m_data != _rhs.m_data)
				Clear().Push(_rhs.m_data, _rhs.m_size);
			return *this;
		}
		//!
		Array& operator = (Array&& _rhs)
		{
			Swap(m_size, _rhs.m_size);
			Swap(m_caps, _rhs.m_caps);
			Swap(m_data, _rhs.m_data);
			return *this;
		}

		TODO("More constructors");

		// [capacity]

		//!	Get size of array.
		uint Size(void) const { return m_size; }
		//!	Get real size of array.
		uint Capacity(void) const { return m_caps; }
		//!	Get number of unused elements.
		uint Unused(void) const { return m_caps - m_size; }
		//!	Is empty.
		bool IsEmpty(void) const { return m_size == 0; }
		//!	Non empty.
		bool NonEmpty(void) const { return m_size != 0; }
		//!	Resize array.
		Array& Resize(uint _size)
		{
			if (_size < m_size)
			{
				DestroyRange(m_data + _size, m_data + m_size);
				m_size = _size;
			}
			else if (_size > m_size)
			{
				if (Unused() < _size)
					_Realloc(GrowTo(m_caps, _size));
				ConstructRange(m_data + m_size, m_data + _size);
				m_size = _size;
			}
			return *this;
		}
		//!	Resize array and fill new elements a value.
		Array& Resize(uint _size, const T& _value)
		{
			if (_size < m_size)
			{
				DestroyRange(m_data + _size, m_data + m_size);
				m_size = _size;
			}
			else if (_size > m_size)
			{
				if (Unused() < _size)
					_Realloc(GrowTo(m_caps, _size));
				ConstructRange(m_data + m_size, m_data + _size, _value);
				m_size = _size;
			}
			return *this;
		}
		//!	Reserve size of array.
		Array& Reserve(uint _size, bool _compact = true)
		{
			if (_size < m_size)
				_size = m_size;
			if (_size > m_caps || (_compact && _size != m_caps))
				_Realloc(_size);
			return *this;
		}
		//! Reallocate array and delete unused elements.
		Array& Compact(void)
		{
			if (m_size != m_caps)
				_Realloc(m_size);
			return *this;
		}
		//!	Destroy all elements.
		Array& Clear(void)
		{
			return Resize(0);
		}
		//!	Destroy all elements and delete memory.
		Array& Free(void)
		{
			if (m_data)
			{
				Resize(0);
				Deallocate(m_data);
				m_data = nullptr;
			}
			return *this;
		}

		// [access]

		//!
		T& operator [] (uint _index)
		{
			ASSERT(_index < m_size, "Invalid index");
			return m_data[_index];
		}
		//!
		const T& operator [] (uint _index) const
		{
			ASSERT(_index < m_size, "Invalid index");
			return m_data[_index];
		}
		//!
		T& At(uint _index)
		{
			ASSERT(_index < m_size, "Invalid index");
			return m_data[_index];
		}
		//!
		const T& At(uint _index) const
		{
			ASSERT(_index < m_size, "Invalid index");
			return m_data[_index];
		}
		//!	Get first element.
		T& Front(void)
		{
			ASSERT(m_size != 0, "Empty array");
			return m_data[0];
		}
		//!	Get first element.
		const T& Front(void) const
		{
			ASSERT(m_size != 0, "Empty array");
			return m_data[0];
		}
		//!	Get last element.
		T& Back(void)
		{
			ASSERT(m_size != 0, "Empty array");
			return m_data[m_size - 1];
		}
		//!	Get last element.
		const T& Back(void) const
		{
			ASSERT(m_size != 0, "Empty array");
			return m_data[m_size - 1];
		}

		TODO_EX("Array", "Add Data");

		// [iterators]

		//!	Get a iterator of begin of array.
		Iterator Begin(void) { return Iterator(m_data); }
		//!	Get a iterator of begin of array.
		ConstIterator Begin(void) const { return ConstIterator(m_data); }
		//!	Get a iterator of end of array.
		Iterator End(void) { return Iterator(m_data + m_size); }
		//!	Get a iterator of end of array.
		ConstIterator End(void) const { return ConstIterator(m_data + m_size); }
		//! Get index from iterator.
		uint Index(Iterator _iter) const { return _iter - m_data; }
		//! Get index from iterator.
		uint Index(ConstIterator _iter) const { return _iter - m_data; }
		//! Get iterator from index
		Iterator GetInterator(uint _index) { return > Iterator(m_data + (_index < m_size ? _index : m_size)); }
		//! Get iterator from index
		ConstIterator GetInterator(uint _index) const { return > Iterator(m_data + (_index < m_size ? _index : m_size)); }

		// [modifiers]

		//! Add one element to end of array.
		Array& Push(const T& _value)
		{
			if (m_size == m_caps)
			{
				if ((&_value - m_data) <= (ptrdiff_t)m_caps)
					return Push(T(_value));
				_Reserve(1);
			}
			Construct(m_data + m_size, _value);
			++m_size;
			return *this;
		}
		//! Add one element to end of array.
		Array& Push(T&& _value)
		{
			if (m_size == m_caps)
				_Reserve(1);
			Construct(m_data + m_size, Forward<T>(_value));
			++m_size;
			return *this;
		}
		//! Add few elements with one value to end of array.
		Array& Push(const T& _value, uint _count)
		{
			_Reserve(_count);
			uint _newSize = m_size + _count;
			ConstructRange(m_data + m_size, m_data + _newSize, _value);
			m_size = _newSize;
			return *this;
		}
		//! Append elements to end of array.
		Array& Push(const T* _value, uint _count)
		{
			ASSERT(!_count || _value);
			_Reserve(_count);
			uint _newSize = m_size + _count;
			CopyRange(m_data + m_size, _value, _value + _count);
			m_size = _newSize;
			return *this;
		}
		//!	Delete one element from end of array.
		Array& Pop(void)
		{
			if (m_size)
			{
				uint _newSize = m_size - 1;
				Destroy(m_data + _newSize);
				m_size = _newSize;
			}
			return *this;
		}
		//!	Delete few elements from end of array.
		Array& Pop(uint _count)
		{
			if (_count > m_size)
				_count = m_size;
			if (_count)
			{
				uint _newSize = m_size - _count;
				DestroyRange(m_data + _newSize, m_data + m_size);
				m_size = _newSize;
			}
			return *this;
		}

		//!	Erase a range of elements. \return iterator to the next element. 
		Iterator Erase(uint _index, uint _count = 1)
		{
			if (_index > m_size)
				_index = m_size;
			if (_index + _count > m_size)
				_count = m_size - _index;
			if (!_count)
				return End();

			T* _dst = m_data + _index;
			T* _src = _dst + _count;
			T* _end = m_data + m_size;
			_dst = AssignMoveRange(_dst, _src, _end);
			DestroyRange(_dst, _end);
			m_size -= _count;

			return Iterator(_src);
		}
		//! Erase an element by iterator. \return iterator to the next element. 	
		Iterator Erase(const Iterator& _pos) { return Erase(_pos - m_data, 1); }
		//!	Erase a range of elements by iterators. \return iterator to the next element. 
		Iterator Erase(const Iterator& _start, const Iterator& _end) { return Erase(_start - m_data, _end - m_data); }
		//! Erase an element. \return iterator to the element by index. \note Ordering of elements is not saved.
		Iterator FastErase(uint _index)
		{
			if (_index < m_size)
			{
				uint _newSize = m_size - 1;
				if (_index != _newSize)
					m_data[_index] = Move(m_data[_newSize]); // move last element to index
				Destroy(m_data + _newSize); // remove last element
				m_size = _newSize;

				return Iterator(m_data + _index);
			}
			return End();
		}
		//! Erase an element. \return iterator to the element by index. \note Ordering of elements is not saved.
		Iterator FastErase(const Iterator& _pos) { return FastErase(_pos - m_data); }
		//! Erase an element if found.
		bool Remove(const T& _value)
		{
			Iterator i = Find(_value);
			if (i != End())
			{
				Erase(i);
				return true;
			}
			return false;
		}
		//! Erase an element if found. \note Ordering of elements is not saved.
		bool FastRemove(const T& _value)
		{
			Iterator i = Find(_value);
			if (i != End())
			{
				FastErase(i);
				return true;
			}
			return false;
		}
		//! Insert an elements in position.
		Iterator Insert(uint _pos, const T* _src, uint _count)
		{
			if (_pos > m_size)
				_pos = m_size;

			if (Unused() < _count)
			{
				uint _newSize = m_size + _count;
				uint _newCaps = GrowTo(m_caps, _newSize);
				T* _newData = Allocate<T>(_newCaps);
				T* _dst = _newData;
				T* _iStart = _dst + _pos; // insert start
				T* _iEnd = _iStart + _count; // insert end
				T* _iPos = m_data + _pos; // insert pos

				MoveAndDestroyRange(_dst, m_data, _iPos);
				CopyRange(_iStart, _src, _src + _count);
				MoveAndDestroyRange(_iEnd, _iPos, m_data + m_size);
				Deallocate(m_data);

				m_size = _newSize;
				m_caps = _newCaps;
				m_data = _newData;
			}
			else
			{
				T* _iStart = m_data + _pos;	// insert start
				T* _iEnd = _iStart + _count; // insert end
				T* _cEnd = m_data + m_size;	// current end
				T* _nEnd = _cEnd + _count; // new end
				T* _dst = _nEnd; // write 
				T* _copySrc = _cEnd; // read
				const T* _srcEnd = _src + _count; // source end

				// construct new elements from array
				for (T* _end = Max(_iEnd, _cEnd); _dst > _end;) 
					Construct(--_dst, Move(*--_copySrc));

				// construct new elements from source
				while (_dst > _cEnd)
					Construct(--_dst, *--_srcEnd);

				// move elements inside array
				while (_dst > _iEnd)
					*--_dst = Move(*--_copySrc);

				// copy from source
				while (_dst > _iStart)
					*--_dst = *--_srcEnd;

				m_size += _count;
			}
			return Iterator(m_data + _pos);
		}
		//! Construct new element in position.
		template<class... Args> Iterator Emplace(uint _pos, Args&&... _args)
		{
			if (_pos > m_size)
				_pos = m_size;

			if (m_size == m_caps)
				_Reserve(1);

			if (_pos == m_size)
			{
				Construct(m_data + m_size, Forward<Args>(_args)...);
				++m_size;
			}
			else
			{
				Construct(m_data + m_size);
				uint _newSize = m_size + 1;
				for (T *dst = m_data + _newSize, *src = m_data + _pos, *end = m_data + m_size; src < _end;)
					Construct(--dst, Move(*--end));
				m_data[_pos] = T(Forward<Args>(_args)...);
				m_size = _newSize;
			}
			return Iterator(m_data + _pos);
		}
		//! Construct new element in end of array.
		template<class... Args> Iterator EmplaceBack(Args&&... _args)
		{
			return Emplace(m_size, Forward<Args>(_args)...);
		}

		// [compare]

		//!
		bool operator == (const Array& _rhs) const
		{
			if (m_size != _rhs.m_size)
				return false;
			if (m_data == _rhs.m_data)
				return true;
			for (uint i = 0; i < m_size; ++i)
			{
				if (m_data[i] != _rhs.m_data[i])
					return false;
			}
			return true;
		}
		//!
		bool operator != (const Array& _rhs)
		{
			return !(*this == _rhs);
		}

		// [operations]

		//! Search first element.
		Iterator Find(const T& _value)
		{
			for (Iterator i = Begin(), e = End(); i != e; ++i)
			{
				if (*i == _value)
					return i;
			}
			return i;
		}
		//! Search first element.
		ConstIterator Find(const T& _value)	const
		{
			for (ConstIterator i = Begin(), e = End(); i != e; ++i)
			{
				if (*i == _value)
					return i;
			}
			return i;
		}
		//! Search next element.
		Iterator Find(const Iterator& _start, const T& _value)
		{
			ASSERT(Index(_start) <= m_size, "Invalid iterator");
			for (Iterator i = _start, e = End(); i != e; ++i)
			{
				if (*i == _value)
					return i;
			}
			return i;
		}
		//! Search next element.
		ConstIterator Find(const ConstIterator& _start, const T& _value) const
		{
			ASSERT(Index(_start) <= m_size, "Invalid iterator");
			for (ConstIterator i = _start, e = End(); i != e; ++i)
			{
				if (*i == _value)
					return i;
			}
			return i;
		}

		TODO_EX("Array", "Add BinarySearch");
		TODO_EX("Array", "Add ReverseFind");



	protected:
		//! Reallocate an array.
		void _Realloc(uint _size)
		{
			T* _newData = Allocate<T>(_size);
			MoveAndDestroyRange(_newData, m_data, m_data + m_size);
			Deallocate<T>(m_data);
			m_data = _newData;
			m_caps = _size;
		}
		//! Do reserve new elements.
		void _Reserve(uint _append)
		{
			if (Unused() < _append)
				_Realloc(GrowTo(m_caps, m_size + _append));
		}

		//! Number of used elements.
		uint m_size = 0;
		//! Number of all elements.
		uint m_caps = 0;
		//! The data.
		T* m_data = nullptr;
	};

	//----------------------------------------------------------------------------//
	// NodeIterator
	//----------------------------------------------------------------------------//

	template <class T> class NodeIterator
	{
	public:

	protected:
		T* m_node;
	};

	//----------------------------------------------------------------------------//
	// List
	//----------------------------------------------------------------------------//

	template <class T> class List
	{
	public:
		class Node
		{
		public:

		protected:
			friend class List;

			T m_value;
			Node* prev;
			Node* next;
		};

		// [capasity]
		// Size
		// IsEmpty
		// NonEmpty
		// Resize
		// Clear


		// [access]
		// Front
		// Back

		// [iterators]
		// Begin
		// End

		// [modifiers]
		// Assign
		// EmplaceFront
		// PushFront
		// PopFront
		// EmplaceBack
		// PushBack
		// PopBack
		// Emplace
		// Insert
		// Erase

		// [compare]
		// ==
		// !=

		// [operations]
		// splice
		// remove
		// remove_if
		// unique
		// merge
		// sort
		// reverse
		// Find

	protected:
		uint m_size = 0;
		Node* m_first = nullptr;
		Node* m_last = nullptr;
	};

	//----------------------------------------------------------------------------//
	// Pair
	//----------------------------------------------------------------------------//

	template <class T, class U> struct Pair
	{
		T first;
		U second;

		Pair(void) = default;
		Pair(const T& _first, const U& _second) : first(_first), second(_second) { }
		Pair(T&& _first, U&& _second) : first(Forward<T>(_first)), second(Forward<U>(_second)) { }
	};

	//----------------------------------------------------------------------------//
	// HashMap
	//----------------------------------------------------------------------------//

	template <class T, class U> class HashMap
	{
	public:
		typedef Pair<T, U> KeyValue;

		class Node
		{
		public:

		protected:
			friend class HashMap;

			KeyValue m_value;
			uint m_hash;
			Node* m_prev;
			Node* m_next;
			Node* m_down;
		};

		// [capasity]
		// Size
		// IsEmpty
		// NonEmpty


		// [access]

		// [iterators]

		// [modifiers]

		// [compare]

		// [operations]

	protected:
		uint m_caps = 0;
		uint m_size = 0;
		Node* m_nodes = nullptr;
		Node** m_buckets = nullptr;
	};

	//----------------------------------------------------------------------------//
	// StringBuffer
	//----------------------------------------------------------------------------//

	//! Get length of string.
	inline uint CStringLength(const char* _str, int _length = -1) { return _str ? (uint)(_length < 0 ? strlen(_str) : _length) : 0; }
	//! Get length of string.
	inline uint CStringLength(const wchar_t* _str, int _length = -1) { return _str ? (uint)(_length < 0 ? wcslen(_str) : _length) : 0; }

	//!
	template <class T> struct StringBuffer : public NonCopyable
	{
	public:

	private:
		//!	Construct. \note For internal usage.
		StringBuffer(uint _size = 0) : capacity(_size) { }
		//!	Construct from string. \note For internal usage.
		StringBuffer(uint _size, const T* _str, uint _length) : capacity(_size), length(_length)
		{
			memcpy(str, _str, _length * sizeof(T));
			str[_length] = 0;
		}

	public:
		//!	Get new buffer with specified size.
		//!\param[in] _size specifies max length.
		static StringBuffer* New(uint _size)
		{
			if (_size == 0)
				return Null();
			StringBuffer* _buff = AllocateBlock<StringBuffer>(sizeof(StringBuffer) + _size);
			new(_buff) StringBuffer(_size);
			return _buff;
		}
		//! Get new buffer from string.
		static StringBuffer* New(const T* _str, int _length = -1)
		{
			_length = CStringLength(_str);
			if (_length)
			{
				StringBuffer* _buff = AllocateBlock<StringBuffer>(sizeof(StringBuffer) + _length);
				new(_buff) StringBuffer(_length, _str, _length);
				return _buff;
			}
			return Null();
		}
		//!	Get empty buffer.
		static StringBuffer* Null(void)
		{
			static StringBuffer _empty;
			AtomicAdd(_empty.refs, 1);
			return &_empty;
		}

		//! Resize and make unique. It's threadsafe.
		//!\param[in] _size specifies max length.
		StringBuffer* Reserve(uint _size, bool _compact = false)
		{
			if (IsShared() || capacity < _size) // buffer is shared or too small.
			{
				uint _newSize = _compact ? (capacity < _size ? _size : capacity) : (GrowTo(capacity, _size) | 15);
				StringBuffer* _buff = New(_newSize);
				memcpy(_buff->str, str, (length + 1) * sizeof(T));
				_buff->length = length;
				Release();
				return _buff;
			}
			return this;
		}
		//!
		StringBuffer* Unique(void)
		{
			return Reserve(capacity, true);
		}
		//!	Make unique directly.
		StringBuffer* Detach(void)
		{
			StringBuffer* _buff = New(capacity);
			memcpy(_buff->str, str, (length + 1) * sizeof(T));
			_buff->length = length;
			Release();
			return _buff;
		}

		//!\return true if buffer is shared.
		bool IsShared(void) const
		{
			return AtomicGet(refs, MemoryOrder::Relaxed) > 1;
		}
		//!\return true if buffer is unique.
		bool IsUnique(void) const
		{
			return AtomicGet(refs, MemoryOrder::Relaxed) == 1;
		}
		//!	Increments the counter of references.
		StringBuffer* AddRef(void)
		{
			AtomicAdd(refs, 1);
			return this;
		}
		//!	Decrements the counter of references and destroy this when he equals zero.
		void Release(void)
		{
			if (AtomicSubtract(refs, 1) == 1)
				Deallocate(this);
		}

		//! Number of references. \note Read only.
		mutable int refs = 1;
		//! Length of string, without last null symbol.
		uint length = 0;
		//! Max length of string, without last null symbol. \note Read only.
		uint capacity = 0;
		union
		{
			//! The string. Max size is capasity + 1.
			T str[1];
			T ch = 0;
		};
	};

	//----------------------------------------------------------------------------//
	// String
	//----------------------------------------------------------------------------//

	//!
	class RX_API String
	{
	public:
		typedef StringBuffer<char> Buffer;

		//!
		DEPRECATED class Proxy
		{
			friend class String;

			Proxy(String* _str, uint _index) : str(_str), index(_index) { }

			String* str;
			uint index;

		public:
			//!
			Proxy& operator = (const Proxy& _rhs)
			{
				str->SetChar(index, static_cast<char>(_rhs));
				return *this;
			}
			//!
			Proxy& operator = (char _ch)
			{
				str->SetChar(index, _ch);
				return *this;
			}
			//!
			operator const char(void) const { return str->At(index); }
		};

		//! 
		String(void) : m_buffer(Buffer::Null()) { }
		//! 
		~String(void) { if (m_buffer) m_buffer->Release(); }
		//! 
		String(const String& _other) : m_buffer(_other.m_buffer->AddRef()) {}
		//! 
		String(String&& _temp) { Swap(m_buffer, _temp.m_buffer); }
		//! 
		String(const char* _str, int _length = -1) : m_buffer(Buffer::New(_str)) { }
		//!
		String(const char* _str, const char* _end) : m_buffer(Buffer::New(_str, (int)(_end - _str))) { }
		//!
		String(char _ch) : m_buffer(_ch ? Buffer::New(&_ch, 1) : Buffer::Null()) { }
		//!
		String(uint _count, char _ch) : m_buffer(Buffer::Null()) { Append(_count, _ch); }

		//! 
		String& operator = (const String& _rhs)
		{
			_rhs.m_buffer->AddRef();
			m_buffer->Release();
			m_buffer = _rhs.m_buffer;
			return *this;
		}
		//! 
		String& operator = (String&& _rhs)
		{
			m_buffer->Release();
			m_buffer = _rhs.m_buffer;
			_rhs.m_buffer = nullptr;
			return *this;
		}
		//! 
		String& operator = (const char* _rhs) { return Clear().Append(_rhs); }
		//! 
		String& operator = (char _rhs) { return Clear().Append(_rhs); }

		//! 
		String& operator += (const String& _rhs) { return Append(_rhs); }
		//! 
		String& operator += (const char* _rhs) { return Append(_rhs); }
		//! 
		String& operator += (char _rhs) { return Append(_rhs); }
		//! 
		String operator + (const String& _rhs) const { return String(*this).Append(_rhs); }
		//! 
		String operator + (const char* _rhs) const { return String(*this).Append(_rhs); }
		//! 
		String operator + (char _rhs) const { return String(*this).Append(_rhs); }
		//!
		friend String operator + (const char* _lhs, const String& _rhs) { return String(_lhs).Append(_rhs); }
		//!
		friend String operator + (char _lhs, const String& _rhs) { return String(_lhs).Append(_rhs); }
		
		// [capasity]

		//!
		uint Size(void) const { return m_buffer->length; }
		//!
		uint Length(void) const { return m_buffer->length; }
		//!
		uint RealLength(void) const { return Length(m_buffer->str); }
		//!
		String& UpdateLength(void)  
		{ 
			m_buffer = m_buffer->Unique();
			m_buffer->length = Length(m_buffer->str);
		}
		//!
		uint Capacity(void) const { return m_buffer->capacity; }
		//! 
		String& Clear(void)
		{
			if (m_buffer->IsShared())
			{
				m_buffer->Release();
				m_buffer = Buffer::Null();
			}
			else
			{
				m_buffer->length = 0;
				m_buffer->str[0] = 0;
			}
			return *this;
		}
		//!
		String& Resize(uint _size, char _ch = 0)
		{
			if (m_buffer->length < _size)
			{
				Append(_size - m_buffer->length, _ch);
			}
			else if (m_buffer->length > _size)
			{
				m_buffer = m_buffer->Unique();
				m_buffer->length = _size;
				m_buffer->str[_size] = 0;
			}
			return *this;
		}
		//! 
		String& Reserve(uint _size, bool _compact = true)
		{
			m_buffer = m_buffer->Reserve(_size, _compact);
			return *this;
		}
		//!
		String& Compact(void)
		{
			if (m_buffer->length != m_buffer->capacity)
			{
				Buffer* _newBuffer = Buffer::New(m_buffer->str, m_buffer->length);
				m_buffer->Release();
				m_buffer = _newBuffer;
			}
			return *this;
		}
		//!
		String& Free(void)
		{
			if (m_buffer->capacity > 0)
			{
				m_buffer->Release();
				m_buffer = Buffer::Null();
			}
			return *this;
		}

		// [access]

		//!
		operator const char* (void) const { return m_buffer->str; }
		//! Read char.
		const char operator [] (int _index) const
		{
			ASSERT((uint)_index <= m_buffer->length, "Invalid index");
			return m_buffer->str[_index];
		}
		//! Modify char. \note It's not effective. Use String::Data instead.
		char& operator () (int _index)
		{
			ASSERT((uint)_index <= m_buffer->length, "Invalid index");
			if(m_buffer->IsShared())
				m_buffer = m_buffer->Detach();
			return m_buffer->str[_index];
			//return Proxy(this, _index);
		}

		//! Get modifiable data.
		char* Data(uint _offset = 0)
		{
			ASSERT(_offset <= m_buffer->length, "Invalid offset");
			if (m_buffer->IsShared())
				m_buffer = m_buffer->Detach();
			return m_buffer->str + _offset;
		}
		//! Get constant data.
		const char* CStr(uint _offset = 0) const
		{ 
			ASSERT(_offset <= m_buffer->length, "Invalid offset");
			return m_buffer->str;
		}
		//! Get constant data.
		const char* Ptr(uint _offset = 0) const { return CStr(_offset); }
		//!
		char At(uint _index) const
		{
			ASSERT(_index <= m_buffer->length + 1, "Invalid index");
			return m_buffer->str[_index];
		}
		//!
		char Front(void) const { return m_buffer->str[0]; }
		//!
		char Back(void) const { return m_buffer->length > 1 ? m_buffer->str[m_buffer->length - 1] : 0; }

		// [iterators]


		// [modifiers]

		//! Append the string.
		String& Append(const String& _str)
		{
			return Append(_str.m_buffer->str, _str.m_buffer->length);
		}
		//! Append the string.
		String& Append(const char* _str, int _length = -1)
		{
			Buffer* _tmp = nullptr;
			if (_str >= m_buffer->str && _str <= m_buffer->str + m_buffer->capacity)
				_tmp = m_buffer->AddRef();

			_length = Length(_str, _length);
			m_buffer = m_buffer->Reserve(m_buffer->length + _length);
			if (_length)
			{
				memcpy(m_buffer->str + m_buffer->length, _str, _length);
				m_buffer->length += _length;
				m_buffer->str[m_buffer->length] = 0;
			}
			if (_tmp)
				_tmp->Release();

			return *this;
		}
		//! Append the string range.
		String Append(const char* _str, const char* _end)
		{
			ASSERT(_str <= _end);
			return Append(_str, (int)(_end - _str));
		}
		//! Append the symbol.
		String& Append(char _ch)
		{
			uint _length = m_buffer->length + 1;
			m_buffer = m_buffer->Reserve(_length);
			m_buffer->str[m_buffer->length] = _ch;
			m_buffer->str[_length] = 0;
			m_buffer->length = _length;
			return *this;
		}
		//! Append the symbols.
		String& Append(uint _count, char _ch)
		{
			if (_count)
			{
				uint _length = m_buffer->length + _count;
				m_buffer = m_buffer->Reserve(_length);
				memset(m_buffer->str + m_buffer->length, _ch, _count);
				m_buffer->str[_length] = 0;
				m_buffer->length = _length;
			}
			return *this;
		}
		//!
		String& SetChar(uint _index, char _ch)
		{
			ASSERT(_index <= m_buffer->length, "Invalid index");
			m_buffer = m_buffer->Unique();
			m_buffer->str[_index] = _ch;
			return *this;
		}

		// [compare]

		// [operations]
		//!
		String SubStr(uint _offset, int _length = -1) const
		{
			ASSERT(_length < 0 || (uint)(_offset + _length) <= m_buffer->length);
			return String(m_buffer->str + _offset, _length);
		}
		// find

		String& Lower(void)
		{
			m_buffer = m_buffer->Unique();
			Lower(m_buffer->str, m_buffer->length);
			return *this;
		}
		String& Upper(void)
		{
			m_buffer = m_buffer->Unique();
			Upper(m_buffer->str, m_buffer->length);
			return *this;
		}


		// [C String utilities]

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
		static uint Length(const char* _str, int _length = -1)
		{
			return _str ? (uint)(_length < 0 ? strlen(_str) : _length) : 0;
		}

		//!
		static char* Lower(char* _str, int _length = -1)
		{
			ASSERT(_length < (int)Length(_str, _length));
			for (const char* _end = _str + Length(_str, _length); _str < _end; ++_str)
				*_str = Lower(*_str);
			return _str;
		}
		//!
		static char* Upper(char* _str, int _length = -1)
		{
			ASSERT(_length < (int)Length(_str, _length));
			for (const char* _end = _str + Length(_str, _length); _str < _end; ++_str)
				*_str = Upper(*_str);
			return _str;
		}



		//!
		static int Compare(const char* _lhs, const char* _rhs, bool _ignoreCase = false)
		{
			_lhs = _lhs ? _lhs : "";
			_rhs = _rhs ? _rhs : "";
			if (_ignoreCase)
			{
				while (*_lhs && Lower(*_lhs++) == Upper(*_rhs++));
				return *_lhs - *_rhs;
			}
			return strcmp(_lhs, _rhs);
		}

		//!
		static String Format(const char* _fmt, ...)
		{
			va_list _args;
			va_start(_args, _fmt);
			String _str = FormatV(_fmt, _args);
			va_end(_args);
			return _str;
		}
		//!
		static String FormatV(const char* _fmt, va_list _args)
		{
#if 0
			char _buff[4096];
			vsnprintf_s(_buff, sizeof(_buff), _fmt, _args);
			return _buff;
#else
			//this code from ptypes: pputf.cxx
			struct
			{
				bool operator () (char _ch) const
				{
					return strchr(" #+-~.0123456789", _ch) != 0;
				}
			} _check_fmtopts;
			enum fmt_type_t { FMT_NONE, FMT_CHAR, FMT_SHORT, FMT_INT, FMT_LONG, FMT_LARGE, FMT_STR, FMT_PTR, FMT_DOUBLE, FMT_LONG_DOUBLE };

			String _res;
			fmt_type_t fmt_type;
			const char *e, *p = _fmt;
			char buf[4096], fbuf[128];
			while (p && *p != 0)
			{
				// write out raw data between format specifiers
				e = strchr(p, '%');
				if (e == 0)
					e = p + strlen(p);
				if (e > p)
					_res.Append(p, e);
				if (*e != '%')
					break;
				if (*++e == '%') // write out a single '%' 
				{
					_res += '%';
					p = e + 1;
					continue;
				}


				// build a temporary buffer for the conversion specification
				fbuf[0] = '%';
				char* f = fbuf + 1;
				bool modif = false;

				// formatting flags and width specifiers
				while (_check_fmtopts(*e) && uint(f - fbuf) < sizeof(fbuf) - 5)
					*f++ = *e++, modif = true;

				// prefixes
				switch (*e)
				{
				case 'h':
					fmt_type = FMT_SHORT;
					*f++ = *e++;
					break;

				case 'L':
					fmt_type = FMT_LONG_DOUBLE;
					*f++ = *e++;
					break;

				case 'l':
				{
					if (*++e == 'l')
					{
#                   if defined(_MSC_VER) || defined(__BORLANDC__)
						*f++ = 'I'; *f++ = '6'; *f++ = '4';
#                   else
						*f++ = 'l'; *f++ = 'l';
#                   endif
						++e;
						fmt_type = FMT_LARGE;
					}
					else
					{
						*f++ = 'l';
						fmt_type = FMT_LONG;
						// x64 ?
					}
				} break;

				default:
					fmt_type = FMT_NONE;
					break;
				};

				// format specifier
				switch (*e)
				{
				case 'c':
					fmt_type = FMT_CHAR;
					*f++ = *e++;
					break;

				case 'd':
				case 'i':
				case 'o':
				case 'u':
				case 'x':
				case 'X':
					if (fmt_type < FMT_SHORT || fmt_type > FMT_LARGE)
						fmt_type = FMT_INT;
					*f++ = *e++;
					break;

				case 'e':
				case 'E':
				case 'f':
				case 'g':
				case 'G':
					if (fmt_type != FMT_LONG_DOUBLE)
						fmt_type = FMT_DOUBLE;
					*f++ = *e++;
					break;

				case 's':
					fmt_type = FMT_STR;
					*f++ = *e++;
					break;

				case 'p':
					fmt_type = FMT_PTR;
					*f++ = *e++;
					break;
				};
				if (fmt_type == FMT_NONE)
					break;
				*f = 0;

				// some formatters are processed here 'manually', while others are passed to snprintf
				int s = 0;
				switch (fmt_type)
				{
				case FMT_NONE: // to avoid compiler warning 
					break;
				case FMT_CHAR:
					if (modif)
						s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int));
					else
						_res += (char)(va_arg(_args, int));
					break;

				case FMT_SHORT:
					s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int));
					break;

				case FMT_INT:
					s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int));
					break;

				case FMT_LONG:
					s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, long));
					break;

				case FMT_LARGE:
					s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int64));
					break;

				case FMT_STR:
				{
					if (modif)
						s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, const char*));
					else
					{
						const char* _str = va_arg(_args, const char*);
						if (_str)
							_res += _str;
						//else _res += "<null>";
					}
				}break;

				case FMT_PTR:
					s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, void*));
					break;
				case FMT_DOUBLE:

					s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, double));
					for (; s > 0; --s)
					{
						if (buf[s - 1] == '0')
							buf[s - 1] = 0;
						else if (buf[s - 1] == '.')
						{
							buf[--s] = 0;
							break;
						}
						else
							break;
					}
					break;

				case FMT_LONG_DOUBLE:
					s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, long double));
					break;
				}

				if (s > 0)
					_res.Append(buf, s);

				p = e;
			}
			return _res;
#endif
		}

		//!
		static bool Match(const char* _str, const char* _pattern, bool _ignoreCase = true)
		{
			// '*' - any symbols sequence
			// ' '(space) - any number of spaces
			// '['charset']' - set of symbols
			// '['lower'-'upper']' - range of symbols
			// "file.ext" matched "*.ext"

			if (!_str || !_str[0])
				return false;
			if (!_pattern || !_pattern[0])
				return false;

			Array<Pair<const char*, const char*>> _stack; // <string, pattern>
			_stack.Push({ _str, _pattern });
			for (const char *_s = _str, *_p = _pattern;;)
			{
			$_frame:
				if (*_p == 0 && *_s == 0)
					goto $_true;
				if (*_s == 0 && *_p != '*')
					goto $_false;
				if (*_p == '*') // подстановка
				{
					++_p;
					if (*_p == 0) goto $_true;
					for (;; )
					{
						_stack.Back().first = _s;
						_stack.Back().second = _p;
						_stack.Push({ _s, _p });
						goto $_frame;
					$_xmatch_true:
						goto $_true;
					$_xmatch_false:
						if (*_s == 0) 
							goto $_false;
						++_s;
					}
				}
				if (*_p == '?') // любой символ 
					goto $_match;
				if (*_p == ' ') // любое количество пробелов
				{
					if (*_s == ' ')
					{
						for (; _p[1] == ' '; ++_p);
						for (; _s[1] == ' '; ++_s);
						goto $_match;
					}
					goto $_false;
				}
				if (*_p == '[') // набор символов
				{
					for (++_p; ; )
					{
						if (*_p == ']' || *_p == 0)
							goto $_false;
						if (*_p == '\\') // управл€ющий символ
						{
							if (_p[1] == '[' || _p[1] == ']' || _p[1] == '\\')
								++_p;
						}
						if (_ignoreCase ? (Lower(*_p) == Lower(*_s)) : (*_p == *_s))
							break;

						if (_p[1] == '-') // диапазон
						{
							if (!_p[2])
								goto $_false;
							if (_ignoreCase)
							{
								char l = Lower(*_s);
								char u = Upper(*_s);
								if (_p[0] <= l && _p[2] >= l)
									break;
								if (_p[0] >= l && _p[2] <= l)
									break;
								if (_p[0] <= u && _p[2] >= u)
									break;
								if (_p[0] >= u && _p[2] <= u)
									break;
							}
							else
							{
								if (_p[0] <= *_s && _p[2] >= *_s)
									break;
								if (_p[0] >= *_s && _p[2] <= *_s)
									break;
							}
							_p += 2;
						}
						++_p;
					}
					while (*_p != ']')
					{
						if (*_p == '\\' && (_p[1] == '[' || _p[1] == ']' || _p[1] == '\\')) // управл€ющий символ
							++_p;
						if (*_p == 0)
						{
							--_p;
							break;
						}
						++_p;
					}
					goto $_match;
				}
				if (*_p == '\\' && (_p[1] == '[' || _p[1] == ']' || _p[1] == '\\')) // управл€ющий символ
					++_p;
				if (_ignoreCase ? (Lower(*_p) != Lower(*_s)) : (*_p != *_s))
					goto $_false;

			$_match:
				++_p;
				++_s;
				continue;
			$_true:
				if (_stack.Size() > 1)
				{
					_stack.Pop();
					_s = _stack.Back().first;
					_p = _stack.Back().second;
					goto $_xmatch_true;
				}
				return true;
			$_false:
				if (_stack.Size() > 1)
				{
					_stack.Pop();
					_s = _stack.Back().first;
					_p = _stack.Back().second;
					goto $_xmatch_false;
				}
				break;
			}
			return false;
		}
		
		//!
		static const char* Find(const char* _str1, const char* _str2, bool _ignoreCase = false)
		{
			_str1 = _str1 ? _str1 : "";
			_str2 = _str2 ? _str2 : "";
			if (_ignoreCase)
			{
				for (; *_str1; *_str1++)
				{
					for (const char *a = _str1, *b = _str2; Lower(*a++) == Lower(*b++);)
					{
						if (!*b)
							return _str1;
					}
				}
				return nullptr;
			}
			return strstr(_str1, _str2);
		}
		TODO_EX("String", "Add ReverseFind");

		
		//!
		static void Split(const char* _str, const char* _delimiters, Array<String>& _dst)
		{
			if (_str)
			{
				const char* _end = _str;
				if (_delimiters)
				{
					while (*_str)
					{
						while (*_str && strchr(_delimiters, *_str))
							++_str;
						_end = _str;
						while (*_end && !strchr(_delimiters, *_end))
							++_end;
						if (_str != _end)
							_dst.Push(String(_str, _end));
						_str = _end;
					}
				}
				else
					_end = _str + strlen(_str);
				if (_str != _end)
					_dst.Push(String(_str, _end));
			}
		}



		static const String Empty;

	private:
		const char* operator * (void) const = delete;
		template <class T> String operator - (const T&) = delete;
		template <class T> friend String operator - (const T&, const String&) = delete;

	protected:
		Buffer* m_buffer = nullptr;
	};


	//----------------------------------------------------------------------------//
	// Atomic
	//----------------------------------------------------------------------------//

	enum class MemoryOrder
	{
		Relaxed = std::memory_order_relaxed,
		Consume = std::memory_order_consume,
		Acquire = std::memory_order_acquire,
		Release = std::memory_order_release,
		AcquireRelease = std::memory_order_acq_rel,
		Sequential = std::memory_order_seq_cst,
	};

	//!
	inline std::memory_order _StdMemoryOrder(MemoryOrder _order)
	{
		return static_cast<std::memory_order>(_order);
	}
	//!
	inline std::memory_order _StdMemoryOrderFail(MemoryOrder _order)
	{
		return _order == MemoryOrder::AcquireRelease ? std::memory_order_acquire : (_order == MemoryOrder::Release ? std::memory_order_relaxed : _StdMemoryOrder(_order));
	}
	//!
	template<class T> T AtomicGet(T& _atom, MemoryOrder _order = MemoryOrder::Sequential)
	{
		ASSERT(_order == MemoryOrder::Relaxed || _order == MemoryOrder::Consume || _order == MemoryOrder::Acquire || _order == MemoryOrder::Sequential);
		return std::atomic_load_explicit(reinterpret_cast<std::atomic<T>*>(&_atom), _StdMemoryOrder(_order));
	}
	//!
	template<class T> void AtomicSet(T& _atom, T _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		ASSERT(_order == MemoryOrder::Relaxed || _order == MemoryOrder::Consume || _order == MemoryOrder::Acquire || _order == MemoryOrder::Sequential);
		std::atomic_store_explicit(reinterpret_cast<std::atomic<T>*>(&_atom), _value, _StdMemoryOrder(_order));
	}
	//!
	template<class T> void AtomicSet(T*& _atom, void* _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		ASSERT(_order == MemoryOrder::Relaxed || _order == MemoryOrder::Consume || _order == MemoryOrder::Acquire || _order == MemoryOrder::Sequential);
		std::atomic_store_explicit(reinterpret_cast<std::atomic<T*>*>(&_atom), _value, _StdMemoryOrder(_order));
	}
	//! \return Previous value.
	template<class T> T AtomicAdd(T& _atom, T _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_fetch_add_explicit(reinterpret_cast<std::atomic<T>*>(&_atom), _value, _StdMemoryOrder(_order));
	}
	//! \return Previous value.
	template<class T> T* AtomicAdd(T*& _atom, ptrdiff_t _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_fetch_add_explicit(reinterpret_cast<std::atomic<T*>*>(&_atom), _value, _StdMemoryOrder(_order));
	}
	//! \return Previous value.
	template<class T> T AtomicSubtract(T& _atom, T _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_fetch_sub_explicit(reinterpret_cast<std::atomic<T>*>(&_atom), _value, _StdMemoryOrder(_order));
	}
	//! \return Previous value.
	template<class T> T* AtomicSubtract(T*& _atom, ptrdiff_t _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_fetch_sub_explicit(reinterpret_cast<std::atomic<T*>*>(&_atom), _value, _StdMemoryOrder(_order));
	}
	//! \return Previous value.
	template <class T> T AtomicAnd(T& _atom, T _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_fetch_and_explicit(reinterpret_cast<std::atomic<T>*>(&_atom), _value, _StdMemoryOrder(_order));
	}
	//! \return Previous value.
	template <class T> T AtomicOr(T& _atom, T _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_fetch_or_explicit(reinterpret_cast<std::atomic<T>*>(&_atom), _value, _StdMemoryOrder(_order));
	}
	//! \return Previous value.
	template <class T> T AtomicXor(T& _atom, T _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_fetch_xor_explicit(reinterpret_cast<std::atomic<T>*>(&_atom), _value, _StdMemoryOrder(_order));
	}
	//!
	template<class T> T AtomicExchange(T& _atom, T _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_exchange_explicit(reinterpret_cast<std::atomic<T>*>(&_atom), _value, _StdMemoryOrder(_order));
	}
	//!
	template<class T> T AtomicExchange(T*& _atom, T* _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_exchange_explicit(reinterpret_cast<std::atomic<T*>*>(&_atom), _value, _StdMemoryOrder(_order));
	}
	//!
	template<class T> bool AtomicCompareExchange(T& _atom, T _expected, T _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_compare_exchange_strong_explicit(reinterpret_cast<std::atomic<T>*>(&_atom), &_expected, _value, _StdMemoryOrder(_order), _StdMemoryOrderFail(_order));
	}
	//!
	template<class T> bool AtomicCompareExchange(T*& _atom, T* _expected, T* _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_compare_exchange_strong_explicit(reinterpret_cast<std::atomic<T*>*>(&_atom), &_expected, _value, _StdMemoryOrder(_order), _StdMemoryOrderFail(_order));
	}
	//!
	template<class T> bool AtomicCompareExchange(T& _atom, T* _expected, T _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_compare_exchange_strong_explicit(reinterpret_cast<std::atomic<T>*>(&_atom), _expected, _value, _StdMemoryOrder(_order), _StdMemoryOrderFail(_order));
	}
	//!
	template<class T> bool AtomicCompareExchange(T*& _atom, T** _expected, T* _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_compare_exchange_strong_explicit(reinterpret_cast<std::atomic<T*>*>(&_atom), _expected, _value, _StdMemoryOrder(_order), _StdMemoryOrderFail(_order));
	}
	//!
	template<class T> bool AtomicCompareExchangeWeak(T& _atom, T _expected, T _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_compare_exchange_weak_explicit(reinterpret_cast<std::atomic<T>*>(&_atom), &_expected, _value, _StdMemoryOrder(_order), _StdMemoryOrderFail(_order));
	}
	//!
	template<class T> bool AtomicCompareExchangeWeak(T*& _atom, T* _expected, T* _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_compare_exchange_weak_explicit(reinterpret_cast<std::atomic<T*>*>(&_atom), &_expected, _value, _StdMemoryOrder(_order), _StdMemoryOrderFail(_order));
	}
	//!
	template<class T> bool AtomicCompareExchangeWeak(T& _atom, T* _expected, T _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_compare_exchange_weak_explicit(reinterpret_cast<std::atomic<T>*>(&_atom), _expected, _value, _StdMemoryOrder(_order), _StdMemoryOrderFail(_order));
	}
	//!
	template<class T> bool AtomicCompareExchangeWeak(T*& _atom, T** _expected, T* _value, MemoryOrder _order = MemoryOrder::Sequential)
	{
		return std::atomic_compare_exchange_weak_explicit(reinterpret_cast<std::atomic<T*>*>(&_atom), _expected, _value, _StdMemoryOrder(_order), _StdMemoryOrderFail(_order));
	}

	//! Atomic variable of numeric type
	template <class T> class Atomic
	{
	public:
		Atomic(T _value = static_cast<T>(0)) : m_value(_value) { }
		Atomic(const Atomic& _value) : m_value(_value.Get()) { }
		Atomic& operator = (const Atomic& _value) { return Set(_value.Get()); }
		Atomic& operator = (T _value) { return Set(_value); }
		operator T(void) const { return Get(); }

		T operator += (T _value) { return Add(_value) + _value; }
		T operator -= (T _value) { return Subtract(_value) - _value; }
		T operator &= (T _value) { return And(_value) & _value; }
		T operator |= (T _value) { return Or(_value) | _value; }
		T operator ^= (T _value) { return Xor(_value) ^ _value; }

		T operator ++ (void) { return Add(static_cast<T>(1)) + static_cast<T>(1); }
		T operator ++ (int) { return Add(static_cast<T>(1)); }
		T operator -- (void) { return Subtract(static_cast<T>(1)) - static_cast<T>(1); }
		T operator -- (int) { return Subtract(static_cast<T>(1)); }

		T Get(void) const { return AtomicGet(m_value); }
		T GetRaw(void) const { return m_value; }
		Atomic& Set(T _value, MemoryOrder _order = MemoryOrder::Sequential) { AtomicSet(m_value, _value); return *this; }
		T Add(T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicAdd(m_value, _value); }
		T Subtract(T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicSubtract(m_value, _value); }
		T And(T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicAnd(m_value, _value); }
		T Or(T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicOr(m_value, _value); }
		T Xor(T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicXor(m_value, _value); }
		T Exchange(T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicExchange(m_value, _value, _order); }
		bool CompareExchange(T _expected, T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchange(m_value, _expected, _value, _order); }
		bool CompareExchange(T* _expected, T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchange(m_value, _expected, _value, _order); }
		bool CompareExchangeWeak(T _expected, T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchangeWeak(m_value, _expected, _value, _order); }
		bool CompareExchangeWeak(T* _expected, T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchangeWeak(m_value, _expected, _value, _order); }

	protected:
		mutable T m_value;
	};

	//! Atomic variable of boolean
	template <> class Atomic<bool>
	{
	public:
		Atomic(bool _value = false) : m_value(_value) { }
		Atomic(const Atomic& _value) : m_value(_value.Get()) { }
		Atomic& operator = (const Atomic& _value) { return Set(_value.Get()); }
		Atomic& operator = (bool _value) { return Set(_value); }
		operator bool(void) const { return Get(); }

		bool Get(void) const { return AtomicGet(m_value); }
		bool GetRaw(void) const { return m_value; }
		Atomic& Set(bool _value, MemoryOrder _order = MemoryOrder::Sequential) { AtomicSet(m_value, _value); return *this; }
		bool Exchange(bool _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicExchange(m_value, _value, _order); }
		bool CompareExchange(bool _expected, bool _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchange(m_value, _expected, _value, _order); }
		bool CompareExchange(bool* _expected, bool _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchange(m_value, _expected, _value, _order); }
		bool CompareExchangeWeak(bool _expected, bool _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchangeWeak(m_value, _expected, _value, _order); }
		bool CompareExchangeWeak(bool* _expected, bool _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchangeWeak(m_value, _expected, _value, _order); }

	protected:
		mutable bool m_value;
	};

	//! Atomic variable of pointer
	template <class T> class Atomic<T*>
	{
	public:
		Atomic(T* _value = nullptr) : m_value(_value) { }
		Atomic(const Atomic& _value) : m_value(_value.Get()) { }
		Atomic& operator = (const Atomic& _value) { return Set(_value.Get()); }
		Atomic& operator = (T* _value) { return Set(_value); }
		operator T(void) const { return Get(); }

		T operator += (ptrdiff_t _value) { return Add(_value) + _value; }
		T operator -= (ptrdiff_t _value) { return Subtract(_value) - _value; }

		T* operator ++ (void) { return Add(1) + 1; }
		T* operator ++ (int) { return Add(1); }
		T* operator -- (void) { return Subtract(1) - 1; }
		T* operator -- (int) { return Subtract(1); }

		T* Get(void) const { return AtomicGet(m_value); }
		T* GetRaw(void) const { return m_value; }
		Atomic& Set(T* _value, MemoryOrder _order = MemoryOrder::Sequential) { AtomicSet(m_value, _value); return *this; }
		T* Add(ptrdiff_t _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicAdd(m_value, _value); }
		T* Subtract(ptrdiff_t _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicSubtract(m_value, _value); }
		T* Exchange(T* _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicExchange(m_value, _value, _order); }
		bool CompareExchange(T* _expected, T* _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchange(m_value, _expected, _value, _order); }
		bool CompareExchange(T** _expected, T* _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchange(m_value, _expected, _value, _order); }
		bool CompareExchangeWeak(T* _expected, T* _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchangeWeak(m_value, _expected, _value, _order); }
		bool CompareExchangeWeak(T** _expected, T* _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchangeWeak(m_value, _expected, _value, _order); }

	protected:
		mutable T* m_value;
	};

	//----------------------------------------------------------------------------//
	// SpinLock
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Math
	//----------------------------------------------------------------------------//



	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	struct IntVector2
	{
		union
		{
			int v[2];
			struct { int x, y; };
		};
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	struct IntVector3
	{
		union
		{
			int v[3];
			struct { int x, y, z; };
		};
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	struct IntVector4
	{
		union
		{
			int v[4];
			struct { int x, y, z, w; };
		};
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	struct Vector2
	{
		union
		{
			float v[2];
			struct { float x, y; };
		};
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	struct Vector3
	{
		union
		{
			float v[3];
			struct { float x, y, z; };
		};
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	struct Vector4
	{
		union
		{
			float v[4];
			struct { float x, y, z, w; };
		};
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	struct Quaternion
	{
		union
		{
			float v[4];
			struct { float x, y, z, w; };
		};
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	struct Matrix3x4
	{
		union
		{
			float v[12];
			float m[3][4]; // [row][column]
			struct
			{
				float m00, m01, m02, m03;
				float m10, m11, m12, m13;
				float m20, m21, m22, m23;
			};
		};
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	struct Matrix4x4
	{
		union
		{
			float v[16];
			float m[1][4]; // [row][column]
			struct
			{
				float m00, m01, m02, m03;
				float m10, m11, m12, m13;
				float m20, m21, m22, m23;
				float m30, m31, m32, m33;
			};
		};
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	struct Ray
	{
		Vector3 origin, dir;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	struct Plane
	{
		Vector3 normal;
		float dist;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	struct AlignedBox
	{
		Vector3 mn, mx;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	struct Frustum
	{
		Plane planes[6];
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	struct DbvNode
	{
		AlignedBox bounds;
		DbvNode* parent;
		union
		{
			DbvNode* child[2] = {0, 0};
			struct
			{
				void* object;
				bool isInternal;
			};
		};
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class DbvTree
	{
	public:

	protected:
		DbvNode* m_root;
		DbvNode* m_free;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}

