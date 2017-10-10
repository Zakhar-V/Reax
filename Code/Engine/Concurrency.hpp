#pragma once

#include "Common.hpp"
#include <atomic>

namespace Reax
{
	//!\addtogroup Concurrency
	//!\{

	//----------------------------------------------------------------------------//
	// Atomic
	//----------------------------------------------------------------------------//

	//!
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
		//!
		Atomic(T _value = static_cast<T>(0)) : m_value(_value) { }
		//!
		Atomic(const Atomic& _value) : m_value(_value.Get()) { }
		//!
		Atomic& operator = (const Atomic& _value) { return Set(_value.Get()); }
		//!
		Atomic& operator = (T _value) { return Set(_value); }
		//!
		operator T(void) const { return Get(); }

		//!
		T operator += (T _value) { return Add(_value) + _value; }
		//!
		T operator -= (T _value) { return Subtract(_value) - _value; }
		//!
		T operator &= (T _value) { return And(_value) & _value; }
		//!
		T operator |= (T _value) { return Or(_value) | _value; }
		//!
		T operator ^= (T _value) { return Xor(_value) ^ _value; }

		//!
		T operator ++ (void) { return Add(static_cast<T>(1)) + static_cast<T>(1); }
		//!
		T operator ++ (int) { return Add(static_cast<T>(1)); }
		//!
		T operator -- (void) { return Subtract(static_cast<T>(1)) - static_cast<T>(1); }
		//!
		T operator -- (int) { return Subtract(static_cast<T>(1)); }

		//!
		T Get(void) const { return AtomicGet(m_value); }
		//!
		T GetRaw(void) const { return m_value; }
		//!
		Atomic& Set(T _value, MemoryOrder _order = MemoryOrder::Sequential) { AtomicSet(m_value, _value); return *this; }
		//!
		T Add(T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicAdd(m_value, _value); }
		//!
		T Subtract(T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicSubtract(m_value, _value); }
		//!
		T And(T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicAnd(m_value, _value); }
		//!
		T Or(T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicOr(m_value, _value); }
		//!
		T Xor(T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicXor(m_value, _value); }
		//!
		T Exchange(T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicExchange(m_value, _value, _order); }
		//!
		bool CompareExchange(T _expected, T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchange(m_value, _expected, _value, _order); }
		//!
		bool CompareExchange(T* _expected, T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchange(m_value, _expected, _value, _order); }
		//!
		bool CompareExchangeWeak(T _expected, T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchangeWeak(m_value, _expected, _value, _order); }
		//!
		bool CompareExchangeWeak(T* _expected, T _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchangeWeak(m_value, _expected, _value, _order); }

	protected:
		mutable T m_value;
	};

	//! Atomic variable of boolean
	template <> class Atomic<bool>
	{
	public:
		//!
		Atomic(bool _value = false) : m_value(_value) { }
		//!
		Atomic(const Atomic& _value) : m_value(_value.Get()) { }
		//!
		Atomic& operator = (const Atomic& _value) { return Set(_value.Get()); }
		//!
		Atomic& operator = (bool _value) { return Set(_value); }
		//!
		operator bool(void) const { return Get(); }

		//!
		bool Get(void) const { return AtomicGet(m_value); }
		//!
		bool GetRaw(void) const { return m_value; }
		//!
		Atomic& Set(bool _value, MemoryOrder _order = MemoryOrder::Sequential) { AtomicSet(m_value, _value); return *this; }
		//!
		bool Exchange(bool _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicExchange(m_value, _value, _order); }
		//!
		bool CompareExchange(bool _expected, bool _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchange(m_value, _expected, _value, _order); }
		//!
		bool CompareExchange(bool* _expected, bool _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchange(m_value, _expected, _value, _order); }
		//!
		bool CompareExchangeWeak(bool _expected, bool _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchangeWeak(m_value, _expected, _value, _order); }
		//!
		bool CompareExchangeWeak(bool* _expected, bool _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchangeWeak(m_value, _expected, _value, _order); }

	protected:
		mutable bool m_value;
	};

	//! Atomic variable of pointer
	template <class T> class Atomic<T*>
	{
	public:
		//!
		Atomic(T* _value = nullptr) : m_value(_value) { }
		//!
		Atomic(const Atomic& _value) : m_value(_value.Get()) { }
		//!
		Atomic& operator = (const Atomic& _value) { return Set(_value.Get()); }
		//!
		Atomic& operator = (T* _value) { return Set(_value); }
		//!
		operator T(void) const { return Get(); }

		//!
		T operator += (ptrdiff_t _value) { return Add(_value) + _value; }
		//!
		T operator -= (ptrdiff_t _value) { return Subtract(_value) - _value; }

		//!
		T* operator ++ (void) { return Add(1) + 1; }
		//!
		T* operator ++ (int) { return Add(1); }
		//!
		T* operator -- (void) { return Subtract(1) - 1; }
		//!
		T* operator -- (int) { return Subtract(1); }

		//!
		T* Get(void) const { return AtomicGet(m_value); }
		//!
		T* GetRaw(void) const { return m_value; }
		//!
		Atomic& Set(T* _value, MemoryOrder _order = MemoryOrder::Sequential) { AtomicSet(m_value, _value); return *this; }
		//!
		T* Add(ptrdiff_t _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicAdd(m_value, _value); }
		//!
		T* Subtract(ptrdiff_t _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicSubtract(m_value, _value); }
		//!
		T* Exchange(T* _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicExchange(m_value, _value, _order); }
		//!
		bool CompareExchange(T* _expected, T* _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchange(m_value, _expected, _value, _order); }
		//!
		bool CompareExchange(T** _expected, T* _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchange(m_value, _expected, _value, _order); }
		//!
		bool CompareExchangeWeak(T* _expected, T* _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchangeWeak(m_value, _expected, _value, _order); }
		//!
		bool CompareExchangeWeak(T** _expected, T* _value, MemoryOrder _order = MemoryOrder::Sequential) { return AtomicCompareExchangeWeak(m_value, _expected, _value, _order); }

	protected:
		mutable T* m_value;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class SpinLock
	{
	public:

	protected:
		Atomic<int> m_lock;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class Mutex
	{
	public:

	protected:
		void* m_handle;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class ReadWriteMutex
	{
	public:

	protected:
		void* m_handle;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class Semaphore
	{
	public:

	protected:
		void* m_handle;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class ThreadSignal
	{
	public:

	protected:
		void* m_handle;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class ConditionalVariable
	{
	public:

	protected:
		void* m_handle;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	class Thread
	{
	public:

	protected:
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//

	//!\} Concurrency
}