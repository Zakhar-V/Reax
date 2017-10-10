#pragma once

#include "Concurrency.hpp"

namespace Reax
{
	//!\addtogroup Base 
	//!\{

	//----------------------------------------------------------------------------//
	// RefCounted
	//----------------------------------------------------------------------------//

	//!
	class RefCounted : public NonCopyable
	{
	public:

		//!
		class WeakReference final : public NonCopyable
		{
		public:
			friend class RefCounted;

			//!
			void AddRef(void)
			{
				AtomicAdd(m_refCount, 1);
			}
			//!
			void Release(void)
			{
				if (AtomicSubtract(m_refCount, 1) == 1)
					delete this;
			}

			RefCounted* GetPtr(void) { return AtomicGet(m_ptr); }

		private:
			//!
			WeakReference(RefCounted* _ptr) : m_ptr(_ptr) { }
			//!
			~WeakReference(void)
			{
				ASSERT(m_ptr == nullptr && m_refCount == 0, "Incorrect deletion");
			}

			//!
			void _Reset(void)
			{
				AtomicSet(m_ptr, nullptr);
				Release();
			}

			RefCounted* m_ptr;
			int m_refCount = 1;
		};

		//!
		RefCounted(void) { }
		//!
		~RefCounted(void)
		{
			ASSERT(m_weakRef == nullptr && m_refCount == 0, "Incorrect deletion");
		}

		//!
		void AddRef(void)
		{
			AtomicAdd(m_refCount, 1);
		}
		//! Safe increment a counter of references. Uses for weak references. 
		bool SafeAddRef(void)
		{
			for (;;)
			{
				int _rc = AtomicGet(m_refCount, MemoryOrder::Relaxed);

				if (!_rc)
					break;

				if (AtomicCompareExchange(m_refCount, _rc, _rc + 1))
					return true;
			}
			return false;
		}
		//!
		void Release(void)
		{
			if (AtomicSubtract(m_refCount, 1) == 1)
				_DeleteThis();
		}

		//!
		WeakReference* GetWeakRef(void)
		{
			for (WeakReference* c = nullptr; (c = AtomicGet(m_weakRef, MemoryOrder::Relaxed)) == nullptr || c == (WeakReference*)-1;)
			{
				if (AtomicCompareExchange<WeakReference>(m_weakRef, (WeakReference*)0, (WeakReference*)-1))
				{
					AtomicSet(m_weakRef, new WeakReference(this));
					break;
				}
			}

			return m_weakRef;
		}

	protected:
		//!
		void _ResetRef(void)
		{
			if (m_weakRef)
			{
				m_weakRef->_Reset();
				m_weakRef = nullptr;
			}
		}
		//!
		virtual void _DeleteThis(void)
		{
			_ResetRef();
		}

	private:
		//!
		WeakReference* m_weakRef = nullptr;
		int m_refCount = 0;
	};

	//----------------------------------------------------------------------------//
	// SharedPtr
	//----------------------------------------------------------------------------//

	//!
	enum NoAddRef_t { NoAddRef };

	//!
	template <class T> class SharedPtr
	{
	public:
		//!
		SharedPtr(void) = default;
		//!
		~SharedPtr(void)
		{
			if (m_ptr)
				m_ptr->Release();
		}
		//!
		SharedPtr(const SharedPtr& _ptr) : SharedPtr(_ptr.m_ptr) {}
		//!
		SharedPtr(SharedPtr&& _ptr) : m_ptr(_ptr.m_ptr)
		{
			_ptr.m_ptr = nullptr;
		}
		//!
		SharedPtr(const T* _ptr) : m_ptr(const_cast<T*>(_ptr))
		{
			if (m_ptr)
				m_ptr->AddRef();
		}
		//!
		SharedPtr(const T* _ptr, NoAddRef_t) : m_ptr(const_cast<T*>(_ptr)) { }

		//!
		SharedPtr& operator = (const T* _ptr)
		{
			if (_ptr)
				_ptr->AddRef();
			if (m_ptr)
				m_ptr->Release();
			m_ptr = const_cast<T*>(_ptr);
			return *this;
		}
		//!
		SharedPtr& operator = (const SharedPtr& _ptr)
		{
			return *this = _p.m_ptr;
		}
		//!
		SharedPtr& operator = (SharedPtr&& _ptr)
		{
			Swap(_ptr.m_ptr, m_ptr);
			return *this;
		}

		//!
		operator T* (void) const { return const_cast<T*>(m_ptr); }
		//!
		T* operator -> (void) const { return const_cast<T*>(m_ptr); }
		//!
		T& operator * (void) const { return *const_cast<T*>(p); }

		//!
		T* Get(void) const { return const_cast<T*>(m_ptr); }
		//!
		template <class X> X* Cast(void) const { return static_cast<X*>(const_cast<T*>(m_ptr)); }

	protected:
		T* m_ptr = nullptr;
	};

	//----------------------------------------------------------------------------//
	// WeakRef
	//----------------------------------------------------------------------------//

	//!
	template <class T> class WeakRef
	{
	public:
		//!
		WeakRef(void) = default;
		//!
		~WeakRef(void) = default;
		//!
		WeakRef(const WeakRef& _ref) : m_ref(_ref.m_ref) { }
		//!
		WeakRef(WeakRef&& _ref) : m_ref(Move(_ref.m_ref)) { }
		//!
		WeakRef(const SharedPtr<T>& _ptr) : WeakRef(_ptr.Get()) { }
		//!
		WeakRef(const T* _ptr) : m_ref(_ptr ? const_cast<T*>(_ptr)->GetweakRef() : nullptr) { }

		//!
		WeakRef& operator = (const WeakRef& _ref)
		{
			m_ref = _ref.m_ref;
			return *this;
		}
		//!
		WeakRef& operator = (WeakRef&& _ref)
		{
			Swap(m_ref, _ref.m_ref);
			return *this;
		}
		//!
		WeakRef& operator = (const SharedPtr<T>& _ptr) { return *this = _ptr.Get(); }
		//!
		WeakRef& operator = (const T* _ptr)
		{
			m_ref = _ptr ? const_cast<T*>(_ptr)->GetweakRef() : nullptr;
			return *this;
		}

		//!
		operator SharedPtr<T>(void) const
		{
			return Lock();
		}

		//!
		T* Get(void) const
		{
			return m_ref ? m_ref->GetPtr() : nullptr;
		}

		//!
		SharedPtr<T> Lock(void) const
		{
			T* _ptr = Get();
			if (_ptr && _ptr->SafeAddRef())
				return SharedPtr<T>(_ptr, NoAddRef);

			return nullptr;
		}

	protected:
		SharedPtr<RefCounted::WeakReference> m_ref;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
	
	//!\} Base
}
