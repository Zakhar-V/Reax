#include <Base.hpp>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
namespace Reax
{


}

using namespace Reax;

/*
TODO:
обдумать поведение строк при вставке нулевых символов
1. контроллировать длинну
2. использовать как есть  +++++++
	- нет проверок при вставке
	- длина m_buffer->length не обязанна совпадать с реальной длиной строки (но всегда больше ее)
	- всегда присутствует завершающий нулевой символ
*/

void main()
{
	std::vector<int> v;
	std::string _str;
	_str.resize(10);
	_str.push_back('\0');
	_str.push_back('x');
	printf("%zd\n", _str.length());
	printf("'%s'\n", _str.c_str());

	_str[0] = 'a';
	_str[1] = 'b';
	_str[2] = 'c';

	printf("%zd\n", _str.length());
	printf("'%s'\n", _str.c_str());
	std::string _str2 = _str;
	printf("'%s' '%c'\n", _str2.c_str(), _str2[11]);

	String _str3;
	_str3.Resize(10);
	_str3.Append('\0');
	_str3.Append('x');
	_str3(0) = 'a';
	_str3(1) = 'b';
	_str3(2) = 'c';
	printf("%zd\n", _str3.Length());
	printf("'%s' '%c'\n", _str3.CStr(), _str3[11]);

	char ch = _str3[1];
	printf("'%c'\n", ch);

	printf("%zd %zd %zd\n", _str.capacity(), _str2.capacity(), _str3.Capacity());

	system("pause");
}

