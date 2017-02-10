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

#include <math.h>

#include <new>
#include <atomic>
#include <initializer_list>

namespace Reax
{
	// grouping
	//!\defgroup Base Base
	//!\{
	//!\defgroup Container Template library
	//!\defgroup Math Math library
	//!\defgroup String	Strings
	//!\defgroup Concurrency Concurrency
	//!\}

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
	// Move semantics
	//----------------------------------------------------------------------------//

	//!\addtogroup Container 
	//!\{

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
		for (T* src = _src; src < _end;)
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
			uint _size = _currentSize;
			while (_size < _newSize)
				_size += (_size + 1) >> 1;
			return _size;
		}
		return _newSize;
	}

	//----------------------------------------------------------------------------//
	// InitializerList
	//----------------------------------------------------------------------------//

	//!
	template <class T> using InitializerList = std::initializer_list<T>;

	/*template <class T> class InitializerList
	{
	public:
		//!
		constexpr InitializerList(void) = default;
		//!
		constexpr InitializerList(std::initializer_list<T> _list) : m_first(_list.begin()), m_last(_list.end()) { }
		//!
		constexpr InitializerList(const T* _start, const T* _end) noexcept : m_first(_first), m_last(_last) { }
		//!
		constexpr InitializerList(const T* _start, size_t _count)  noexcept : m_first(_first), m_last(_first + _count) { }
		//!
		constexpr const T* Begin(void) const noexcept { return m_first; }
		//!
		constexpr const T* End(void) const noexcept { return m_last; }
		//!
		constexpr uint Size(void) const noexcept { return (uint)(m_last - m_first); }

	private:
		const T* m_first = nullptr;
		const T* m_last = nullptr;
	};

	//!
	template <class T> constexpr const T* begin(InitializerList<T> _list) noexcept { return _list.Begin(); }
	//!
	template <class T> constexpr const T* end(InitializerList<T> _list) noexcept { return _list.End(); }*/


	//----------------------------------------------------------------------------//
	// VoidType
	//----------------------------------------------------------------------------//

	template<class...> struct _CheckParams
	{	
		typedef void Type;
	};

	template<class... T> using VoidType = typename _CheckParams<T...>::Type;

	//----------------------------------------------------------------------------//
	// Iterator	utilities
	//----------------------------------------------------------------------------//

	//!
	enum class IteratorType
	{
		Array,
		Generic,
	};

	//! Tag for random acess iterators
	enum class ArrayIteratorTag { ID = (int)IteratorType::Array };
	//! Tag for other iterators
	enum class GenericIteratorTag { ID = (int)IteratorType::Generic };

	//! Classify iterator type
	template <class T, class = void> struct IteratorTag { static const auto Type = GenericIteratorTag::ID; };
	//! Classify iterator type
	template <class T> struct IteratorTag<T*> { static const auto Type = ArrayIteratorTag::ID; };

	//!
	template <class T> bool IsArrayIterator(void) { return IteratorTag<T>::Type == static_cast<decltype(IteratorTag<T>::Type)>(IteratorType::Array); }
	//!
	template <class T> bool IsGenericIterator(void) { return IteratorTag<T>::Type == static_cast<decltype(IteratorTag<T>::Type)>(IteratorType::Generic); }

	//! Get distance between two array iterators. 
	template <class T> uint _IteratorDistance(T _start, T _end, ArrayIteratorTag) { return (uint)(_end - _start); }
	//! Get distance between two generic iterators. 
	template <class T> uint _IteratorDistance(T _start, T _end, GenericIteratorTag)
	{
		uint _dist = 0;
		while (_start != _end) ++_start, ++_dist;
		return _dist;
	}
	//! Get distance between two iterators. 
	template <class T> uint IteratorDistance(T _start, T _end) { return _IteratorDistance(_start, _end, IteratorTag<T>::Type); }
	
	//! Get tag for std iterator
	template <class T> struct _StdIteratorCategory { static const GenericIteratorTag Type = GenericIteratorTag::ID; };
	//! Get tag for std iterator
	template <> struct _StdIteratorCategory<std::random_access_iterator_tag> { static const ArrayIteratorTag Type = ArrayIteratorTag::ID; };
	//! Classify std iterators type
	template <class T> struct IteratorTag<T, VoidType<typename T::iterator_category>> { static const auto Type = _StdIteratorCategory<T::iterator_category>::Type; };

	//----------------------------------------------------------------------------//
	// ArrayIterator
	//----------------------------------------------------------------------------//

	//! Iterator with random access.
	template <class T> class ArrayIterator
	{
	public:
		//!
		ArrayIterator(T* _ptr = nullptr) : m_ptr(_ptr) { }
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

	//!
	template <class T> struct IteratorTag<ArrayIterator<T>> { static const auto Type = ArrayIteratorTag::ID; };

	//----------------------------------------------------------------------------//
	// Array
	//----------------------------------------------------------------------------//

	//! Dynamic array.
	template <class T> class Array
	{
	public:
		typedef ArrayIterator<T> Iterator;
		typedef ArrayIterator<const T> ConstIterator;

		//!	Default constructor.
		Array(void) = default;
		//!	Destructor.
		~Array(void) { Free(); }
		//!	Copy constructor.
		Array(const Array& _rhs) { Push(_rhs.m_data, _rhs.m_size); }
		//! Move constructor.
		Array(Array&& _rhs) : m_size(_rhs.m_size), m_capacity(_rhs.m_capacity), m_data(_rhs.m_data)
		{
			_rhs.m_size = 0;
			_rhs.m_capacity = 0;
			_rhs.m_data = nullptr;
		}
		//! Fill constructor.
		explicit Array(uint _size) { Resize(_size); }
		//! Fill constructor.
		Array(uint _size, const T& _value) { Resize(_size, _value); }
		//! Range constructor.
		template <class I> Array(I _start, I _end) { Push(_start, _end); }
		//! Initializer list constructor.
		Array(InitializerList<T> _list) { Push(_list); }

		//! Copy assignment.
		Array& operator = (const Array& _rhs)
		{
			if (m_data != _rhs.m_data)
				Clear().Push(_rhs.m_data, _rhs.m_size);
			return *this;
		}
		//!	Move assignment.
		Array& operator = (Array&& _rhs)
		{
			Swap(m_size, _rhs.m_size);
			Swap(m_capacity, _rhs.m_capacity);
			Swap(m_data, _rhs.m_data);
			return *this;
		}
		//! Initializer list assignment. 
		Array& operator = (InitializerList<T> _rhs) { return Clear().Push(_rhs); }

		// [capacity]

		//!	Get size of array.
		uint Size(void) const { return m_size; }
		//!	Get real size of array.
		uint Capacity(void) const { return m_capacity; }
		//!	Get number of unused elements.
		uint Unused(void) const { return m_capacity - m_size; }
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
					_Realloc(GrowTo(m_capacity, _size));
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
					_Realloc(GrowTo(m_capacity, _size));
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
			if (_size > m_capacity || (_compact && _size != m_capacity))
				_Realloc(_size);
			return *this;
		}
		//! Reallocate array and delete unused elements.
		Array& Compact(void)
		{
			if (m_size != m_capacity)
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
			ASSERT(m_size != 0, "Empty container");
			return m_data[0];
		}
		//!	Get first element.
		const T& Front(void) const
		{
			ASSERT(m_size != 0, "Empty container");
			return m_data[0];
		}
		//!	Get last element.
		T& Back(void)
		{
			ASSERT(m_size != 0, "Empty container");
			return m_data[m_size - 1];
		}
		//!	Get last element.
		const T& Back(void) const
		{
			ASSERT(m_size != 0, "Empty container");
			return m_data[m_size - 1];
		}
		//!	Get data.
		T* Data(void) { return m_data; }
		//!	Get data.
		const T* Data(void) const { return m_data; }

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
		Iterator GetIterator(uint _index) { return Iterator(m_data + (_index < m_size ? _index : m_size)); }
		//! Get iterator from index
		ConstIterator GetIterator(uint _index) const { return ConstIterator(m_data + (_index < m_size ? _index : m_size)); }

		// [modifiers]

		//! Add one element to end of array.
		Array& Push(const T& _value)
		{
			if (m_size == m_capacity)
				_Reserve(1);
			Construct(m_data + m_size, _value);
			++m_size;
			return *this;
		}
		//! Add one element to end of array.
		Array& Push(T&& _value)
		{
			if (m_size == m_capacity)
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
		//! Append elements to end of array.
		template <class I> Array& Push(I _start, I _end) { return _Push(_start, _end, IteratorTag<I>::Type); }
		//! Append elements to end of array.
		Array& Push(const InitializerList<T>& _list) { return Push(_list.begin(), (uint)_list.size()); }

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
		Iterator Erase(ConstIterator _pos) { return Erase(_pos - m_data, 1); }
		//!	Erase a range of elements by iterators. \return iterator to the next element. 
		Iterator Erase(ConstIterator _start, ConstIterator _end) { return Erase(_start - m_data, _end - m_data); }
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
		Iterator FastErase(ConstIterator _pos) { return FastErase(_pos - m_data); }
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
		Iterator Insert(ConstIterator _pos, const T* _src, uint _count) { return Insert(_pos - m_data, _src, _count); }
		//! Insert an elements in position.
		Iterator Insert(uint _pos, const T* _src, uint _count)
		{
			if (_pos > m_size)
				_pos = m_size;

			if (Unused() < _count)
			{
				uint _newSize = m_size + _count;
				uint _newCaps = GrowTo(m_capacity, _newSize);
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
				m_capacity = _newCaps;
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
		template<class... Args> Iterator Emplace(ConstIterator _pos, Args&&... _args) { return Emplace(_pos - m_data, Forward<Args>(_args)...); }
		//! Construct new element in position.
		template<class... Args> Iterator Emplace(uint _pos, Args&&... _args)
		{
			if (_pos > m_size)
				_pos = m_size;

			if (m_size == m_capacity)
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
			return End();
		}
		//! Search first element.
		ConstIterator Find(const T& _value)	const
		{
			for (ConstIterator i = Begin(), e = End(); i != e; ++i)
			{
				if (*i == _value)
					return i;
			}
			return End();
		}
		//! Search next element.
		Iterator Find(ConstIterator _start, const T& _value)
		{
			ASSERT(Index(_start) <= m_size, "Invalid iterator");
			for (Iterator i = _start, e = End(); i != e; ++i)
			{
				if (*i == _value)
					return i;
			}
			return End();
		}
		//! Search next element.
		ConstIterator Find(ConstIterator _start, const T& _value) const
		{
			ASSERT(Index(_start) <= m_size, "Invalid iterator");
			for (ConstIterator i = _start, e = End(); i != e; ++i)
			{
				if (*i == _value)
					return i;
			}
			return End();
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
			m_capacity = _size;
		}
		//! Do reserve new elements.
		void _Reserve(uint _append)
		{
			if (Unused() < _append)
				_Realloc(GrowTo(m_capacity, m_size + _append));
		}
		//! 
		template <class I> Array& _Push(I _start, I _end, ArrayIteratorTag)
		{
			return Push(&(*_start), IteratorDistance(_start, _end));
		}
		//!
		template <class I> Array& _Push(I _start, I _end, GenericIteratorTag)
		{
			_Reserve(IteratorDistance(_start, _end));
			while (_start != _end)
				Push(*_start++);
			return *this;
		}

		//! Number of used elements.
		uint m_size = 0;
		//! Number of all elements.
		uint m_capacity = 0;
		//! The data.
		T* m_data = nullptr;
	};

	//!
	template <class T> auto begin(Array<T>& _array)->decltype(_array.Begin()) { return _array.Begin(); }
	//!
	template <class T> auto begin(const Array<T>& _array)->decltype(_array.Begin()) { return _array.Begin(); }
	//!
	template <class T> auto end(Array<T>& _array)->decltype(_array.End()) { return _array.End(); }
	//!
	template <class T> auto end(const Array<T>& _array)->decltype(_array.End()) { return _array.End(); }

	//----------------------------------------------------------------------------//
	// ListIterator
	//----------------------------------------------------------------------------//

	//! Iterator for double-linked nodes.
	template <class T> class ListIterator
	{
	public:
		typedef typename BaseType<decltype(reinterpret_cast<T*>(0)->Value())>::Type ValueType;
		//!
		ListIterator(T* _ptr = nullptr) : m_ptr(_ptr) { }

		//!	Implicit conversion from non-const iterator to const iterator.
		operator const typename ListIterator<const T>(void) const { return ListIterator<const T>(m_ptr); }

		//!	Get address.
		ValueType* operator & (void) { ASSERT(m_ptr != nullptr); return &(m_ptr->Value()); }
		//!	Get address.
		ValueType* operator -> (void) { ASSERT(m_ptr != nullptr); return &(m_ptr->Value()); }
		//!	Get reference.
		ValueType& operator * (void) { ASSERT(m_ptr != nullptr); return m_ptr->Value(); }
		//!	Get node.
		T* Node(void) { return m_ptr; }

		//!	Go to next element.
		ListIterator& operator ++ (void) { ASSERT(m_ptr != nullptr); m_ptr = m_ptr->Next(); return *this; }
		//!	Go to next element.
		ListIterator operator ++ (int) { ASSERT(m_ptr != nullptr); ListIterator _next(m_ptr); m_ptr = m_ptr->Next(); return _next; }
		//!	Go to prev element.
		ListIterator& operator -- (void) { ASSERT(m_ptr != nullptr); m_ptr = m_ptr->Prev(); return *this; }
		//!	Go to prev element.
		ListIterator operator -- (int) { ASSERT(m_ptr != nullptr); ListIterator _prev(m_ptr); m_ptr = m_ptr->Prev(); return _prev; }

		//!
		bool operator == (const ListIterator& _rhs) const { return m_ptr == _rhs.m_ptr; }
		//!
		bool operator != (const ListIterator& _rhs) const { return m_ptr != _rhs.m_ptr; }

	protected:

		T* m_ptr;
	};

	//!
	template <class T> struct IteratorTag<ListIterator<T>> { static const auto Type = GenericIteratorTag::ID; };

	//----------------------------------------------------------------------------//
	// List
	//----------------------------------------------------------------------------//

	//! Double-linked list.
	template <class T> class List
	{
	public:
		//!
		struct Node;

		//!
		struct NodeBase
		{
			//!
			Node* Next(void) { return m_next; }
			//!
			const Node* Next(void) const { return m_next; }
			//!
			Node* Prev(void) { return m_prev; }
			//!
			const Node* Prev(void) const { return m_prev; }

		protected:
			friend class List;
			//!
			Node* m_prev;
			//!
			Node* m_next;
		};

		//!
		struct Node : public NodeBase
		{
			//!
			T& Value(void) { return m_value; }
			//!
			const T& Value(void) const { return m_value; }

		protected:
			friend class List;
			//!
			T m_value;
		};

		//!
		typedef ListIterator<Node> Iterator;
		//!
		typedef ListIterator<const Node> ConstIterator;

		//!
		List(void) { _Reset(); }
		//!
		~List(void) { Clear(); }
		//! 
		List(const List& _other) : List()
		{
			for (const T& i : _other)
				Push(i);
		}
		//!
		List(List&& _temp) : m_size(_temp.m_size)
		{
			Node* _first = _temp.m_tail.m_next;
			Node* _last = _temp.m_tail.m_prev;
			if (_first != _last)
			{
				_Bind(_first, _last);
				_temp._Reset();
			}
			else
				_Reset();
		}
		//!
		List(InitializerList<T> _list)
		{
			_Reset();
			for (const T& i : _list)
				Push(i);
		}

		//!
		List& operator = (const List& _rhs)
		{
			if (&m_tail != _rhs.m_tail)
			{
				Clear();
				for (const T& i : _rhs)
					Push(i);
			}
			return *this;
		}
		//!
		List& operator = (List&& _rhs)
		{
			Node* _first = _rhs.m_tail.m_next;
			Node* _last = _rhs.m_tail.m_prev;
			uint _size = _rhs.m_size;

			if (m_size)
			{
				_rhs._Bind(m_tail.m_next, m_tail.m_prev);
				_rhs.m_size = m_size;
			}
			else
				_rhs._Reset();

			if (_first != _last)
			{
				_Bind(_first, _last);
				m_size = _size;
			}
			else
				_Reset();

			return *this;
		}
		//!
		List& operator = (InitializerList<T> _rhs)
		{
			Clear();
			for (const T& i : _rhs)
				Push(i);
			return *this;
		}

		//!
		uint Size(void) const { return m_size; }
		//!
		bool IsEmpty(void) const { return m_size == 0; }
		//!
		bool NonEmpty(void) const { return m_size != 0; }
		//!
		List& Clear(void)
		{
			while (m_size)
				_Erase(m_tail.m_next);
			return *this;
		}


		//!
		T& Front(void) { ASSERT(m_size != 0); return m_tail.m_next->m_value; }
		//!
		const T& Front(void) const { ASSERT(m_size != 0); return m_tail.m_next->m_value; }
		//!
		T& Back(void) { ASSERT(m_size != 0); return m_tail.m_prev->m_value; }
		//!
		const T& Back(void) const { ASSERT(m_size != 0); return m_tail.m_prev->m_value; }

		//!
		Iterator Begin(void) { return m_tail.m_next; }
		//!
		ConstIterator Begin(void) const { return m_tail.m_next; }
		//!
		Iterator End(void) { return Iterator(static_cast<Node*>(&m_tail)); }
		//!
		ConstIterator End(void) const { return ConstIterator(static_cast<const Node*>(&m_tail)); }


		//!
		List& PushFront(const T& _value)
		{
			Node* _newNode = _Insert(m_tail.m_next);
			Construct(&_newNode->m_value, _value);
			return *this;
		}
		//!
		List& PushFront(T&& _value)
		{
			Node* _newNode = _Insert(m_tail.m_next);
			Construct(&_newNode->m_value, Forward<T>(_value));
			return *this;
		}
		//!
		List& Push(const T& _value)
		{
			Node* _newNode = _Insert(&m_tail);
			Construct(&_newNode->m_value, _value);
			return *this;
		}
		//!
		List& Push(T&& _value)
		{
			Node* _newNode = _Insert(&m_tail);
			Construct(&_newNode->m_value, Forward<T>(_value));
			return *this;
		}

		//!
		List& PopFront(void)
		{
			if (m_size)
				_Erase(m_tail.m_next);
			return *this;
		}
		//!
		List& Pop(void)
		{
			if (m_size)
				_Erase(m_tail.m_prev);
			return *this;
		}

		//!
		Iterator Insert(ConstIterator _pos, const T& _value)
		{
			ASSERT(_pos.Node() != nullptr);
			Node* _newNode = _Insert(_pos);
			Construct(&_newNode->m_value, _value);
			return _newNode;
		}
		//!
		Iterator Erase(ConstIterator _pos)
		{
			ASSERT(_pos.Node() != nullptr);
			if (_pos.Node() != &m_tail)
				return _Erase(_pos.Node());
			return End();
		}

		TODO_EX("List", "Add insert");
		TODO_EX("List", "Add Emplace, EmplaceFront, EmplaceBack");
		TODO_EX("List", "Add Splice, Merge, Sort, Reverse, Unique");


		//!
		bool operator == (const List& _rhs) const
		{
			if (m_size != _rhs.m_size)
				return false;
			if (&m_tail == &_rhs.m_tail)
				return true;
			for (ConstIterator l = Begin(), r = _rhs.Begin(), e = End(); l != e; ++l, ++r)
			{
				if (*l != *r)
					return false;
			}
			return true;
		}
		//!
		bool operator != (const List& _rhs) const { return !(*this == _rhs); }


		//!
		Iterator Find(const T& _value) { return Find(Begin(), _value); }
		//!
		ConstIterator Find(const T& _value) const { return Find(Begin(), _value); }
		//!
		Iterator Find(ConstIterator _start, const T& _value)
		{
			for (ConstIterator i = _start, e = End(); i != e; ++i)
			{
				if (*i == _value)
					return i;
			}
			return End();
		}
		//!
		ConstIterator Find(ConstIterator _start, const T& _value) const
		{
			for (ConstIterator i = _start, e = End(); i != e; ++i)
			{
				if (*i == _value)
					return i;
			}
			return End();
		}
		//!
		Iterator Remove(const T& _value) { return Erase(Find(_value)); }
		//!
		Iterator Remove(ConstIterator _start, const T& _value) { return Erase(Find(_start, _value)); }

	protected:
		//!
		void _Reset(void)
		{
			m_size = 0;
			m_tail.m_next = static_cast<Node*>(&m_tail); // head --> tail
			m_tail.m_prev = static_cast<Node*>(&m_tail); // tail <-- head
		}
		//!
		void _Bind(Node* _first, Node* _last)
		{
			ASSERT(_first != _last);
			ASSERT(_first != nullptr);
			ASSERT(_last != nullptr);

			m_tail.m_next = _first;
			_first->m_prev = static_cast<Node*>(&m_tail);
			m_tail.m_prev = _last;
			_last->m_next = static_cast<Node*>(&m_tail);
		}
		//!
		Node* _Insert(NodeBase* _dest)
		{
			ASSERT(_dest != nullptr);

			Node* _newNode = Allocate<Node>(1);
			_newNode->m_next = static_cast<Node*>(_dest);
			_newNode->m_prev = _dest->m_prev;
			_dest->m_prev->m_next = _newNode;
			_dest->m_prev = _newNode;

			++m_size;

			return _newNode;
		}
		//!
		Node* _Erase(Node* _node)
		{
			ASSERT(_node != nullptr && _node != &m_tail);
			Node* _next = _node->m_next;
			_node->m_next->m_prev = _node->m_prev;
			_node->m_prev->m_next = _node->m_next;
			--m_size;

			Destroy(&_node->m_value);
			Deallocate(_node);

			return _next; // ?
		}

		uint m_size;
		NodeBase m_tail;
	};

	//!
	template <class T> auto begin(List<T>& _list)->decltype(_list.Begin()) { return _list.Begin(); }
	//!
	template <class T> auto begin(const List<T>& _list)->decltype(_list.Begin()) { return _list.Begin(); }
	//!
	template <class T> auto end(List<T>& _list)->decltype(_list.End()) { return _list.End(); }
	//!
	template <class T> auto end(const List<T>& _list)->decltype(_list.End()) { return _list.End(); }

	//----------------------------------------------------------------------------//
	// Pair
	//----------------------------------------------------------------------------//

	//! Pair. 
	template <class T, class U> struct Pair
	{
		T first;
		U second;

		Pair(void) = default;
		Pair(const T& _first, const U& _second) : first(_first), second(_second) { }
		Pair(T&& _first, U&& _second) : first(Forward<T>(_first)), second(Forward<U>(_second)) { }
	};

	//----------------------------------------------------------------------------//
	// Hash 
	//----------------------------------------------------------------------------//

	//! SDMB hash
	inline uint Hash(const void* _data, uint _size, uint _hash = 0)
	{
		const uint8* _start = reinterpret_cast<const uint8*>(_data);
		const uint8* _end = _start + _size;
		while (_start < _end)
			_hash = *_start++ + +(_hash << 6) + (_hash << 16) - _hash;
		return _hash;
	}

	//! Generic hash function.
	template <class T> inline uint MakeHash(const T& _value) { return Hash(&_value, sizeof(_value)); }
	//!
	template <> inline uint MakeHash(const char& value) { return (uint)value; }
	//!
	template <> inline uint MakeHash(const unsigned char& value) { return (uint)value; }
	//!
	template <> inline uint MakeHash(const int16& _value) { return (uint)_value; }
	//!
	template <> inline uint MakeHash(const uint16& _value) { return (uint)_value; }
	//!
	template <> inline uint MakeHash(const uint& _value) { return (uint)_value; }
	//!
	template <> inline uint MakeHash(const int& _value) { return (uint)_value; }
	//!
	template <> inline uint MakeHash(const int64& _value) { return (uint)((_value >> 32) | (_value & 0xffffffff)); }
	//!
	template <> inline uint MakeHash(const uint64& _value) { return (uint)((_value >> 32) | (_value & 0xffffffff)); }
	//!
	inline uint MakeHash(void* _value) { return (uint)(size_t)_value; }
	//!
	inline uint MakeHash(const void* _value) { return (uint)(size_t)_value; }
	//!
	template <class T> inline uint MakeHash(T* _value) { return (uint)((size_t)_value / sizeof(T)); }
	//!
	template <class T> inline uint MakeHash(const T* _value) { return (uint)((size_t)_value / sizeof(T)); }

	//----------------------------------------------------------------------------//
	// HashMap
	//----------------------------------------------------------------------------//

	//! Unordered associative array.
	template <class T, class U> class HashMap
	{
	public:
		//! Max size of bucket = 2^(MAX_BUCKET_SIZE_POW2) (0=1, 1=2, 2=4, 3=8 ...)
		static const uint MAX_BUCKET_SIZE_POW2 = 0;
		//!
		typedef Pair<const T, U> KeyValue;

		//!
		struct Node;

		//!
		struct NodeBase
		{
			//!
			Node* Next(void) { return m_next; }
			//!
			const Node* Next(void) const { return m_next; }
			//!
			Node* Prev(void) { return m_prev; }
			//!
			const Node* Prev(void) const { return m_prev; }

		protected:
			friend class HashMap;
			//!
			Node* m_prev;
			//!
			Node* m_next;
		};

		//!
		struct Node : public NodeBase
		{
			//!
			KeyValue& Value(void) { return m_value; }
			//!
			const KeyValue& Value(void) const { return m_value; }
			//!
			Node* Down(void) { return m_down; }
			//!
			const Node* Down(void) const { return m_down; }

		protected:
			friend class HashMap;
			//!
			Node* m_down;
			//!
			uint m_hash;
			//!
			KeyValue m_value;
		};

		//!
		typedef ListIterator<Node> Iterator;
		//!
		typedef ListIterator<const Node> ConstIterator;

		//!
		HashMap(void) { _ResetList(); }
		//!
		~HashMap(void)
		{
			Clear();
			delete[] m_buckets;
		}
		//!
		HashMap(const HashMap& _other)
		{
			_ResetList();
			Insert(_other.Begin(), _other.End());
		}
		//!
		HashMap(HashMap&& _temp) : m_size(_temp.m_size)
		{
			Node* _first = _temp.m_tail.m_next;
			Node* _last = _temp.m_tail.m_prev;
			if (_first != _last)
			{
				_BindList(_first, _last);
				_temp._ResetList();
				Swap(m_buckets, _temp.m_buckets);
				Swap(m_numBuckets, _temp.m_numBuckets);
			}
			else
				_ResetList();
		}
		//!
		HashMap(ConstIterator _start, ConstIterator _end) : HashMap() { Insert(_start, _end); }
		//!
		HashMap(InitializerList<KeyValue> _list) : HashMap() { Insert(_list); }

		//!
		HashMap& operator = (const HashMap& _rhs)
		{
			if (m_buckets != _rhs.m_buckets)
				Clear().Insert(_rhs.Begin(), _rhs.End());
			return *this;
		}
		//!
		HashMap& operator = (HashMap&& _rhs)
		{
			Node* _first = _rhs.m_tail.m_next;
			Node* _last = _rhs.m_tail.m_prev;
			uint _size = _rhs.m_size;

			if (m_size)
			{
				_rhs._BindList(m_tail.m_next, m_tail.m_prev);
				_rhs.m_size = m_size;
			}
			else
				_rhs._ResetList();

			if (_first != _last)
			{
				_BindList(_first, _last);
				m_size = _size;
			}
			else
				_ResetList();

			Swap(m_buckets, _rhs.m_buckets);
			Swap(m_numBuckets, _rhs.m_numBuckets);

			return *this;
		}
		//!
		HashMap& operator = (InitializerList<KeyValue> _list) { return Clear().Insert(_list); }


		//!
		const T& Front(void) const { ASSERT(m_size != 0); return m_tail.m_next->m_value.first; }
		//!
		const T& Back(void) const { ASSERT(m_size != 0); return m_tail.m_prev->m_value.first; }

		//!
		Iterator Begin(void) { return m_tail.m_next; }
		//!
		ConstIterator Begin(void) const { return m_tail.m_next; }
		//!
		Iterator End(void) { return static_cast<Node*>(&m_tail); }
		//!
		ConstIterator End(void) const { return static_cast<const Node*>(&m_tail); }


		//!
		uint Size(void) const { return m_size; }
		//!
		bool IsEmpty(void) const { return m_size == 0; }
		//!
		bool NonEmpty(void) const { return m_size != 0; }
		//!
		HashMap& Clear(void)
		{
			while (m_size)
			{
				Node* _head = m_tail.m_next;
				Node* _next = _head->m_next;
				_head->m_next->m_prev = _head->m_prev;
				_head->m_prev->m_next = _head->m_next;
				--m_size;
			}
			memset(m_buckets, 0, m_numBuckets * sizeof(m_buckets[0]));
			return *this;
		}

		//!
		uint BucketCount(void) const { return m_numBuckets; }

		//!
		U& operator[] (const T& _key)
		{
			uint _hash = MakeHash(_key);
			Node* _node = _Find(_key, _hash);
			if (!_node)
			{
				_node = _Insert(_hash);
				Construct(const_cast<T*>(&_node->m_value.first), _key);
				Construct(&_node->m_value.second);
			}
			return _node->m_value.second;
		}
		//!
		U& operator[] (T&& _key)
		{
			uint _hash = MakeHash(_key);
			Node* _node = _Find(_key, _hash);
			if (!_node)
			{
				_node = _Insert(_hash);
				Construct(const_cast<T*>(&_node->m_value.first), Forward<T>(_key));
				Construct(&_node->m_value.second);
			}
			return _node->m_value.second;
		}

		//!
		Iterator Insert(const KeyValue& _value)
		{
			uint _hash = MakeHash(_value.first);
			Node* _node = _Find(_value.first, _hash);
			if (!_node)
			{
				_node = _Insert(_hash);
				Construct(const_cast<T*>(&_node->m_value.first), _value.first);
				Construct(&_node->m_value.second, _value.second);
			}
			else
				_node->m_value.second = _value.second;
			return _node;
		}
		//!
		Iterator Insert(const KeyValue&& _value)
		{
			uint _hash = MakeHash(_value.first);
			Node* _node = _Find(_value.first, _hash);
			if (!_node)
			{
				_node = _Insert(_hash);
				Construct(const_cast<T*>(&_node->m_value.first), Forward<T>(_value.first));
				Construct(&_node->m_value.second, Forward<U>(_value.second));
			}
			else
				_node->m_value.second = _value.second;
			return _node;
		}
		//!
		Iterator Insert(ConstIterator _iter)
		{
			ASSERT(_iter.Node() != nullptr);
			Node* _insert = _iter.Node();
			uint _hash = _node->m_hash;
			Node* _node = _Find(_insert->m_value.first, _hash);
			if (!_node)
			{
				_node = _Insert(_hash);
				Construct(const_cast<T*>(&_newNode->m_value.first), _insert->m_value.first);
				Construct(&_newNode->m_value.second, _insert->m_value.second);
			}
			else
				_node->m_value.second = _insert->m_value.second;

			return _node;
		}
		//!
		Iterator Insert(ConstIterator _start, ConstIterator _end)
		{
			if (_start != _end)
			{
				Iterator _r = Insert(_start++);
				for (; _start != _end; ++_start)
					Insert(_start);
				return _r;
			}
			return End();
		}
		//!
		Iterator Insert(InitializerList<KeyValue> _list)
		{
			if (_list.size())
			{
				const KeyValue* _start = _list.begin();
				Iterator _r = Insert(*_start++);
				for (; _start != _list.end(); ++_start)
					Insert(*_start);
				return _r;
			}
			return End();
		}

		//!
		Iterator Erase(const T& _key)
		{
			uint _hash = MakeHash(_key);
			uint _index = _hash & (m_numBuckets - 1);
			Node* _prev = nullptr;
			Node* _node = _Find(_key, _index, _prev);
			if (_node)
				return _Erase(_node, _index, _prev);
			return End();
		}
		//!
		Iterator Erase(ConstIterator _pos)
		{
			Node* _node = _pos.Node();
			ASSERT(_node != nullptr);
			if (_pos != &m_tail)
			{
				uint _index = _node->m_hash & (m_numBuckets - 1);
				return _Erase(_node, _index, _Prev(m_buckets[_index], _node));
			}
			return End();
		}

		//!
		Iterator Find(const T& _key)
		{
			uint _hash = MakeHash(_key);
			Node* _node = _Find(_key, _hash);
			return _node ? _node : End();
		}
		//!
		ConstIterator Find(const T& _key) const
		{
			uint _hash = MakeHash(_key);
			Node* _node = _Find(_key, _hash);
			return _node ? _node : End();
		}
		//!
		bool Contains(const T& Key) const
		{
			return _Find(_key, MakeHash(_key)) != nullptr;
		}

	protected:
		//!
		void _ResetList(void)
		{
			m_size = 0;
			m_tail.m_next = static_cast<Node*>(&m_tail); // head --> tail
			m_tail.m_prev = static_cast<Node*>(&m_tail); // tail <-- head
		}
		//!
		void _BindList(Node* _first, Node* _last)
		{
			ASSERT(_first != _last);
			ASSERT(_first != nullptr);
			ASSERT(_last != nullptr);

			m_tail.m_next = _first;
			_first->m_prev = static_cast<Node*>(&m_tail);
			m_tail.m_prev = _last;
			_last->m_next = static_cast<Node*>(&m_tail);
		}
		//!
		Node* _Find(const T& _key, uint _hash) const
		{
			for (Node* _node = m_buckets ? m_buckets[_hash & (m_numBuckets - 1)] : nullptr; _node; _node = _node->m_down)
			{
				if (_node->m_value.first == _key)
					return _node;
			}
			return nullptr;
		}
		//!
		Node* _Find(const T& _key, uint _index, Node*& _prev) const
		{
			for (Node* _node = m_buckets ? m_buckets[_index] : nullptr; _node; _prev = _node, _node = _node->m_down)
			{
				if (_node->m_value.first == _key)
					return _node;
			}
			return nullptr;
		}
		//!
		Node* _Prev(Node* _bucket, Node* _node)
		{
			for (Node* _prev = nullptr; _bucket; _prev = _bucket, _bucket = _bucket->m_down)
			{
				if (_bucket == _node)
					return _prev;
			}
			return nullptr;
		}
		//!
		void _Rehash(uint _newSize)
		{
			Node** _newData = new Node*[_newSize];
			memset(_newData, 0, _newSize * sizeof(Node*));

			delete[] m_buckets;
			m_buckets = _newData;
			m_numBuckets = _newSize;

			uint _mask = m_numBuckets - 1;
			for (Node* i = m_tail.m_next; i != &m_tail; i = i->m_next)
			{
				uint _index = i->m_hash & _mask;
				i->m_down = m_buckets[_index];
				m_buckets[_index] = i;
			}
		}
		//!
		Node* _Insert(uint _hash)
		{
			if (!m_buckets)
			{
				m_numBuckets = 4; // min num buckets
				m_buckets = new Node*[4];
				memset(m_buckets, 0, m_numBuckets * sizeof(Node*));
			}

			uint _index = _hash & (m_numBuckets - 1);
			Node* _newNode = Allocate<Node>(1);
			_newNode->m_hash = _hash;

			_newNode->m_next = static_cast<Node*>(&m_tail);
			_newNode->m_prev = m_tail.m_prev;
			m_tail.m_prev->m_next = _newNode;
			m_tail.m_prev = _newNode;

			_newNode->m_down = m_buckets[_index];
			m_buckets[_index] = _newNode;

			++m_size;

			if (m_size > m_numBuckets << MAX_BUCKET_SIZE_POW2)
				_Rehash(m_numBuckets << 1);

			return _newNode;
		}
		//!
		Node* _Erase(Node* _node, uint _index, Node* _prev)
		{
			ASSERT(_node != nullptr);
			ASSERT(_node != &m_tail);

			if (_prev)
				_prev->m_down = _node->m_down;
			else
				m_buckets[_index] = _node->m_down;

			Node* _next = _node->m_next;
			_node->m_next->m_prev = _node->m_prev;
			_node->m_prev->m_next = _node->m_next;
			--m_size;

			Destroy(&_node->m_value);
			Deallocate(_node);

			return _next;
		}


		NodeBase m_tail;
		Node** m_buckets = nullptr;
		uint m_numBuckets = 0;
		uint m_size;
	};

	//!
	template <class T, class U> auto begin(HashMap<T, U>& _map)->decltype(_map.Begin()) { return _map.Begin(); }
	//!
	template <class T, class U> auto begin(const HashMap<T, U>& _map)->decltype(_map.Begin()) { return _map.Begin(); }
	//!
	template <class T, class U> auto end(HashMap<T, U>& _map)->decltype(_map.End()) { return _map.End(); }
	//!
	template <class T, class U> auto end(const HashMap<T, U>& _map)->decltype(_map.End()) { return _map.End(); }

	//!\} Container

	//----------------------------------------------------------------------------//
	// String
	//----------------------------------------------------------------------------//

	//!\addtogroup String
	//!\{

	//! Multibyte string.
	class String
	{
	public:
		//!
		//typedef char* Iterator;
		//!
		//typedef const char* ConstIterator;
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

	//! Unicode string. \ingroup String
	class RX_API WString
	{
	public:


	protected:
		uint m_length = 0;
		uint m_capacity = 0;
		wchar_t* m_data = nullptr;
	};

	//!\} String

	//----------------------------------------------------------------------------//
	// Atomic
	//----------------------------------------------------------------------------//

	//!\addtogroup Concurrency
	//!\{

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

	//!\} Concurrency

	//----------------------------------------------------------------------------//
	// SpinLock
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// Math
	//----------------------------------------------------------------------------//

	//!\addtogroup Math
	//!\{

	typedef float float32;
	typedef struct half float16;
	typedef double float64;

	static const float EPSILON = 1e-6f;
	static const float EPSILON2 = 1e-12f;
	static const float PI = 3.1415926535897932384626433832795f;
	static const float DEGREES = 57.295779513082320876798154814105f;
	static const float RADIANS = 0.01745329251994329576923690768489f;

	//! Get minimum value
	template <typename T> const T& Min(const T& _a, const T& _b) { return _a < _b ? _a : _b; }
	//! Get minimum value
	template <typename T> const T& Min(const T& _a, const T& _b, const T& _c) { return _a < _b ? (_a < _c ? _a : _c) : (_b < _c ? _b : _c); }
	//! Get maximum value
	template <typename T> const T& Max(const T& _a, const T& _b) { return _a > _b ? _a : _b; }
	//! Get maximum value
	template <typename T> const T& Max(const T& _a, const T& _b, const T& _c) { return _a > _b ? (_a > _c ? _a : _c) : (_b > _c ? _b : _c); }
	//! 
	template <typename T> const T Clamp(T _x, T _l, T _u) { return _x > _l ? (_x < _u ? _x : _u) : _l; }
	//! 
	template <typename T> const T Clamp01(T _x, T _l, T _u) { return Clamp<T>(_x, T(0), T(1)); }
	//! Linear interpolation
	template <typename T> T Mix(const T& _a, const T& _b, float _t) { return _a + (_b - _a) * _t; }
	//! 
	template <typename T> T Abs(T _x) { return abs(_x); }
	//! 
	template <typename T> T Radians(T _degrees) { return _degrees * RADIANS; }
	//! 
	template <typename T> T Degrees(T _radians) { return _radians * DEGREES; }
	//! 
	template <typename T> T Sqr(T _x) { return _x * _x; }


	//! 
	inline float Sqrt(float _x) { return sqrt(_x); }
	//! 
	inline float RSqrt(float _x) 
	{ 
		TODO("Use asm");
		return 1 / sqrt(_x);
	}
	//! 
	inline float Sin(float _x) { return sin(_x); }
	//! 
	inline float Cos(float _x) { return cos(_x); }
	//! 
	inline void SinCos(float _a, float& _s, float& _c)
	{ 
		TODO("Use asm");
		_s = sin(_a), _c = cos(_a);
	}
	//! 
	inline float Tan(float _x) { return tan(_x); }
	//! 
	inline float ASin(float _x) { return asin(_x); }
	//! 
	inline float ACos(float _x) { return acos(_x); }
	//! 
	inline float ATan2(float _y, float _x) { return atan2(_y, _x); }
	//! 
	inline float Log2(float _x) { return log2(_x); }
	//! 
	inline int Log2i(int _x) { return (int)log2f((float)_x); }

	//! 
	inline uint FirstPow2(uint _val)
	{
		--_val |= _val >> 16;
		_val |= _val >> 8;
		_val |= _val >> 4;
		_val |= _val >> 2;
		_val |= _val >> 1;
		return ++_val;
	}
	//! 
	inline bool IsPow2(uint _val) { return (_val & (_val - 1)) == 0; }
	//! 
	inline uint8 FloatToByte(float _value) { return (uint8)(_value * 0xff); }
	//! 
	inline float ByteToFloat(uint8 _value) { return (float)(_value * (1.0f / 255.0f)); }
	//! 
	inline uint16 FloatToHalf(float _value)
	{
		union { float f; uint i; }_fb = { _value };
#	ifdef _FAST_HALF_FLOAT
		return (uint16)((_fb.i >> 16) & 0x8000) | ((((_fb.i & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((_fb.i >> 13) & 0x03ff);
#	else
		uint _s = (_fb.i >> 16) & 0x00008000; // sign
		int _e = ((_fb.i >> 23) & 0x000000ff) - 0x00000070; // exponent
		uint _r = _fb.i & 0x007fffff; // mantissa
		if (_e < 1)
		{
			if (_e < -10)
				return 0;
			_r = (_r | 0x00800000) >> (14 - _e);
			return (uint16)(_s | _r);
		}
		else if (_e == 0x00000071)
		{
			if (_r == 0)
				return (uint16)(_s | 0x7c00); // Inf
			else
				return (uint16)(((_s | 0x7c00) | (_r >>= 13)) | (_r == 0)); // NAN
		}
		if (_e > 30)
			return (uint16)(_s | 0x7c00); // Overflow
		return (uint16)((_s | (_e << 10)) | (_r >> 13));
#	endif
	}
	//! 
	inline float HalfToFloat(uint16 _value)
	{
		union { uint i; float f; }_fb;
#	ifdef _FAST_HALF_FLOAT
		_fb.i = ((_value & 0x8000) << 16) | (((_value & 0x7c00) + 0x1C000) << 13) | ((_value & 0x03FF) << 13);
#	else
		register int _s = (_value >> 15) & 0x00000001; // sign
		register int _e = (_value >> 10) & 0x0000001f; // exponent
		register int _r = _value & 0x000003ff; // mantissa
		if (_e == 0)
		{
			if (_r == 0) // Plus or minus zero
			{
				_fb.i = _s << 31;
				return _fb.f;
			}
			else // Denormalized number -- renormalize it
			{
				for (; !(_r & 0x00000400); _r <<= 1, _e -= 1);
				_e += 1;
				_r &= ~0x00000400;
			}
		}
		else if (_e == 31)
		{
			if (_r == 0) // Inf
			{
				_fb.i = (_s << 31) | 0x7f800000;
				return _fb.f;
			}
			else // NaN
			{
				_fb.i = ((_s << 31) | 0x7f800000) | (_r << 13);
				return _fb.f;
			}
		}
		_e = (_e + 112) << 23;
		_r = _r << 13;
		_fb.i = ((_s << 31) | _e) | _r;
#	endif
		return _fb.f;
	}
	//! 
	inline float FixedToFloat(uint _value, uint _bits, float _default = 0.0f)
	{
		if (_bits > 31)
			_bits = 31;
		return _bits ? ((float)_value) / ((float)((1u << _bits) - 1u)) : _default;
	}
	//! 
	inline uint FloatToFixed(float _value, uint _bits)
	{
		if (_bits > 31)
			_bits = 31;
		if (_value <= 0)
			return 0;
		if (_value >= 1)
			return (1u << _bits) - 1u;
		return static_cast<uint>(_value * (float)(1u << _bits));
	}
	//! 
	inline uint FixedToFixed(uint _value, uint _from, uint _to)
	{
		if (_from > 31)
			_from = 31;
		if (_to > 31)
			_to = 31;
		if (_from > _to)
			_value >>= _from - _to;
		else if (_from < _to && _value != 0)
		{
			uint _max = (1u << _from) - 1u;
			if (_value == _max)
				_value = (1u << _to) - 1u;
			else if (_max > 0)
				_value *= (1u << _to) / _max;
			else _value = 0;
		}
		return _value;
	}

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	//! IntVector2
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

	//! IntVector3
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

	//! IntVector4
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

	//! Vector2 
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

	//! Vector3
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

	//! Vector4
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

	//! Quaternion
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

	//! Matrix3x4
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

	//! Matrix4x4
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

	//! Ray
	struct Ray
	{
		Vector3 origin, dir;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	//! Plane
	struct Plane
	{
		Vector3 normal;
		float dist;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	//! AlignedBox
	struct AlignedBox
	{
		Vector3 mn, mx;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	//! Frustum
	struct Frustum
	{
		Plane planes[6];
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	//! Dynamic bounding-volumes tree node
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

	//! Dynamic bounding-volumes tree
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
	
	//!\} Math
}

