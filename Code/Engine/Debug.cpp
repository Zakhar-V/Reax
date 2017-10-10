#include "Debug.hpp"

namespace Reax
{
	//----------------------------------------------------------------------------//
	// Assert
	//----------------------------------------------------------------------------//

	void Assert(const char* _type, const char* _func, const char* _file, int _line, const char* _desc, const char* _msg, ...)
	{
		NOT_IMPLEMENTED_YET();

		printf("%s(%d)<%s>: %s(%s)", _file, _line, _func, _type, _desc);
		if (_msg)
		{
			printf(" : ");
			va_list _args;
			va_start(_args, _msg);
			vprintf(_msg, _args);
			va_end(_args);
		}
		printf("\n");
	}

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
