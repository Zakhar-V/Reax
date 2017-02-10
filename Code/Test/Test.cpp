#include <Base.hpp>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <list>
namespace Reax
{
	template <class T> class WeakRef;

	class RefCounted : public NonCopyable
	{
	public:
		//!
		struct WRef : public NonCopyable
		{
		public:
			//!
			void AddRef(void) { AtomicAdd(m_refCount, 1); }
			//!
			void Release(void)
			{
				if (AtomicSubtract(m_refCount, 1) == 1)
					delete this;
			}
			//!
			RefCounted* _Lock(void)
			{ 
				RefCounted* _object = AtomicGet(m_object, MemoryOrder::Relaxed);
				if (_object)
					_object->AddRef();
				return _object;
			}

		private:
			friend class RefCounted;
			//!
			WRef(RefCounted* _obj) : m_object(_obj) { }
			//!
			void _Reset(void)
			{
				AtomicSet(m_object, nullptr);
				Release();
			}

			int m_refCount = 1;
			RefCounted* m_object;
		};

		virtual ~RefCounted(void)
		{
			ASSERT(m_refCount == 0);
			ASSERT(m_weakRef == nullptr);
		}
		void AddRef(void) { AtomicAdd(m_refCount, 1); }
		void Release(void)
		{
			if (AtomicSubtract(m_refCount, 1) == 1)
			{
				_DeleteThis();
			}
		}
		
		WRef* _GetWeakRef(void)
		{
			if (!m_weakRef)
				m_weakRef = new WRef(this);
			return m_weakRef;
		}
		
	protected:
		virtual void _DeleteThis(void)
		{
			_ReleaseWeakRef();
			delete this;
		}

		void _ReleaseWeakRef(void)
		{
			if (m_weakRef)
			{
				m_weakRef->_Reset();
				m_weakRef = nullptr;
			}
		}

		WRef* m_weakRef = nullptr;
		int m_refCount = 0;
	};

	//!
	template<class T> T* AddRef(T* _ptr)
	{
		if (_ptr)
			_ptr->AddRef();
		return _ptr;
	}

	//!
	template<class T> void Release(T* _ptr)
	{
		if (_ptr)
			_ptr->Release();
	}

	enum _NoAddRef { NoAddRef };

	template <class T> class SharedPtr
	{
	public:
		//!
		SharedPtr(void) = default;
		//!
		SharedPtr(const T* _ptr) : m_ptr(AddRef(const_cast<T*>(_ptr))) { }
		//!
		SharedPtr(const T* _ptr, _NoAddRef) : m_ptr(const_cast<T*>(_ptr)) { }
		//!
		SharedPtr(const SharedPtr& _ptr) : m_ptr(AddRef(const_cast<T*>(_ptr.m_ptr))) { }
		//!
		SharedPtr(SharedPtr&& _ptr) : m_ptr(_ptr.m_ptr) { _ptr.m_ptr = nullptr; }
		//!
		~SharedPtr(void) { Release(m_ptr); }

		//!
		SharedPtr& operator = (const T* _ptr) { AddRef(const_cast<T*>(_ptr)); Release(m_ptr); m_ptr = const_cast<T*>(_ptr); return *this; }
		//!
		SharedPtr& operator = (const SharedPtr& _ptr) { AddRef(const_cast<T*>(_ptr.m_ptr)); Release(m_ptr); m_ptr = const_cast<T*>(_ptr.m_ptr); return *this; }
		//!
		SharedPtr& operator = (SharedPtr&& _ptr) { Release(m_ptr); m_ptr = _ptr.m_ptr; _ptr.m_ptr = nullptr; return *this; }

		//!
		T* operator -> (void) const { ASSERT(m_ptr != nullptr, "nullptr"); return const_cast<T*>(m_ptr); }
		//!
		T& operator * (void) const { ASSERT(m_ptr != nullptr, "nullptr"); return *const_cast<T*>(m_ptr); }
		//!
		operator T* (void) const { return const_cast<T*>(m_ptr); }
		//!
		T* Get(void) const { return const_cast<T*>(m_ptr); }

		//!
		template<class X> X* Cast(void) const { return static_cast<X*>(const_cast<T*>(m_ptr)); }
		//!
		template<class X> X* DynamicCast(void) const { return dynamic_cast<X*>(const_cast<T*>(m_ptr)); }


	private:
		T* m_ptr = nullptr;
	};

	template <class T> class WeakRef
	{
	public:
		WeakRef(const T* _ptr)
		{
			m_obj = const_cast<T*>(_ptr);
			if (m_obj)
			{
				m_ref = m_obj->_GetWeakRef();
			}
		}

		SharedPtr<T> Lock(void)
		{
			if(m_ref)
		}

	private:
		void _AddRef(T* _obj)
		{
			m_obj = _obj;
			m_ref = _obj->
		}
		void _Release(void)
		{
			if (m_ref)
			{
				m_ref = nullptr;
				m_obj = nullptr;
			}
		}
		T* m_obj = nullptr;
		int* m_ref = nullptr;
	};


	//template <class T> struct _StdIteratorTag;
//	template <class T> struct _StdIteratorTag<std::random_access_iterator_tag> { static const ArrayIteratorType Type = ArrayIteratorType::ID; };


	/*template <class T> _StdListIterator(std::list<T>::const_iterator i)
	{
		return i;
	}

	template <class T> struct _IteratorInfo;
	template <class T> struct _IteratorInfo<decltype(_StdListIterator(T()))>
	{ 
		//static const decltype(_StdIteratorTag<std::iterator_traits>::iterator_category) Type = ArrayIteratorType::ID; 
	};*/

	template<class T> class I
	{
	public:

	};

	template<class T> class A
	{
	public:
		typedef I<T> Iter;
		typedef I<const T> ConstIter;
	};

	template <class T, class = void> struct Info;
	template <class T> struct Info<T, std::void_t<typename A<T>::Iter>> { };
}

using namespace Reax;

#include <string>
#include <unordered_map>


//template <class T> struct IteratorTag<T, VoidType<typename std::vector<T>::const_iterator>> { static const ArrayIteratorTag Type = ArrayIteratorTag::ID; };
//template <class T> struct IteratorTag<T, typename std::basic_string<T>::iterator> { static const ArrayIteratorTag Type = ArrayIteratorTag::ID; };
//template <class T> struct IteratorTag<T, typename std::basic_string<T>::const_iterator> { static const ArrayIteratorTag Type = ArrayIteratorTag::ID; };

/*
TODO:
*/

//template <class C, class T> struct IteratorInfo { static const GenericIteratorType Type = GenericIteratorType::ID; };
//template <class T> struct StlIteratorType;
//template <class T> struct StlIteratorType<typename std::vector<T>::iterator> { typedef typename std::vector<T>::iterator Type; };

//template <class T> struct IteratorInfo<typename std::vector<T>::const_iterator> { static const ArrayIteratorType Type = ArrayIteratorType::ID; };

#include <Windows.h>

struct Timer
{
	static double Now(void)
	{
		LARGE_INTEGER _c, _f;
		QueryPerformanceCounter(&_c);
		QueryPerformanceFrequency(&_f);
		return (double)_c.QuadPart / (double)_f.QuadPart;

	}
	void Start(void)
	{
		start = Now();
	}
	void Stop(void)
	{
		time = Now() - start;
	}

	double start;
	double time;
};

void StdContainerBenchmark(uint _num = 10000000)
{
	Timer _timer;
	int _sum = 0;
	printf("%d objects\n", _num);

	std::vector<int> _stdVec;
	std::list<int> _stdList;
	std::unordered_map<int, int> _stdMap;


	// array append
	{
		_timer.Start();
		for (uint i = 0; i < _num; ++i)
			_stdVec.push_back(i);
		_timer.Stop();
		printf("std::vector::push_back: time: %f seconds, capacity: %zd\n", _timer.time, _stdVec.capacity());
	}

	// list append
	{
		_timer.Start();
		for (uint i = 0; i < _num; ++i)
			_stdList.push_back(i);
		_timer.Stop();
		printf("std::list::push_back: time: %f seconds\n", _timer.time);
	}

	// map insert
	{
		_timer.Start();
		for (uint i = 0; i < _num; ++i)
			_stdMap[i] = i;
		_timer.Stop();
		printf("std::unordered_map::operator[]: time: %f seconds, buckets: %zd\n", _timer.time, _stdMap.bucket_count());

	}

	// array lookup
	{
		_sum = 0;
		_timer.Start();
		for (const auto& i : _stdVec)
			_sum += i;
		_timer.Stop();
		printf("std::vector lookup: time: %f seconds, sum: %d\n", _timer.time, _sum);
	}


	// list lookup
	{
		_sum = 0;
		_timer.Start();
		for (const auto& i : _stdList)
			_sum += i;
		_timer.Stop();
		printf("std::list lookup: time: %f seconds, sum: %d\n", _timer.time, _sum);
	}

	// map lookup
	{
		_sum = 0;
		_timer.Start();
		for (const auto& i : _stdMap)
			_sum += i.first;
		_timer.Stop();
		printf("std::unordered_map lookup: time: %f seconds, sum: %d\n", _timer.time, _sum);
	}

	// array pop
	{
		_timer.Start();
		for (uint i = 0; i < _num; ++i)
			_stdVec.pop_back();
		_timer.Stop();
		printf("std::vector::pop_back: time: %f seconds, capacity: %zd\n", _timer.time, _stdVec.capacity());
	}

	// list pop
	{
		_timer.Start();
		for (uint i = 0; i < _num; ++i)
			_stdList.pop_back();
		_timer.Stop();
		printf("std::list::pop_back: time: %f seconds\n", _timer.time);
	}

	// map erase
	{
		_timer.Start();
		for (uint i = 0; i < _num; ++i)
			_stdMap.erase(i);
		_timer.Stop();
		printf("std::unordered_map::erase: time: %f seconds, buckets: %zd\n", _timer.time, _stdMap.bucket_count());
	}
}
void MyContainerBenchmark()
{
	Timer _timer;
	uint _num = 10000000;
	int _sum = 0;
	printf("%d objects\n", _num);

	Array<int> _myVec;
	List<int> _myList;
	HashMap<int, int> _myMap;


	// array append
	{

		_timer.Start();
		for (uint i = 0; i < _num; ++i)
			_myVec.Push(i);
		_timer.Stop();
		printf("Reax::Array::Push: time: %f seconds, capacity: %d\n", _timer.time, _myVec.Capacity());
	}

	// list append
	{
		_timer.Start();
		for (uint i = 0; i < _num; ++i)
			_myList.Push(i);
		_timer.Stop();
		printf("Reax::List::Push: time: %f seconds\n", _timer.time);
	}

	// map insert
	{
		_timer.Start();
		for (uint i = 0; i < _num; ++i)
			_myMap[i] = i;
		_timer.Stop();
		printf("Reax::HashMap::operator[]: time: %f seconds, buckets: %d\n", _timer.time, _myMap.BucketCount());
	}

	// array lookup
	{
		_sum = 0;
		_timer.Start();
		for (const auto& i : _myVec)
			_sum += i;
		_timer.Stop();
		printf("Reax::Array lookup: time: %f seconds, sum: %d\n", _timer.time, _sum);
	}


	// list lookup
	{
		_sum = 0;
		_timer.Start();
		for (const auto& i : _myList)
			_sum += i;
		_timer.Stop();
		printf("Reax::List lookup: time: %f seconds, sum: %d\n", _timer.time, _sum);
	}

	// map lookup
	{
		_sum = 0;
		_timer.Start();
		for (const auto& i : _myMap)
			_sum += i.first;
		_timer.Stop();
		printf("Reax::HashMap lookup: time: %f seconds, sum: %d\n", _timer.time, _sum);
	}

	// array pop
	{
		_timer.Start();
		for (uint i = 0; i < _num; ++i)
			_myVec.Pop();
		_timer.Stop();
		printf("Reax::Array::Pop: time: %f seconds, capacity: %d\n", _timer.time, _myVec.Capacity());
	}

	// list pop
	{
		_timer.Start();
		for (uint i = 0; i < _num; ++i)
			_myList.Pop();
		_timer.Stop();
		printf("Reax::List::Pop: time: %f seconds\n", _timer.time);
	}

	// map erase
	{
		_timer.Start();
		for (uint i = 0; i < _num; ++i)
			_myMap.Erase(i);
		_timer.Stop();
		printf("Reax::HashMap::Erase: time: %f seconds, buckets: %d\n", _timer.time, _myMap.BucketCount());
	}
}
// - ƒобавить новые контейнеры в Base.natvis
// - заменить m_caps на m_capasity

void main()
{
	std::vector<int> sv = { 1, 2, 3 };
	auto svb = sv.begin();
	auto sve = sv.end();

	Array<int> rv = { 1, 2, 3 };
	auto rvb = rv.Begin();
	auto rve = rv.End();

	std::list<int> sl = { 1, 2, 3 };
	auto slb = sl.begin();
	auto sle = sl.end();

	List<int> rl = { 1, 2, 3 };
	auto rlb = rl.Begin();
	auto rle = rl.End();

	std::unordered_map<int, int> sm = { { 1, 1 },{ 2, 2 },{ 3, 3 } };
	auto smb = sm.begin();
	auto sme = sm.end();

	HashMap<int, int> rm = { { 1, 1 },{ 2, 2 },{ 3, 3 } };
	auto rmb = rm.Begin();
	auto rme = rm.End();

	std::string ss = "abc";
	auto ssb = sm.begin();
	auto sse = sm.end();

	String rs = "abc";
	auto rsb = rs.Begin();
	auto rse = rs.End();

	//StdContainerBenchmark();
	//printf("\n");
	//MyContainerBenchmark();
	//printf("\n");
	//StdContainerBenchmark();

	system("pause");
}

