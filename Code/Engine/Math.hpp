#pragma once

#include "Common.hpp"
#include <math.h>

namespace Reax
{
	//!\addtogroup Math
	//!\{

	//----------------------------------------------------------------------------//
	// Math
	//----------------------------------------------------------------------------//

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
			DbvNode* child[2] = { 0, 0 };
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
