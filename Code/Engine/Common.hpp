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

	//!
	RX_API void Assert(const char* _type, const char* _func, const char* _file, int _line, const char* _desc, const char* _msg = nullptr, ...);

	//----------------------------------------------------------------------------//
	// NonCopyable
	//----------------------------------------------------------------------------//
	
	//!\addtogroup Base 
	//!\{

	//!
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
	// Singleton
	//----------------------------------------------------------------------------//

	//!
	template <class T> class Singleton
	{
	public:
		//!
		Singleton(void)
		{
			ASSERT(s_instance == nullptr, "Instance already exists");
			s_instance = static_cast<T*>(this);
		}
		//!
		~Singleton(void)
		{
			s_instance = nullptr;
		}

		//!
		static T* Get(void) { return s_instance; }
		
		static T* const& Instance;

	protected:
		static T* s_instance;
	};

	template <class T> T* Singleton<T>::s_instance = nullptr;
	template <class T> T* const& Singleton<T>::Instance = Singleton<T>::s_instance;

	//!\} Base

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
	
	//!\} Container

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}