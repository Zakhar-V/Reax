#pragma once

#include "Common.hpp"
#include <initializer_list>
#include <iterator>

namespace Reax
{
	//!\addtogroup Container 
	//!\{

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
	// InitializerList
	//----------------------------------------------------------------------------//

	//!
	template <class T> using InitializerList = std::initializer_list<T>;

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
		Iterator Insert(KeyValue&& _value)
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

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	//!\} Container
}
