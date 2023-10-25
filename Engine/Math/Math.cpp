#include "Math.hpp"

#include "Containers\String.hpp"

#include "SIMD.hpp"

//Math

Quaternion2 Math::Slerp(const Quaternion2& a, const Quaternion2& b, F32 t) noexcept
{
	static constexpr F32 DOT_THRESHOLD = 0.9995f;

	Quaternion2 v0 = a.Normalize();
	Quaternion2 v1 = b.Normalize();

	F32 dot = v0.Dot(v1);

	if (dot < 0.0f)
	{
		v1.x = -v1.x;
		v1.y = -v1.y;
		dot = -dot;
	}

	if (dot > DOT_THRESHOLD)
	{
		Quaternion2 out{
			v0.x + ((v1.x - v0.x) * t),
			v0.y + ((v1.y - v0.y) * t)
		};

		return out.Normalize();
	}

	F32 theta0 = Math::Acos(dot);
	F32 theta = theta0 * t;
	F32 sinTheta = Math::Sin(theta);
	F32 sinTheta0 = Math::Sin(theta0);

	F32 s0 = Math::Cos(theta) - dot * sinTheta / sinTheta0;
	F32 s1 = sinTheta / sinTheta0;

	return {
		v0.x * s0 + v1.x * s1,
		v0.y * s0 + v1.y * s1
	};
}

Quaternion3 Math::Slerp(const Quaternion3& a, const Quaternion3& b, F32 t) noexcept
{
	static constexpr F32 DOT_THRESHOLD = 0.9995f;

	Quaternion3 v0 = a.Normalize();
	Quaternion3 v1 = b.Normalize();

	F32 dot = v0.Dot(v1);

	if (dot < 0.0f)
	{
		v1.x = -v1.x;
		v1.y = -v1.y;
		v1.z = -v1.z;
		v1.w = -v1.w;
		dot = -dot;
	}

	if (dot > DOT_THRESHOLD)
	{
		Quaternion3 out{
			v0.x + ((v1.x - v0.x) * t),
			v0.y + ((v1.y - v0.y) * t),
			v0.z + ((v1.z - v0.z) * t),
			v0.w + ((v1.w - v0.w) * t)
		};

		return out.Normalize();
	}

	F32 theta0 = Math::Acos(dot);
	F32 theta = theta0 * t;
	F32 sinTheta = Math::Sin(theta);
	F32 sinTheta0 = Math::Sin(theta0);

	F32 s0 = Math::Cos(theta) - dot * sinTheta / sinTheta0;
	F32 s1 = sinTheta / sinTheta0;

	return {
		v0.x * s0 + v1.x * s1,
		v0.y * s0 + v1.y * s1,
		v0.z * s0 + v1.z * s1,
		v0.w * s0 + v1.w * s1
	};
}

//NOISE
static U8 simplexPerm[512]{
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

static F64 simplexGrad2[12][2]{
	{ 1.0, 1.0 }, {-1.0, 1.0 }, { 1.0,-1.0 }, {-1.0,-1.0 },
	{ 1.0, 0.0 }, {-1.0, 0.0 }, { 1.0, 0.0 }, {-1.0, 0.0 },
	{ 0.0, 1.0 }, { 0.0,-1.0 }, { 0.0, 1.0 }, { 0.0,-1.0 }
};

static F64 simplexGrad3[12][3]{
	{ 1.0, 1.0, 0.0 }, {-1.0, 1.0, 0.0 }, { 1.0,-1.0, 0.0 }, {-1.0,-1.0, 0.0 },
	{ 1.0, 0.0, 1.0 }, {-1.0, 0.0, 1.0 }, { 1.0, 0.0,-1.0 }, {-1.0, 0.0,-1.0 },
	{ 0.0, 1.0, 1.0 }, { 0.0,-1.0, 1.0 }, { 0.0, 1.0,-1.0 }, { 0.0,-1.0,-1.0 }
};

NH_INLINE F64 Grad(I64 hash, F64 x)
{
	I64 r = hash & 15;
	return ((1.0 + (r & 7)) * x) * ((r & 8) ? -1 : 1);
}

NH_INLINE F64 Dot(F64 x0, F64 y0, F64 x1, F64 y1)
{
	return x0 * x1 + y0 * y1;
}

NH_INLINE F64 Dot(F64 x0, F64 y0, F64 z0, F64 x1, F64 y1, F64 z1)
{
	return x0 * x1 + y0 * y1 + z0 * z1;
}

F64 Math::Simplex1(F64 x) noexcept
{
	I64 i0 = Floor(x);
	F64 x0 = x - i0;
	F64 x1 = x0 - 1.0;

	F64 t0 = 1.0 - x0 * x0;
	t0 *= t0;

	F64 t1 = 1.0 - x1 * x1;
	t1 *= t1;

	return 0.395 * ((t0 * t0 * Grad(simplexPerm[i0 & 0xff], x0)) + (t1 * t1 * Grad(simplexPerm[(i0 + 1) & 0xff], x1)));
}

F64 Math::Simplex2(F64 x, F64 y) noexcept
{
	static constexpr F64 F2 = 0.36602540378;
	static constexpr F64 G2 = 0.2113248654;

	F64 n0, n1, n2;

	F64 g = (x + y) * F2;
	I64 i = Floor(x + g);
	I64 j = Floor(y + g);

	F64 t = (i + j) * G2;
	F64 X0 = i - t;
	F64 Y0 = j - t;
	F64 x0 = x - X0;
	F64 y0 = y - Y0;

	I64 i1, j1;
	if (x0 > y0) { i1 = 1; j1 = 0; }
	else { i1 = 0; j1 = 1; }

	F64 x1 = x0 - i1 + G2;
	F64 y1 = y0 - j1 + G2;
	F64 x2 = x0 - 1.0 + 2.0 * G2;
	F64 y2 = y0 - 1.0 + 2.0 * G2;

	I64 ii = i & 255;
	I64 jj = j & 255;
	I64 gi0 = simplexPerm[ii + simplexPerm[jj]] % 12;
	I64 gi1 = simplexPerm[ii + i1 + simplexPerm[jj + j1]] % 12;
	I64 gi2 = simplexPerm[ii + 1 + simplexPerm[jj + 1]] % 12;
	F64 t0 = 0.5 - x0 * x0 - y0 * y0;

	if (t0 < 0) { n0 = 0.0; }
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * Dot(simplexGrad2[gi0][0], simplexGrad2[gi0][1], x0, y0);
	}

	F64 t1 = 0.5 - x1 * x1 - y1 * y1;

	if (t1 < 0) { n1 = 0.0; }
	else
	{
		t1 *= t1;
		n1 = t1 * t1 * Dot(simplexGrad2[gi1][0], simplexGrad2[gi1][1], x1, y1);
	}

	F64 t2 = 0.5 - x2 * x2 - y2 * y2;

	if (t2 < 0) { n2 = 0.0; }
	else
	{
		t2 *= t2;
		n2 = t2 * t2 * Dot(simplexGrad2[gi2][0], simplexGrad2[gi2][1], x2, y2);
	}

	return 70.0 * (n0 + n1 + n2);
}

F64 Math::Simplex3(F64 x, F64 y, F64 z) noexcept
{
	static constexpr F64 F3 = 1.0 / 3.0;
	static constexpr F64 G3 = 1.0 / 6.0;

	F64 n0, n1, n2, n3;

	F64 s = (x + y + z) * F3;
	I64 i = Floor(x + s);
	I64 j = Floor(y + s);
	I64 k = Floor(z + s);
	F64 t = (i + j + k) * G3;
	F64 X0 = i - t;
	F64 Y0 = j - t;
	F64 Z0 = k - t;
	F64 x0 = x - X0;
	F64 y0 = y - Y0;
	F64 z0 = z - Z0;

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

	F64 x1 = x0 - i1 + G3;
	F64 y1 = y0 - j1 + G3;
	F64 z1 = z0 - k1 + G3;
	F64 x2 = x0 - i2 + 2.0 * G3;
	F64 y2 = y0 - j2 + 2.0 * G3;
	F64 z2 = z0 - k2 + 2.0 * G3;
	F64 x3 = x0 - 1.0 + 3.0 * G3;
	F64 y3 = y0 - 1.0 + 3.0 * G3;
	F64 z3 = z0 - 1.0 + 3.0 * G3;

	I64 ii = i & 255;
	I64 jj = j & 255;
	I64 kk = k & 255;
	I64 gi0 = simplexPerm[ii + simplexPerm[jj + simplexPerm[kk]]] % 12;
	I64 gi1 = simplexPerm[ii + i1 + simplexPerm[jj + j1 + simplexPerm[kk + k1]]] % 12;
	I64 gi2 = simplexPerm[ii + i2 + simplexPerm[jj + j2 + simplexPerm[kk + k2]]] % 12;
	I64 gi3 = simplexPerm[ii + 1 + simplexPerm[jj + 1 + simplexPerm[kk + 1]]] % 12;

	F64 t0 = 0.5 - x0 * x0 - y0 * y0 - z0 * z0;
	if (t0 < 0) { n0 = 0.0; }
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * Dot(simplexGrad3[gi0][0], simplexGrad3[gi0][1], simplexGrad3[gi0][2], x0, y0, z0);
	}
	F64 t1 = 0.5 - x1 * x1 - y1 * y1 - z1 * z1;
	if (t1 < 0) { n1 = 0.0; }
	else
	{
		t1 *= t1;
		n1 = t1 * t1 * Dot(simplexGrad3[gi1][0], simplexGrad3[gi1][1], simplexGrad3[gi1][2], x1, y1, z1);
	}
	F64 t2 = 0.5 - x2 * x2 - y2 * y2 - z2 * z2;
	if (t2 < 0) { n2 = 0.0; }
	else
	{
		t2 *= t2;
		n2 = t2 * t2 * Dot(simplexGrad3[gi2][0], simplexGrad3[gi2][1], simplexGrad3[gi2][2], x2, y2, z2);
	}
	F64 t3 = 0.5 - x3 * x3 - y3 * y3 - z3 * z3;
	if (t3 < 0) { n3 = 0.0; }
	else
	{
		t3 *= t3;
		n3 = t3 * t3 * Dot(simplexGrad3[gi3][0], simplexGrad3[gi3][1], simplexGrad3[gi3][2], x3, y3, z3);
	}

	return 32.0 * (n0 + n1 + n2 + n3);
}

//Vector2

Vector2::operator String() const { return String(x, ", ", y); }
Vector2::operator String16() const { return String16(x, u", ", y); }
Vector2::operator String32() const { return String32(x, U", ", y); }

//Vector3

Vector3& Vector3::operator*=(const Quaternion3& q)
{
	F32 xx = q.x + q.x;
	F32 yy = q.y + q.y;
	F32 zz = q.z + q.z;
	F32 xxw = xx * q.w;
	F32 yyw = yy * q.w;
	F32 zzw = zz * q.w;
	F32 xxx = xx * q.x;
	F32 yyx = yy * x;
	F32 zzx = zz * q.x;
	F32 yyy = yy * q.y;
	F32 zzy = zz * q.y;
	F32 zzz = zz * z;
	x = ((x * ((1.0f - yyy) - zzz)) + (y * (yyx - zzw))) + (z * (zzx + yyw));
	y = ((x * (yyx + zzw)) + (y * ((1.0f - xxx) - zzz))) + (z * (zzy - xxw));
	z = ((x * (zzx - yyw)) + (y * (zzy + xxw))) + (z * ((1.0f - xxx) - yyy));

	return *this;
}

Vector3 Vector3::operator*(const Quaternion3& q) const
{
	F32 xx = q.x + q.x;
	F32 yy = q.y + q.y;
	F32 zz = q.z + q.z;
	F32 xxw = xx * q.w;
	F32 yyw = yy * q.w;
	F32 zzw = zz * q.w;
	F32 xxx = xx * q.x;
	F32 yyx = yy * x;
	F32 zzx = zz * q.x;
	F32 yyy = yy * q.y;
	F32 zzy = zz * q.y;
	F32 zzz = zz * z;

	return {
		((x * ((1.0f - yyy) - zzz)) + (y * (yyx - zzw))) + (z * (zzx + yyw)),
		((x * (yyx + zzw)) + (y * ((1.0f - xxx) - zzz))) + (z * (zzy - xxw)),
		((x * (zzx - yyw)) + (y * (zzy + xxw))) + (z * ((1.0f - xxx) - yyy))
	};
}

Vector3::operator String() const { return String(x, ", ", y, ", ", z); }
Vector3::operator String16() const { return String16(x, u", ", y, u", ", z); }
Vector3::operator String32() const { return String32(x, U", ", y, U", ", z); }

//Vector4

Vector4::operator String() const { return String(x, ", ", y, ", ", z, ", ", w); }
Vector4::operator String16() const { return String16(x, u", ", y, u", ", z, u", ", w); }
Vector4::operator String32() const { return String32(x, U", ", y, U", ", z, U", ", w); }

//Vector2Int

Vector2Int::operator String() const { return String(x, ", ", y); }
Vector2Int::operator String16() const { return String16(x, u", ", y); }
Vector2Int::operator String32() const { return String32(x, U", ", y); }

//Vector3Int

Vector3Int::operator String() const { return String(x, ", ", y, ", ", z); }
Vector3Int::operator String16() const { return String16(x, u", ", y, u", ", z); }
Vector3Int::operator String32() const { return String32(x, U", ", y, U", ", z); }

//Vector4Int

Vector4Int::operator String() const { return String(x, ", ", y, ", ", z, ", ", w); }
Vector4Int::operator String16() const { return String16(x, u", ", y, u", ", z, u", ", w); }
Vector4Int::operator String32() const { return String32(x, U", ", y, U", ", z, U", ", w); }

//Matrix4

Matrix4::Matrix4(const Vector3& position, const Vector3& rotation, const Vector3& scale)
{
	Quaternion3 q{ rotation };
	q.Normalized();

	F32 xx = 2.0f * q.x * q.x;
	F32 xy = 2.0f * q.x * q.y;
	F32 xz = 2.0f * q.x * q.z;
	F32 xw = 2.0f * q.x * q.w;
	F32 yy = 2.0f * q.y * q.y;
	F32 yz = 2.0f * q.y * q.z;
	F32 yw = 2.0f * q.y * q.w;
	F32 zz = 2.0f * q.z * q.z;
	F32 zw = 2.0f * q.z * q.w;

	a.x = (1.0f - yy - zz) * scale.x;
	a.y = xy - zw;
	a.z = xz + yw;
	a.w = 0.0f;

	b.x = xy + zw;
	b.y = (1.0f - xx - zz) * scale.y;
	b.z = yz - xw;
	b.w = 0.0f;

	c.x = xz - yw;
	c.y = yz + xw;
	c.z = (1.0f - xx - yy) * scale.z;
	c.w = 0.0f;

	d.x = position.x;
	d.y = position.y;
	d.z = position.z;
	d.w = 1.0f;
}
Matrix4::Matrix4(const Vector3& position, const Quaternion3& rotation, const Vector3& scale)
{
	Quaternion3 q = rotation.Normalize();

	F32 xx = 2.0f * q.x * q.x;
	F32 xy = 2.0f * q.x * q.y;
	F32 xz = 2.0f * q.x * q.z;
	F32 xw = 2.0f * q.x * q.w;
	F32 yy = 2.0f * q.y * q.y;
	F32 yz = 2.0f * q.y * q.z;
	F32 yw = 2.0f * q.y * q.w;
	F32 zz = 2.0f * q.z * q.z;
	F32 zw = 2.0f * q.z * q.w;

	a.x = (1.0f - yy - zz) * scale.x;
	a.y = xy - zw;
	a.z = xz + yw;
	a.w = 0.0f;

	b.x = xy + zw;
	b.y = (1.0f - xx - zz) * scale.y;
	b.z = yz - xw;
	b.w = 0.0f;

	c.x = xz - yw;
	c.y = yz + xw;
	c.z = (1.0f - xx - yy) * scale.z;
	c.w = 0.0f;

	d.x = position.x;
	d.y = position.y;
	d.z = position.z;
	d.w = 1.0f;
}

void Matrix4::Set(const Vector3& position, const Vector3& rotation, const Vector3& scale)
{
	Quaternion3 q{ rotation };
	q.Normalized();

	F32 xx = 2.0f * q.x * q.x;
	F32 xy = 2.0f * q.x * q.y;
	F32 xz = 2.0f * q.x * q.z;
	F32 xw = 2.0f * q.x * q.w;
	F32 yy = 2.0f * q.y * q.y;
	F32 yz = 2.0f * q.y * q.z;
	F32 yw = 2.0f * q.y * q.w;
	F32 zz = 2.0f * q.z * q.z;
	F32 zw = 2.0f * q.z * q.w;

	a.x = (1.0f - yy - zz) * scale.x;
	a.y = xy - zw;
	a.z = xz + yw;
	a.w = 0.0f;

	b.x = xy + zw;
	b.y = (1.0f - xx - zz) * scale.y;
	b.z = yz - xw;
	b.w = 0.0f;

	c.x = xz - yw;
	c.y = yz + xw;
	c.z = (1.0f - xx - yy) * scale.z;
	c.w = 0.0f;

	d.x = position.x;
	d.y = position.y;
	d.z = position.z;
	d.w = 1.0f;
}

void Matrix4::Set(const Vector3& position, const Quaternion3& rotation, const Vector3& scale)
{
	Quaternion3 q = rotation.Normalize();

	F32 xx = 2.0f * q.x * q.x;
	F32 xy = 2.0f * q.x * q.y;
	F32 xz = 2.0f * q.x * q.z;
	F32 xw = 2.0f * q.x * q.w;
	F32 yy = 2.0f * q.y * q.y;
	F32 yz = 2.0f * q.y * q.z;
	F32 yw = 2.0f * q.y * q.w;
	F32 zz = 2.0f * q.z * q.z;
	F32 zw = 2.0f * q.z * q.w;

	a.x = (1.0f - yy - zz) * scale.x;
	a.y = xy - zw;
	a.z = xz + yw;
	a.w = 0.0f;

	b.x = xy + zw;
	b.y = (1.0f - xx - zz) * scale.y;
	b.z = yz - xw;
	b.w = 0.0f;

	c.x = xz - yw;
	c.y = yz + xw;
	c.z = (1.0f - xx - yy) * scale.z;
	c.w = 0.0f;

	d.x = position.x;
	d.y = position.y;
	d.z = position.z;
	d.w = 1.0f;
}