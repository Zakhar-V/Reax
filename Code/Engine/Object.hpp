#pragma once

#include "RefCounting.hpp"
#include "Container.hpp"

namespace Reax
{
	typedef SharedPtr<class Object> ObjectPtr;

	struct Attribute
	{
		enum Flags
		{

		};

		enum class Type
		{
			Null,
		};
	};

	struct TypeInfo
	{

		HashMap<uint, Attribute> attributes;
	};

	class RX_API Object : public RefCounted
	{
	public:

	protected:
	};

#define gReflection ReflectionSystem::Instance

	class RX_API ReflectionSystem : public Singleton<ReflectionSystem>
	{

	};
}
