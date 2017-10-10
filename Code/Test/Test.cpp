#include <Container.hpp>
#include <Concurrency.hpp>
#include <String.hpp>
#include <Object.hpp>
#include <stdio.h>
#include <stdlib.h>

#include <memory>
#include <vector>
#include <list>
namespace Reax
{

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

	StdContainerBenchmark();
	printf("\n");
	MyContainerBenchmark();
	printf("\n");
	StdContainerBenchmark();

	std::shared_ptr<int> ssp = std::make_shared<int>(0);
	std::weak_ptr<int> swp = ssp;
	std::shared_ptr<int> ssp2 = swp.lock();
	//ssp.use_count();
	system("pause");
}

