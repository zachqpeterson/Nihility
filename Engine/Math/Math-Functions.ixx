module;

#include "Containers\ContainerDefines.hpp"
#include "Containers\String.hpp"

#include <math.h>
#include <cstringt.h>

export module Math:Functions;

export import :Constants;
import Core;

constexpr U64 secret0 = 0xa0761d6478bd642full;
constexpr U64 secret1 = 0xe7037ed1a0b428dbull;
constexpr U64 secret2 = 0x8ebc6af09c88c6e3ull;
constexpr U64 secret3 = 0x589965cc75374cc3ull;

export class NH_API Math
{
public:
	template <typename Type> static constexpr Type Min(const Type& a) noexcept { return a; }
	template <typename Type> static constexpr Type Min(const Type& a, const Type& b) noexcept { return a > b ? b : a; }
	template <typename Type, typename... Args> static constexpr Type Min(const Type& a, const Type& b, Args&&... args) noexcept { return a < b ? Min(a, args...) : Min(b, args...); }

	template <typename Type> static constexpr Type Max(const Type& a) noexcept { return a; }
	template <typename Type> static constexpr Type Max(const Type& a, const Type& b) noexcept { return a < b ? b : a; }
	template <typename Type, typename... Args> static constexpr Type Max(const Type& a, const Type& b, Args&&... args) noexcept { return a > b ? Max(a, args...) : Max(b, args...); }

	template <typename Type> static constexpr Type Abs(const Type& n) noexcept { return n < (Type)0 ? -n : n; }
	template <typename Type> static constexpr Type Clamp(const Type& n, const Type& min, const Type& max) noexcept { return n < min ? min : n > max ? max : n; }
	template <typename Type> static constexpr Type Sign(const Type& n) noexcept { return (Type)((n > (Type)0) - (n < (Type)0)); }
	template <typename Type> static constexpr Type NonZeroSign(const Type& n) noexcept { return (Type)(2 * (n > (Type)0) - (Type)1); }
	template <FloatingPoint Type> static constexpr I64 Floor(const Type& n) noexcept { return n >= (Type)0 ? (I64)n : (I64)n - 1; }
	template <FloatingPoint Type> static constexpr Type FloorF(const Type& n) noexcept { return n > Traits<Type>::MaxPrecision ? n : n >= (Type)0 ? (Type)(I64)n : (Type)(I64)n - (Type)1; }
	template <FloatingPoint Type> static constexpr I64 Ceiling(const Type& n) noexcept { return (n - (I64)n) < (Type)0 ? (I64)n : (I64)n + 1; }
	template <FloatingPoint Type> static constexpr Type CeilingF(const Type& n) noexcept { return n > Traits<Type>::MaxPrecision ? n : (n - (I64)n) < (Type)0 ? (Type)(I64)n : (Type)(I64)n + (Type)1; }
	template <FloatingPoint Type> static constexpr Type Round(const Type& n) noexcept { return (Type)(I64)(n + 0.5); }
	template <FloatingPoint Type> static constexpr Type RoundI(const Type& n) noexcept { return (I64)(n + 0.5); }
	template <FloatingPoint Type> static constexpr Type Mod(const Type& n, const Type& d) noexcept { return n - d * FloorF(n / d); }
	template <FloatingPoint Type> static constexpr Type Closest(Type n, Type a, Type b) noexcept { return n < (b + a) * 0.5f ? a : b; }
	template <Integer Type> static constexpr Type Closest(Type n, Type a, Type b) noexcept { return n < (b + a) >> 1 ? a : b; }

	template <FloatingPoint Type> static constexpr Type DegToRad(Type deg) noexcept { return deg * DEG_TO_RAD_T<Type>; }
	template <FloatingPoint Type> static constexpr Type RadToDeg(Type rad) noexcept { return rad * RAD_TO_DEG_T<Type>; }
	template <FloatingPoint Type> static constexpr Type NormalizeAngle(Type f) noexcept { return (Type)(f - (TWO_PI * FloorF((f + PI) / TWO_PI))); }

	template <FloatingPoint Type> static constexpr bool IsZero(Type f) noexcept { return f < Traits<Type>::Epsilon&& f > -Traits<Type>::Epsilon; }
	template <FloatingPoint Type> static constexpr bool IsNaN(Type f) noexcept { return f != f; }
	template <FloatingPoint Type> static constexpr bool IsInf(Type f) noexcept { return f == Traits<Type>::Infinity; }
	template <FloatingPoint Type> static constexpr bool IsNegInf(Type f) noexcept { return f == -Traits<Type>::Infinity; }

private:
	template<FloatingPoint Type> static constexpr I64 FindWhole(const Type f) noexcept
	{
		if (Abs(f - FloorF(f)) >= (Type)0.5) { return (I64)(FloorF(f) + Sign(f)); }
		else { return (I64)FloorF(f); }
	}

	template<FloatingPoint Type> static constexpr I64 FindFraction(const Type f) noexcept
	{
		if (Abs(f - FloorF(f)) >= (Type)0.5) { return (I64)(f - FloorF(f) - Sign(f)); }
		else { return (I64)(f - FloorF(f)); }
	}

	template<FloatingPoint Type> static constexpr I64 FindExponent(const Type f, const I64 exponent) noexcept
	{
		if (f < 1e-03) { return FindExponent(f * (Type)1e+04, exponent - 4); }
		else if (f < 1e-01) { return FindExponent(f * (Type)1e+02, exponent - 2); }
		else if (f < 1.0) { return FindExponent(f * (Type)10.0, exponent - 1); }
		else if (f > 10.0) { return FindExponent(f / (Type)10.0, exponent + 1); }
		else if (f > 1e+02) { return FindExponent(f / (Type)1e+02, exponent + 2); }
		else if (f > 1e+04) { return FindExponent(f / (Type)1e+04, exponent + 4); }
		else { return exponent; }
	}

	template<FloatingPoint Type> static constexpr Type FindMantissa(const Type f) noexcept
	{
		if (f < 1.0) { return FindMantissa(f * (Type)10.0); }
		else if (f > 10.0) { return FindMantissa(f / (Type)10.0); }
		else { return f; }
	}

	template<FloatingPoint Type> static constexpr Type LogMantissa(const I32 n) noexcept
	{
		switch (n)
		{
		case 2: return (Type)0.6931471805599453;
		case 3: return (Type)1.0986122886681097;
		case 4: return (Type)1.3862943611198906;
		case 5: return (Type)1.6094379124341004;
		case 6: return (Type)1.7917594692280550;
		case 7: return (Type)1.9459101490553133;
		case 8: return (Type)2.0794415416798359;
		case 9: return (Type)2.1972245773362194;
		case 10: return (Type)2.3025850929940457;
		default: return (Type)0.0;
		}
	}

	template<FloatingPoint Type> static constexpr Type LogMain(const Type f) noexcept
	{
		Type f1 = (f - (Type)1.0) / (f + (Type)1.0);

		Type f2 = f1 * f1;


		I32 depth = 24;
		Type res = (Type)(2 * (depth + 1) - 1);

		while (depth > 0)
		{
			res = (Type)(2 * depth - 1) - (Type)(depth * depth) * f2 / res;

			--depth;
		}

		return (Type)2.0 * f1 / res;
	}

	template<FloatingPoint Type> static constexpr Type ExpRecur(const Type x, const I32 depth) noexcept
	{
		if (depth < 25) { return (Type)1.0 + x / (Type)(depth - 1) - x / depth / ExpRecur(x, depth + 1); }
		else { return (Type)1.0; }
	}

	template <Number Base, Signed Exp> static constexpr Base PowBySquaring(const Base b, const Base val, const Exp e) noexcept
	{
		if (b < 0) { return PowBySquaring(b, (Base)1 / b, -e); }
		else if (e > 1)
		{
			if (e & 1) { return PowBySquaring(b * b, val * b, e / 2); }
			else { return PowBySquaring(b * b, val, e / 2); }
		}
		else if (e == 1) { return val * b; }
		else { return val; }
	}

public:
	template <FloatingPoint Type> static constexpr Type Sin(Type f) noexcept
	{
		if (ConstantEvaluation())
		{
			f = NormalizeAngle(f);

			const Type f2 = f * f;
			Type pow = f;

			Type result = pow;
			pow *= f2;
			result += (Type)-0.16666667 * pow;
			pow *= f2;
			result += (Type)0.0083333333 * pow;
			pow *= f2;
			result += (Type)-0.0001984127 * pow;
			pow *= f2;
			result += (Type)2.7557319e-6 * pow;
			pow *= f2;
			result += (Type)-2.5052108e-8 * pow;
			pow *= f2;
			result += (Type)1.6059044e-10 * pow;
			pow *= f2;
			result += (Type)-7.6471637e-13 * pow;
			pow *= f2;
			result += (Type)2.8114573e-15 * pow;
			pow *= f2;
			result += (Type)-8.2206352e-18 * pow;
			pow *= f2;

			return result;
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return sinf(f); }
			else { return sin(f); }
		}
	}
	template <FloatingPoint Type> static constexpr Type Cos(Type f) noexcept
	{
		if (ConstantEvaluation())
		{
			f = NormalizeAngle(f);

			const Type f2 = f * f;
			Type pow = 1;

			Type result = 1;
			pow *= f2;
			result += (Type)-0.5 * pow;
			pow *= f2;
			result += (Type)0.0416666667 * pow;
			pow *= f2;
			result += (Type)-0.0013888889 * pow;
			pow *= f2;
			result += (Type)2.48015873e-5 * pow;
			pow *= f2;
			result += (Type)-2.75573192e-7 * pow;
			pow *= f2;
			result += (Type)2.0876757e-9 * pow;
			pow *= f2;
			result += (Type)-1.14707456e-11 * pow;
			pow *= f2;
			result += (Type)4.77947733e-14 * pow;
			pow *= f2;
			result += (Type)-1.5619207e-16 * pow;
			pow *= f2;

			return result;
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return cosf(f); }
			else { return cos(f); }
		}
	}
	template <FloatingPoint Type> static constexpr Type Tan(Type f) noexcept
	{
		if (ConstantEvaluation())
		{
			return Sin(f) / Cos(f);
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return tanf(f); }
			else { return tan(f); }
		}
	}

	template <FloatingPoint Type> static Type SinH(Type f) noexcept { if constexpr (IsSame<Type, F32>) { return sinfh(f); } else { return sinh(f); } }
	template <FloatingPoint Type> static Type CosH(Type f) noexcept { if constexpr (IsSame<Type, F32>) { return cosfh(f); } else { return cosh(f); } }
	template <FloatingPoint Type> static Type TanH(Type f) noexcept { if constexpr (IsSame<Type, F32>) { return tanfh(f); } else { return tanh(f); } }

	template <FloatingPoint Type> static Type Asin(Type f) noexcept { if constexpr (IsSame<Type, F32>) { return asinf(f); } else { return asin(f); } }
	template <FloatingPoint Type> static Type Acos(Type f) noexcept { if constexpr (IsSame<Type, F32>) { return acosf(f); } else { return acos(f); } }
	template <FloatingPoint Type> static Type Atan(Type f) noexcept { if constexpr (IsSame<Type, F32>) { return atanf(f); } else { return atan(f); } }
	template <FloatingPoint Type> static Type AsinH(Type f) noexcept { if constexpr (IsSame<Type, F32>) { return asinfh(f); } else { return asinh(f); } }
	template <FloatingPoint Type> static Type AcosH(Type f) noexcept { if constexpr (IsSame<Type, F32>) { return acosfh(f); } else { return acosh(f); } }
	template <FloatingPoint Type> static Type AtanH(Type f) noexcept { if constexpr (IsSame<Type, F32>) { return atanfh(f); } else { return atanh(f); } }
	template <FloatingPoint Type> static Type Atan2(Type x, const Type y) noexcept { if constexpr (IsSame<Type, F32>) { return atan2f(x, y); } else { return atan2(x, y); } }

	template <FloatingPoint Type> static constexpr Type Sqrt(Type f) noexcept
	{
		if (ConstantEvaluation())
		{
			if (f < 0.0) { return Traits<Type>::NaN; }

			Type result = f;
			result = (Type)0.5 * (result + f / result);
			result = (Type)0.5 * (result + f / result);
			result = (Type)0.5 * (result + f / result);
			result = (Type)0.5 * (result + f / result);
			result = (Type)0.5 * (result + f / result);
			result = (Type)0.5 * (result + f / result);
			result = (Type)0.5 * (result + f / result);
			result = (Type)0.5 * (result + f / result);
			result = (Type)0.5 * (result + f / result);
			result = (Type)0.5 * (result + f / result);

			return result;
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return sqrtf(f); }
			else { return sqrt(f); }
		}
	}
	template <FloatingPoint Type> static Type Cbrt(Type f) noexcept { if constexpr (IsSame<Type, F32>) { return cbrtf(f); } else { return cbrt(f); } }
	template <FloatingPoint Type> static constexpr Type InvSqrt(Type f) noexcept { return (Type)1.0 / Sqrt(f); }
	template <FloatingPoint Type> static constexpr Type LogE(Type f) noexcept
	{
		if (ConstantEvaluation())
		{
			if (IsNaN(f)) { return Traits<Type>::NaN; }
			else if (f < 0) { return Traits<Type>::NaN; }
			else if (f < Traits<Type>::Epsilon) { return -Traits<Type>::Infinity; }
			else if (Abs(f - (Type)1.0) < Traits<Type>::Epsilon) { return (Type)0.0; }
			else if (f == Traits<Type>::Infinity) { return Traits<Type>::Infinity; }
			else
			{
				if (f < (Type)0.5 || f >(Type)1.5)
				{
					Type mantissa = FindMantissa(f);
					Type logMantissa = LogMain(mantissa / FloorF(mantissa)) + LogMantissa<Type>((I32)mantissa);

					return logMantissa + LOG_TEN_T<Type> *FindExponent(f, 0);
				}
				else
				{
					return LogMain(f);
				}
			}
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return logf(f); }
			else { return log(f); }
		}
	}
	template <FloatingPoint Type> static Type Log2(Type f) noexcept { if constexpr (IsSame<Type, F32>) { return log2f(f); } else { return log2(f); } }
	template <FloatingPoint Type> static Type Log10(Type f) noexcept { if constexpr (IsSame<Type, F32>) { return log10f(f); } else { return log10(f); } }
	template <FloatingPoint Type> static Type LogN(Type f, const Type b) noexcept { if constexpr (IsSame<Type, F32>) { return log2f(f) / log2f(b); } else { return log2(f) / log2(b); } }
	template <FloatingPoint Type> static constexpr Type Exp(Type f) noexcept
	{
		if (ConstantEvaluation())
		{
			if (IsNaN(f)) { return Traits<Type>::NaN; }
			else if (IsNegInf(f)) { return (Type)0; }
			else if (Abs(f) < Traits<Type>::Epsilon) { return 1; }
			else if (IsInf(f)) { return Traits<Type>::Infinity; }
			else if (Abs(f) < 2.0) { return (Type)1.0 / ((Type)1.0 - f / ExpRecur(f, 2)); }
			else { I64 frac = FindFraction(f); return Pow(E_T<Type>, FindWhole(f)) * ((Type)1.0 / ((Type)1.0 - frac / ExpRecur((Type)frac, 2))); }
		}
		else
		{
			if constexpr (IsSame<Type, F32>) { return expf(f); }
			else { return exp(f); }
		}
	}
	template <Number Base, Integer Exp> static constexpr Base Pow(Base b, Exp e) noexcept
	{
		if (ConstantEvaluation())
		{
			if (e == 3) { return b * b * b; }
			else if (e == 2) { return b * b; }
			else if (e == 1) { return b; }
			else if (e == 0) { return 1; }
			else if (e == Traits<Base>::MinValue) { return 0; }
			else if (e == Traits<Base>::MaxValue) { return Traits<Base>::Infinity; }
			else { return PowBySquaring(b, (Base)1, e); }
		}
		else
		{
			if constexpr (IsInteger<Base>) { return (Base)pow((F64)b, (F64)e); }
			else if constexpr (IsSame<Base, F32>) { return powf(b, (F32)e); }
			else { return pow(b, (F64)e); }
		}
	}
	template <Number Base, FloatingPoint Exp> static constexpr Exp Pow(Base b, Exp e) noexcept
	{
		if (ConstantEvaluation())
		{
			if (b < 0.0) { return Traits<Exp>::NaN; }
			else { return Exp(e * LogE(b)); }
		}
		else
		{
			if constexpr (IsSame<Exp, F32>) { return powf((F32)b, e); }
			else { return pow((F64)b, e); }
		}
	}

	template <FloatingPoint Type> static constexpr Type Remap(Type iMin, Type iMax, Type oMin, Type oMax, Type t) noexcept { return Lerp(oMin, oMax, InvLerp(iMin, iMax, t)); } //TODO: Vector for output
	template <FloatingPoint Type> static constexpr Type MoveTowards(Type a, Type b, Type t) noexcept { return Abs(b - a) <= t ? b : a + Sin(b - a) * t; }
	template <class Type, FloatingPoint FP> static constexpr Type Lerp(Type a, Type b, FP t) noexcept { return a + (b - a) * t; }
	template <class Type, FloatingPoint FP> static constexpr Type InvLerp(Type a, Type b, FP t) noexcept { return (t - a) / (b - a); }
	template <class Type> static constexpr Type Tween(const Type& a, const Type& b) noexcept { return Lerp(a, b, 1.0f - Math::Pow(0.1f, (F32)Time::DeltaTime())); }
	template <class Type, FloatingPoint F> static constexpr Type Tween(const Type& a, const Type& b, F power) noexcept { return Lerp(a, b, (F)1.0 - Math::Pow(power, (F)Time::DeltaTime())); }
	template <class Type> static constexpr Type Trajectory(Type start, Type velocity, Type acceleration, Type jerk, F32 t) noexcept
	{
		F32 t2 = t * t;
		F32 t3 = t2 * t;
	
		F32 f3 = t2 * 0.5f;
		F32 f4 = t3 * 0.166666667f;
	
		return start + velocity * t + acceleration * f3 + jerk * f4;
	}

private:

	STATIC_CLASS(Math);
};

export class NH_API Noise
{
public:
	static constexpr F32 Simplex1(F32 x) noexcept
	{
		F32 i0 = Math::FloorF(x);
		F32 x0 = x - i0;
		F32 x1 = x0 - 1.0f;

		F32 t0 = 1.0f - x0 * x0;
		t0 *= t0;

		F32 t1 = 1.0f - x1 * x1;
		t1 *= t1;

		return 0.395f * ((t0 * t0 * Grad(simplexPerm[(I64)Math::Mod(i0, 256.0f)], x0)) + (t1 * t1 * Grad(simplexPerm[(I64)Math::Mod(i0 + 1, 256.0f)], x1)));
	}
	static constexpr F32 Simplex2(F32 x, F32 y) noexcept
	{
		F32 n0, n1, n2;

		F32 g = (x + y) * F2;
		F32 i = Math::FloorF(x + g);
		F32 j = Math::FloorF(y + g);

		F32 t = (i + j) * G2;
		F32 X0 = i - t;
		F32 Y0 = j - t;
		F32 x0 = x - X0;
		F32 y0 = y - Y0;

		I64 i1, j1;
		if (x0 > y0) { i1 = 1; j1 = 0; }
		else { i1 = 0; j1 = 1; }

		F32 x1 = x0 - i1 + G2;
		F32 y1 = y0 - j1 + G2;
		F32 x2 = x0 - 1.0f + 2.0f * G2;
		F32 y2 = y0 - 1.0f + 2.0f * G2;

		I64 ii = (I64)Math::Mod(i, 256.0f);
		I64 jj = (I64)Math::Mod(j, 256.0f);
		I64 gi0 = simplexPerm[ii + simplexPerm[jj]] % 12;
		I64 gi1 = simplexPerm[ii + i1 + simplexPerm[jj + j1]] % 12;
		I64 gi2 = simplexPerm[ii + 1 + simplexPerm[jj + 1]] % 12;
		F32 t0 = 0.5f - x0 * x0 - y0 * y0;

		if (t0 < 0) { n0 = 0.0f; }
		else
		{
			t0 *= t0;
			n0 = t0 * t0 * Dot(simplexGrad2[gi0][0], simplexGrad2[gi0][1], x0, y0);
		}

		F32 t1 = 0.5f - x1 * x1 - y1 * y1;

		if (t1 < 0) { n1 = 0.0f; }
		else
		{
			t1 *= t1;
			n1 = t1 * t1 * Dot(simplexGrad2[gi1][0], simplexGrad2[gi1][1], x1, y1);
		}

		F32 t2 = 0.5f - x2 * x2 - y2 * y2;

		if (t2 < 0) { n2 = 0.0f; }
		else
		{
			t2 *= t2;
			n2 = t2 * t2 * Dot(simplexGrad2[gi2][0], simplexGrad2[gi2][1], x2, y2);
		}

		return 70.0f * (n0 + n1 + n2);
	}
	static constexpr F32 Simplex3(F32 x, F32 y, F32 z) noexcept
	{
		F32 n0, n1, n2, n3;

		F32 s = (x + y + z) * F3;
		I64 i = Math::Floor(x + s);
		I64 j = Math::Floor(y + s);
		I64 k = Math::Floor(z + s);
		F32 t = (i + j + k) * G3;
		F32 X0 = i - t;
		F32 Y0 = j - t;
		F32 Z0 = k - t;
		F32 x0 = x - X0;
		F32 y0 = y - Y0;
		F32 z0 = z - Z0;

		I64 i1, j1, k1;
		I64 i2, j2, k2;
		if (x0 >= y0)
		{
			if (y0 >= z0)
			{
				i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
			}
			else if (x0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; }
			else { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; }
		}
		else
		{
			if (y0 < z0) { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; }
			else if (x0 < z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; }
			else { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; }
		}

		F32 x1 = x0 - i1 + G3;
		F32 y1 = y0 - j1 + G3;
		F32 z1 = z0 - k1 + G3;
		F32 x2 = x0 - i2 + 2.0f * G3;
		F32 y2 = y0 - j2 + 2.0f * G3;
		F32 z2 = z0 - k2 + 2.0f * G3;
		F32 x3 = x0 - 1.0f + 3.0f * G3;
		F32 y3 = y0 - 1.0f + 3.0f * G3;
		F32 z3 = z0 - 1.0f + 3.0f * G3;

		I64 ii = i & 255;
		I64 jj = j & 255;
		I64 kk = k & 255;
		I64 gi0 = simplexPerm[ii + simplexPerm[jj + simplexPerm[kk]]] % 12;
		I64 gi1 = simplexPerm[ii + i1 + simplexPerm[jj + j1 + simplexPerm[kk + k1]]] % 12;
		I64 gi2 = simplexPerm[ii + i2 + simplexPerm[jj + j2 + simplexPerm[kk + k2]]] % 12;
		I64 gi3 = simplexPerm[ii + 1 + simplexPerm[jj + 1 + simplexPerm[kk + 1]]] % 12;

		F32 t0 = 0.5f - x0 * x0 - y0 * y0 - z0 * z0;
		if (t0 < 0) { n0 = 0.0f; }
		else
		{
			t0 *= t0;
			n0 = t0 * t0 * Dot(simplexGrad3[gi0][0], simplexGrad3[gi0][1], simplexGrad3[gi0][2], x0, y0, z0);
		}
		F32 t1 = 0.5f - x1 * x1 - y1 * y1 - z1 * z1;
		if (t1 < 0) { n1 = 0.0f; }
		else
		{
			t1 *= t1;
			n1 = t1 * t1 * Dot(simplexGrad3[gi1][0], simplexGrad3[gi1][1], simplexGrad3[gi1][2], x1, y1, z1);
		}
		F32 t2 = 0.5f - x2 * x2 - y2 * y2 - z2 * z2;
		if (t2 < 0) { n2 = 0.0f; }
		else
		{
			t2 *= t2;
			n2 = t2 * t2 * Dot(simplexGrad3[gi2][0], simplexGrad3[gi2][1], simplexGrad3[gi2][2], x2, y2, z2);
		}
		F32 t3 = 0.5f - x3 * x3 - y3 * y3 - z3 * z3;
		if (t3 < 0) { n3 = 0.0f; }
		else
		{
			t3 *= t3;
			n3 = t3 * t3 * Dot(simplexGrad3[gi3][0], simplexGrad3[gi3][1], simplexGrad3[gi3][2], x3, y3, z3);
		}

		return 32.0f * (n0 + n1 + n2 + n3);
	}

private:
	static constexpr F32 Grad(I64 hash, F32 x)
	{
		I64 r = hash & 15;
		return ((1.0f + (r & 7)) * x) * ((r & 8) ? -1 : 1);
	}
	static constexpr F32 Dot(F32 x0, F32 y0, F32 x1, F32 y1)
	{
		return x0 * x1 + y0 * y1;
	}
	static constexpr F32 Dot(F32 x0, F32 y0, F32 z0, F32 x1, F32 y1, F32 z1)
	{
		return x0 * x1 + y0 * y1 + z0 * z1;
	}

	static constexpr inline U8 simplexPerm[512]{
		151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,
		69,142,8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,
		252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,
		171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,
		122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,65,25,63,
		161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,
		159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,
		147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,
		183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,
		129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,
		228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,
		239,107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,
		4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,
		61,156,180,151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,
		36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,
		62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,
		136,171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,
		229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,65,25,
		63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,
		159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,
		118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,
		170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,
		39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,
		34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,
		49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};
	static constexpr inline F32 simplexGrad2[12][2]{
		{ 1.0, 1.0 }, {-1.0, 1.0 }, { 1.0,-1.0 }, {-1.0,-1.0 },
		{ 1.0, 0.0 }, {-1.0, 0.0 }, { 1.0, 0.0 }, {-1.0, 0.0 },
		{ 0.0, 1.0 }, { 0.0,-1.0 }, { 0.0, 1.0 }, { 0.0,-1.0 }
	};
	static constexpr inline F32 simplexGrad3[12][3]{
		{ 1.0, 1.0, 0.0 }, {-1.0, 1.0, 0.0 }, { 1.0,-1.0, 0.0 }, {-1.0,-1.0, 0.0 },
		{ 1.0, 0.0, 1.0 }, {-1.0, 0.0, 1.0 }, { 1.0, 0.0,-1.0 }, {-1.0, 0.0,-1.0 },
		{ 0.0, 1.0, 1.0 }, { 0.0,-1.0, 1.0 }, { 0.0, 1.0,-1.0 }, { 0.0,-1.0,-1.0 }
	};
	static constexpr inline F32 F2 = 0.36602540378f;
	static constexpr inline F32 G2 = 0.2113248654f;
	static constexpr inline F32 F3 = 1.0f / 3.0f;
	static constexpr inline F32 G3 = 1.0f / 6.0f;

	STATIC_CLASS(Noise);
};

//Based on wyrand - https://github.com/wangyi-fudan/wyhash
export class NH_API Random
{
public:
	/// <summary>
	/// Creates a completely unpredictable random integer
	/// </summary>
	/// <returns>The random number</returns>
	static U64 TrueRandomInt()
	{
		I64 time = Time::CoreCounter();
		time = Mix(time ^ secret0, seed ^ secret1);
		seed = Mix(time ^ secret0, secret2);
		return Mix(seed, seed ^ secret3);
	}

	/// <summary>
	/// Creates a deterministically random integer based on the current seed
	/// </summary>
	/// <returns>The random number</returns>
	static U64 RandomInt()
	{
		seed += secret0;
		return Mix(seed, seed ^ secret1);
	}

	/// <summary>
	/// Creates a deterministically random integer within the range based on the current seed
	/// </summary>
	/// <param name="lower:">Lower bound, inclusive</param>
	/// <param name="upper:">Upper bound, exclusive</param>
	/// <returns>The random number</returns>
	static U64 RandomRange(U64 lower, U64 upper)
	{
		U64 num = upper + lower;
		U64 rand = RandomInt();
		Multiply(rand, num);
		return num - lower;
	}

	/// <summary>
	/// Creates a deterministically random integer with a uniform distribution based on the current seed
	/// </summary>
	/// <returns>The random number</returns>
	static F64 RandomUniform()
	{
		constexpr F64 norm = 1.0 / (1ull << 52);
		U64 rand = RandomInt();
		return (rand >> 12) * norm;
	}

	/// <summary>
	/// Creates a deterministically random integer with a gausian distribution based on the current seed
	/// </summary>
	/// <returns>The random number</returns>
	static F64 RandomGausian()
	{
		constexpr F64 norm = 1.0 / (1ull << 20);
		U64 rand = RandomInt();
		return ((rand & 0x1fffff) + ((rand >> 21) & 0x1fffff) + ((rand >> 42) & 0x1fffff)) * norm - 3.0;
	}

	/// <summary>
	/// Sets the seed used to generate random numbers, seed get automatically updated whenever any of these functions are called
	/// </summary>
	/// <param name="newSeed:">The new seed value</param>
	static void SeedRandom(U64 newSeed) { seed = newSeed; }

	template<Character C = C8>
	static StringBase<C> RandomString(U32 length) noexcept
	{
		String str{};
		str.Resize(16);

		C* it = str.Data();

		for (U32 i = 0; i < length; ++i)
		{
			*it++ = StringLookup<C>::ALPHANUM_LOOKUP[RandomRange(0, Length(StringLookup<C>::ALPHANUM_LOOKUP))];
		}

		return str;
	}

private:
	static inline U64 seed{ 0 };

	static void Multiply(U64& a, U64& b)
	{
#if defined __SIZEOF_INT128__
		__uint128_t r = a;
		r *= b;
		a = (U64)r;
		b = (U64)(r >> 64);
#elif defined _MSC_VER
		a = _umul128(a, b, &b);
#else
		U64 ha = a >> 32, hb = b >> 32, la = (U32)A, lb = (U32)B, hi, lo;
		U64 rh = ha * hb, rm0 = ha * lb, rm1 = hb * la, rl = la * lb, t = rl + (rm0 << 32), c = t < rl;
		lo = t + (rm1 << 32);
		c += lo < t;
		hi = rh + (rm0 >> 32) + (rm1 >> 32) + c;
		a = lo;
		b = hi;
#endif
	}
	static U64 Mix(U64 a, U64 b) { Multiply(a, b); return a ^ b; }
	static U64 Read8(const U8* p)
	{
#if defined NH_LITTLE_ENDIAN
		U64 v;
		memcpy(&v, p, 8);
		return v;
#elif defined __GNUC__ || defined __INTEL_COMPILER || defined __clang__
		U64 v;
		memcpy(&v, p, 8);
		return __builtin_bswap64(v);
#elif defined _MSC_VER
		U64 v;
		memcpy(&v, p, 8);
		return _byteswap_uint64(v);
#else
		U64 v;
		memcpy(&v, p, 8);
		return (((v >> 56) & 0xff) | ((v >> 40) & 0xff00) | ((v >> 24) & 0xff0000) | ((v >> 8) & 0xff000000) |
			((v << 8) & 0xff00000000) | ((v << 24) & 0xff0000000000) | ((v << 40) & 0xff000000000000) | ((v << 56) & 0xff00000000000000));
#endif
	}
	static U64 Read4(const U8* p)
	{
#if defined NH_LITTLE_ENDIAN
		U32 v;
		memcpy(&v, p, 4);
		return v;
#elif defined __GNUC__ || defined __INTEL_COMPILER || defined __clang__
		U32 v;
		memcpy(&v, p, 4);
		return __builtin_bswap32(v);
#elif defined _MSC_VER
		U32 v;
		memcpy(&v, p, 4);
		return _byteswap_ulong(v);
#else
		U32 v;
		memcpy(&v, p, 4);
		return (((v >> 24) & 0xff) | ((v >> 8) & 0xff00) | ((v << 8) & 0xff0000) | ((v << 24) & 0xff000000));
#endif
	}
	static U64 Read3(const U8* p, U64 k) { return (((U64)p[0]) << 16) | (((U64)p[k >> 1]) << 8) | p[k - 1]; }

	STATIC_CLASS(Random);
};

//Based on wyhash - https://github.com/wangyi-fudan/wyhash
export class NH_API Hash
{
public:
	/// <summary>
	/// Creates a hash for a string literal at compile-time, faster but less unique
	/// </summary>
	/// <param name="str:">The string literal</param>
	/// <param name="length:">The length of the string</param>
	/// <returns>The hash</returns>
	static constexpr U64 StringHash(const C8* str, U64 length)
	{
		U64 hash = 5381;
		U64 i = 0;

		for (i = 0; i < length; ++str, ++i)
		{
			hash = ((hash << 5) + hash) + (*str);
		}

		return hash;
	}

	/// <summary>
	/// Creates a hash for a string literal at compile-time, case insensitive, faster but less unique
	/// </summary>
	/// <param name="str:">The string literal</param>
	/// <param name="length:">The length of the string</param>
	/// <returns>The hash</returns>
	static constexpr U64 StringHashCI(const C8* str, U64 length)
	{
		U64 hash = 5381;
		U64 i = 0;

		for (i = 0; i < length; ++str, ++i)
		{
			C8 c = *str;
			if (c < 97) { c += 32; }

			hash = ((hash << 5) + hash) + c;
		}

		return hash;
	}

	/// <summary>
	/// Creates a hash for a string view at compile-time, faster but less unique
	/// </summary>
	/// <param name="string:">The string view</param>
	/// <returns>The hash</returns>
	static constexpr U64 StringHash(const StringView& string)
	{
		U64 hash = 5381;
		U64 i = 0;

		const C8* str = string.Data();

		for (i = 0; i < string.Size(); ++str, ++i)
		{
			hash = ((hash << 5) + hash) + (*str);
		}

		return hash;
	}

	/// <summary>
	/// Creates a hash for a string view at compile-time, case insensitive, faster but less unique
	/// </summary>
	/// <param name="string:">The string view</param>
	/// <returns>The hash</returns>
	static constexpr U64 StringHashCI(const StringView& string)
	{
		U64 hash = 5381;
		U64 i = 0;

		const C8* str = string.Data();

		for (i = 0; i < string.Size(); ++str, ++i)
		{
			C8 c = *str;
			if (c < 97) { c += 32; }

			hash = ((hash << 5) + hash) + c;
		}

		return hash;
	}

	/// <summary>
	/// Creates a hash for any type, slower but more unique
	/// </summary>
	/// <param name="value:">The value to hash</param>
	/// <param name="seed:">The seed to use</param>
	/// <returns>The hash</returns>
	template<class Type> requires(!IsPointer<Type>)
	static U64 SeededHash(const Type& value, U64 seed = 0)
	{
		constexpr U64 length = sizeof(Type);

		const U8* p = (const U8*)&value;
		seed ^= Mix(seed ^ secret0, secret1);

		U64	a, b;
		if constexpr (length <= 16)
		{
			if constexpr (length >= 4)
			{
				a = (Read4(p) << 32) | Read4(p + ((length >> 3) << 2));
				b = (Read4(p + length - 4) << 32) | Read4(p + length - 4 - ((length >> 3) << 2));
			}
			else if constexpr (length > 0) { a = Read3(p, length); b = 0; }
			else { a = b = 0; }
		}
		else
		{
			U64 i = length;
			if constexpr (length > 48)
			{
				U64 seed1 = seed, seed2 = seed;
				do
				{
					seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed);
					seed1 = Mix(Read8(p + 16) ^ secret2, Read8(p + 24) ^ seed1);
					seed2 = Mix(Read8(p + 32) ^ secret3, Read8(p + 40) ^ seed2);
					p += 48;
					i -= 48;
				} while (i > 48);

				seed ^= seed1 ^ seed2;
			}

			while (i > 16)
			{
				seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed);
				i -= 16;
				p += 16;
			}

			a = Read8(p + i - 16);
			b = Read8(p + i - 8);
		}

		a ^= secret1;
		b ^= seed;
		Multiply(a, b);
		return Mix(a ^ secret0 ^ length, b ^ secret1);
	}

	template<>
	static U64 SeededHash<StringBase<C8>>(const StringBase<C8>& value, U64 seed)
	{
		return SeededHash(value.Data(), seed);
	}

	template<>
	static U64 SeededHash<StringBase<C16>>(const StringBase<C16>& value, U64 seed)
	{
		return SeededHash(value.Data(), seed);
	}

	template<>
	static U64 SeededHash<StringBase<C32>>(const StringBase<C32>& value, U64 seed)
	{
		return SeededHash(value.Data(), seed);
	}

	template<>
	static U64 SeededHash<StringBase<CW>>(const StringBase<CW>& value, U64 seed)
	{
		return SeededHash(value.Data(), seed);
	}

	/// <summary>
	/// Creates a hash for any type, slower but more unique
	/// </summary>
	/// <param name="value:">The value to hash</param>
	/// <param name="seed:">The seed to use</param>
	/// <returns>The hash</returns>
	template <class Type, U64 len>
	static U64 SeededHash(const Type(&value)[len], U64 seed = 0)
	{
		constexpr U64 length = len * sizeof(Type);
		const U8* p = (const U8*)value;
		seed ^= Mix(seed ^ secret0, secret1);

		U64	a, b;
		if constexpr (length <= 16)
		{
			if constexpr (length >= 4)
			{
				a = (Read4(p) << 32) | Read4(p + ((length >> 3) << 2));
				b = (Read4(p + length - 4) << 32) | Read4(p + length - 4 - ((length >> 3) << 2));
			}
			else if constexpr (length > 0) { a = Read3(p, length); b = 0; }
			else { a = b = 0; }
		}
		else
		{
			U64 i = length;
			if constexpr (length > 48)
			{
				U64 seed1 = seed, seed2 = seed;
				do
				{
					seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed);
					seed1 = Mix(Read8(p + 16) ^ secret2, Read8(p + 24) ^ seed1);
					seed2 = Mix(Read8(p + 32) ^ secret3, Read8(p + 40) ^ seed2);
					p += 48;
					i -= 48;
				} while (i > 48);

				seed ^= seed1 ^ seed2;
			}

			while (i > 16)
			{
				seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed);
				i -= 16;
				p += 16;
			}

			a = Read8(p + i - 16);
			b = Read8(p + i - 8);
		}

		a ^= secret1;
		b ^= seed;
		Multiply(a, b);
		return Mix(a ^ secret0 ^ length, b ^ secret1);
	}

	/// <summary>
	/// Creates a hash for any type, slower but more unique
	/// </summary>
	/// <param name="value:">The value to hash</param>
	/// <param name="seed:">The seed to use</param>
	/// <returns>The hash</returns>
	template<class Type>
	static U64 SeededHash(const Type* value, U64 len, U64 seed = 0)
	{
		const U64 length = len * sizeof(Type);
		const U8* p = (const U8*)value;
		seed ^= Mix(seed ^ secret0, secret1);

		U64	a, b;
		if (length <= 16)
		{
			if (length >= 4)
			{
				a = (Read4(p) << 32) | Read4(p + ((length >> 3) << 2));
				b = (Read4(p + length - 4) << 32) | Read4(p + length - 4 - ((length >> 3) << 2));
			}
			else if (length > 0) { a = Read3(p, length); b = 0; }
			else { a = b = 0; }
		}
		else
		{
			U64 i = length;
			if (length > 48)
			{
				U64 seed1 = seed, seed2 = seed;
				do
				{
					seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed);
					seed1 = Mix(Read8(p + 16) ^ secret2, Read8(p + 24) ^ seed1);
					seed2 = Mix(Read8(p + 32) ^ secret3, Read8(p + 40) ^ seed2);
					p += 48;
					i -= 48;
				} while (i > 48);

				seed ^= seed1 ^ seed2;
			}

			while (i > 16)
			{
				seed = Mix(Read8(p) ^ secret1, Read8(p + 8) ^ seed);
				i -= 16;
				p += 16;
			}

			a = Read8(p + i - 16);
			b = Read8(p + i - 8);
		}

		a ^= secret1;
		b ^= seed;
		Multiply(a, b);
		return Mix(a ^ secret0 ^ length, b ^ secret1);
	}

private:
	static void Multiply(U64& a, U64& b)
	{
#if defined __SIZEOF_INT128__
		__uint128_t r = a;
		r *= b;
		a = (U64)r;
		b = (U64)(r >> 64);
#elif defined _MSC_VER
		a = _umul128(a, b, &b);
#else
		U64 ha = a >> 32, hb = b >> 32, la = (U32)A, lb = (U32)B, hi, lo;
		U64 rh = ha * hb, rm0 = ha * lb, rm1 = hb * la, rl = la * lb, t = rl + (rm0 << 32), c = t < rl;
		lo = t + (rm1 << 32);
		c += lo < t;
		hi = rh + (rm0 >> 32) + (rm1 >> 32) + c;
		a = lo;
		b = hi;
#endif
	}
	static U64 Mix(U64 a, U64 b) { Multiply(a, b); return a ^ b; }
	static U64 Read8(const U8* p)
	{
#if defined NH_LITTLE_ENDIAN
		U64 v;
		memcpy(&v, p, 8);
		return v;
#elif defined __GNUC__ || defined __INTEL_COMPILER || defined __clang__
		U64 v;
		memcpy(&v, p, 8);
		return __builtin_bswap64(v);
#elif defined _MSC_VER
		U64 v;
		memcpy(&v, p, 8);
		return _byteswap_uint64(v);
#else
		U64 v;
		memcpy(&v, p, 8);
		return (((v >> 56) & 0xff) | ((v >> 40) & 0xff00) | ((v >> 24) & 0xff0000) | ((v >> 8) & 0xff000000) |
			((v << 8) & 0xff00000000) | ((v << 24) & 0xff0000000000) | ((v << 40) & 0xff000000000000) | ((v << 56) & 0xff00000000000000));
#endif
	}
	static U64 Read4(const U8* p)
	{
#if defined NH_LITTLE_ENDIAN
		U32 v;
		memcpy(&v, p, 4);
		return v;
#elif defined __GNUC__ || defined __INTEL_COMPILER || defined __clang__
		U32 v;
		memcpy(&v, p, 4);
		return __builtin_bswap32(v);
#elif defined _MSC_VER
		U32 v;
		memcpy(&v, p, 4);
		return _byteswap_ulong(v);
#else
		U32 v;
		memcpy(&v, p, 4);
		return (((v >> 24) & 0xff) | ((v >> 8) & 0xff00) | ((v << 8) & 0xff0000) | ((v << 24) & 0xff000000));
#endif
	}
	static U64 Read3(const U8* p, U64 k) { return (((U64)p[0]) << 16) | (((U64)p[k >> 1]) << 8) | p[k - 1]; }

	STATIC_CLASS(Hash);
};

export constexpr inline U64 operator""_Hash(const C8 * str, U64 length) { return Hash::StringHash(str, length); }
export constexpr inline U64 operator""_HashCI(const C8 * str, U64 length) { return Hash::StringHashCI(str, length); }