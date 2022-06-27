#include "Math.hpp"

#include "Containers/String.hpp"
#include "Core/Logger.hpp"

#include <math.h>

//TODO: Temp
#include <string>

static U8 simplexPerm[512] = {
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

static I8 simplexGrad2[12][2] = {
        {1, 1},{-1, 1},{ 1,-1},{-1,-1},
        {1, 0},{-1, 0},{ 1, 0},{-1, 0},
        {0, 1},{ 0,-1},{ 0, 1},{ 0,-1}
};

static I8 simplexGrad3[12][3] = {
        {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
        {1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
        {0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1}
};

const Vector2 Vector2::ONE = { 1.0f,  1.0f };
const Vector2 Vector2::ZERO = { 0.0f,  0.0f };
const Vector2 Vector2::RIGHT = { 1.0f,  0.0f };
const Vector2 Vector2::LEFT = { -1.0f,  0.0f };
const Vector2 Vector2::UP = { 0.0f,  1.0f };
const Vector2 Vector2::DOWN = { 0.0f, -1.0f };

const Vector3 Vector3::ONE = { 1.0f,  1.0f,  1.0f };
const Vector3 Vector3::ZERO = { 0.0f,  0.0f,  0.0f };
const Vector3 Vector3::RIGHT = { 1.0f,  0.0f,  0.0f };
const Vector3 Vector3::LEFT = { -1.0f,  0.0f,  0.0f };
const Vector3 Vector3::UP = { 0.0f,  1.0f,  0.0f };
const Vector3 Vector3::DOWN = { 0.0f, -1.0f,  0.0f };
const Vector3 Vector3::FORWARD = { 0.0f,  0.0f,  1.0f };
const Vector3 Vector3::BACK = { 0.0f,  0.0f, -1.0f };

const Vector4 Vector4::ONE = { 1.0f,  1.0f,  1.0f,  1.0f };
const Vector4 Vector4::ZERO = { 0.0f,  0.0f,  0.0f,  0.0f };
const Vector4 Vector4::RIGHT = { 1.0f,  0.0f,  0.0f,  0.0f };
const Vector4 Vector4::LEFT = { -1.0f,  0.0f,  0.0f,  0.0f };
const Vector4 Vector4::UP = { 0.0f,  1.0f,  0.0f,  0.0f };
const Vector4 Vector4::DOWN = { 0.0f, -1.0f,  0.0f,  0.0f };
const Vector4 Vector4::FORWARD = { 0.0f,  0.0f,  1.0f,  0.0f };
const Vector4 Vector4::BACK = { 0.0f,  0.0f, -1.0f,  0.0f };
const Vector4 Vector4::OUTWARD = { 0.0f,  0.0f,  0.0f,  1.0f };
const Vector4 Vector4::INWARD = { 0.0f,  0.0f,  0.0f, -1.0f };

const Vector2Int Vector2Int::ONE = { 1, 1 };
const Vector2Int Vector2Int::ZERO = { 0, 0 };
const Vector2Int Vector2Int::RIGHT = { 1, 0 };
const Vector2Int Vector2Int::LEFT = { -1, 0 };
const Vector2Int Vector2Int::UP = { 0, 1 };
const Vector2Int Vector2Int::DOWN = { 0, -1 };

const Vector3Int Vector3Int::ONE = { 1,  1,  1 };
const Vector3Int Vector3Int::ZERO = { 0,  0,  0 };
const Vector3Int Vector3Int::RIGHT = { 1,  0,  0 };
const Vector3Int Vector3Int::LEFT = { -1,  0,  0 };
const Vector3Int Vector3Int::UP = { 0,  1,  0 };
const Vector3Int Vector3Int::DOWN = { 0, -1,  0 };
const Vector3Int Vector3Int::FORWARD = { 0,  0,  1 };
const Vector3Int Vector3Int::BACK = { 0,  0, -1 };

const Vector4Int Vector4Int::ONE = { 1,  1,  1,  1 };
const Vector4Int Vector4Int::ZERO = { 0,  0,  0,  0 };
const Vector4Int Vector4Int::RIGHT = { 1,  0,  0,  0 };
const Vector4Int Vector4Int::LEFT = { -1,  0,  0,  0 };
const Vector4Int Vector4Int::UP = { 0,  1,  0,  0 };
const Vector4Int Vector4Int::DOWN = { 0, -1,  0,  0 };
const Vector4Int Vector4Int::FORWARD = { 0,  0,  1,  0 };
const Vector4Int Vector4Int::BACK = { 0,  0, -1,  0 };
const Vector4Int Vector4Int::OUTWARD = { 0,  0,  0,  1 };
const Vector4Int Vector4Int::INWARD = { 0,  0,  0, -1 };

const Matrix2 Matrix2::IDENTITY = { { 1.f, 0.f }, { 0.f, 1.f } };
const Matrix3 Matrix3::IDENTITY = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f }, { 0.f, 0.f, 1.f } };
const Matrix4 Matrix4::IDENTITY = { { 1.f, 0.f, 0.f, 0.f }, { 0.f, 1.f, 0.f, 0.f }, { 0.f, 0.f, 1.f, 0.f }, { 0.f, 0.f, 0.f, 1.f } };

const Quaternion Quaternion::IDENTITY = { 0.0f,  0.0f,  0.0f,  1.0f };

//TRIGONOMETRY
F32 Math::Sin(F32 f)
{
    return sinf(f);
    //return 4.0f * (0.31830988618f * f * (1.0f - Abs(0.31830988618f * f)));
}

F64 Math::Sin(F64 f)
{
    return sin(f);
    //return 4.0 * (0.31830988618 * f * (1.0 - Abs(0.31830988618 * f)));
}

F32 Math::Cos(F32 f)
{
    return cosf(f);
    //return 4.0f * (0.5f - 0.31830988618f * f) * (1.0f - Abs(0.5f - 0.31830988618f * f));
}

F64 Math::Cos(F64 f)
{
    return cos(f);
    //return 4.0 * (0.5 - 0.31830988618 * f) * (1.0 - Abs(0.5 - 0.31830988618 * f));
}

F32 Math::Tan(F32 f) { return tanf(f); }
F64 Math::Tan(F64 f) { return tan(f); }
F32 Math::Asin(F32 f) { return asinf(f); }
F64 Math::Asin(F64 f) { return asin(f); }
F32 Math::Acos(F32 f) { return acosf(f); }
F64 Math::Acos(F64 f) { return acos(f); }
F32 Math::Atan(F32 f) { return atanf(f); }
F64 Math::Atan(F64 f) { return atan(f); }

F32 Math::Log2(F32 f) { return log2f(f); }
F64 Math::Log2(F64 f) { return log2(f); }
F32 Math::Log10(F32 f) { return log10f(f); }
F64 Math::Log10(F64 f) { return log10(f); }
F32 Math::LogN(F32 f) { return logf(f); }
F64 Math::LogN(F64 f) { return log(f); }

//LINEAR
F32 Math::Sqrt(F32 f)
{
    constexpr U32 add = 127 << 23;

    union
    {
        F32 f;
        U32 i;
    } conv = { .f = f };

    conv.i += add;
    conv.i >>= 1;

    return conv.f;
}

F64 Math::Sqrt(F64 f)
{
    return Sqrt((F32)f);
}

F32 Math::InvSqrt(F32 f)
{
    union
    {
        F32 f;
        U32 i;
    } conv = { .f = f };

    conv.i = 0x5f3759df - (conv.i >> 1);
    conv.f *= 1.5F - (f * 0.5F * conv.f * conv.f);
    return conv.f;
}

F64 Math::InvSqrt(F64 f)
{
    return InvSqrt((F32)f);
}

Matrix4&& Math::Orthographic(F32 left, F32 right, F32 bottom, F32 top, F32 near, F32 far)
{
    Matrix4 result = Matrix4::IDENTITY;
    result[0][0] = 2.0f / (right - left);
    result[1][1] = 2.0f / (top - bottom);
    result[2][2] = -2.0f / (far - near);
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);
    result[3][2] = -(far + near) / (far - near);
    return Move(result);
}

Matrix4&& Math::Perspective(F32 fov, F32 aspect, F32 near, F32 far)
{
    F32 scale = 1.0f / Math::Tan(fov * 0.5f);

    Matrix4 result = Matrix4::IDENTITY;
    result[0][0] = scale; //TODO: Aspect?
    result[1][1] = scale;
    result[2][2] = -far / (far - near);
    result[3][2] = -far * near / (far - near);
    result[2][3] = -1.0f;
    result[3][3] = 0.0f;
    return Move(result);
}

//NOISE
F64 Math::Simplex1(F64 x)
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

F64 Math::Simplex2(F64 x, F64 y)
{
    F64 n0, n1, n2;

    constexpr F64 F2 = 0.36602540378;
    F64 g = (x + y) * F2;
    I64 i = Floor(x + g);
    I64 j = Floor(y + g);

    constexpr F64 G2 = 0.2113248654;
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
        n0 = t0 * t0 * Dot(simplexGrad2[gi0], x0, y0);
    }

    F64 t1 = 0.5 - x1 * x1 - y1 * y1;

    if (t1 < 0) { n1 = 0.0; }
    else
    {
        t1 *= t1;
        n1 = t1 * t1 * Dot(simplexGrad2[gi1], x1, y1);
    }

    F64 t2 = 0.5 - x2 * x2 - y2 * y2;

    if (t2 < 0) { n2 = 0.0; }
    else
    {
        t2 *= t2;
        n2 = t2 * t2 * Dot(simplexGrad2[gi2], x2, y2);
    }

    return 70.0 * (n0 + n1 + n2);
}

F64 Math::Simplex3(F64 x, F64 y, F64 z)
{
    F64 n0, n1, n2, n3;

    constexpr F64 F3 = 1.0 / 3.0;
    F64 s = (x + y + z) * F3;
    I64 i = Floor(x + s);
    I64 j = Floor(y + s);
    I64 k = Floor(z + s);
    constexpr F64 G3 = 1.0 / 6.0;
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
        n0 = t0 * t0 * Dot(simplexGrad3[gi0], x0, y0, z0);
    }
    F64 t1 = 0.5 - x1 * x1 - y1 * y1 - z1 * z1;
    if (t1 < 0) { n1 = 0.0; }
    else
    {
        t1 *= t1;
        n1 = t1 * t1 * Dot(simplexGrad3[gi1], x1, y1, z1);
    }
    F64 t2 = 0.5 - x2 * x2 - y2 * y2 - z2 * z2;
    if (t2 < 0) { n2 = 0.0; }
    else
    {
        t2 *= t2;
        n2 = t2 * t2 * Dot(simplexGrad3[gi2], x2, y2, z2);
    }
    F64 t3 = 0.5 - x3 * x3 - y3 * y3 - z3 * z3;
    if (t3 < 0) { n3 = 0.0; }
    else
    {
        t3 *= t3;
        n3 = t3 * t3 * Dot(simplexGrad3[gi3], x3, y3, z3);
    }

    return 32.0 * (n0 + n1 + n2 + n3);
}

F64 Math::Dot(I8 g[2], F64 x, F64 y)
{
    return g[0] * x + g[1] * y;
}

F64 Math::Dot(I8 g[3], F64 x, F64 y, F64 z)
{
    return g[0] * x + g[1] * y + g[2] * z;
}

F64 Math::Grad(I64 hash, F64 x)
{
    I64 r = hash & 15;
    return ((1.0 + (r & 7)) * x) * ((r & 8) ? -1 : 1);
}

//RANDOM
I64 Math::Random(U32 seed)
{
    srand(seed);
    return rand();
}

I64 Math::RandomRange(I64 min, I64 max, U32 seed)
{
    srand(seed);
    return (rand() % (max - min + 1)) + min;
}

F32 Math::RandomF(U32 seed)
{
    return (F32)Random(seed) / (F32)RAND_MAX;
}

F32 Math::RandomRangeF(F32 min, F32 max, U32 seed)
{
    return min + ((F32)Random(seed) / ((F32)RAND_MAX) / (max - min));
}

//HASHING
U64 Math::Hash(const String& str, U64 max)
{
    const char* ptr = (const char*)str;
    U64 hash = 0;
    while (*ptr)
    {
        hash = hash * 101 + *ptr;
        ++ptr;
    }
    return hash % max;
}

U64 Math::Hash(U64 value, U64 max)
{
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ull;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebull;
    value = value ^ (value >> 31);
    return value % max;
}

//VECTOR2
Vector2::Vector2(const String& str)
{
    sscanf(str, "%f,%f", &x, &y);
}

Vector2& Vector2::operator=(const String& str)
{
    sscanf(str, "%f,%f", &x, &y);

    return *this;
}

//VECTOR3
Vector3::Vector3(const String& str)
{
    sscanf(str, "%f,%f,%f", &x, &y, &z);
}

Vector3& Vector3::operator=(const String& str)
{
    sscanf(str, "%f,%f,%f", &x, &y, &z);

    return *this;
}

//VECTOR4
Vector4::Vector4(const String& str)
{
    sscanf(str, "%f,%f,%f,%f", &x, &y, &z, &w);
}

Vector4& Vector4::operator=(const String& str)
{
    sscanf(str, "%f,%f,%f,%f", &x, &y, &z, &w);

    return *this;
}

//VECTOR2INT
Vector2Int::Vector2Int(const String& str)
{
    sscanf(str, "%d,%d", &x, &y);
}

Vector2Int& Vector2Int::operator=(const String& str)
{
    sscanf(str, "%d,%d", &x, &y);

    return *this;
}

//VECTOR3INT
Vector3Int::Vector3Int(const String& str)
{
    sscanf(str, "%d,%d,%d", &x, &y, &z);
}

Vector3Int& Vector3Int::operator=(const String& str)
{
    sscanf(str, "%d,%d,%d", &x, &y, &z);

    return *this;
}

//VECTOR4INT
Vector4Int::Vector4Int(const String& str)
{
    sscanf(str, "%d,%d,%d,%d", &x, &y, &z, &w);
}

Vector4Int& Vector4Int::operator=(const String& str)
{
    sscanf(str, "%d,%d,%d,%d", &x, &y, &z, &w);

    return *this;
}

//QUATERNION
Quaternion Quaternion::AxisAngle(const Vector3& axis, F32 angle, bool normalize)
{
    const F32 halfAngle = 0.5f * angle;
    F32 s = Math::Sin(halfAngle);
    F32 c = Math::Cos(halfAngle);

    Quaternion q = { s * axis.x, s * axis.y, s * axis.z, c };
    if (normalize) { return q.Normalized(); }
    return q;
}

Quaternion Quaternion::AxisAngle(const Vector2& axis, F32 angle, bool normalize)
{
    const F32 halfAngle = 0.5f * angle;
    F32 s = Math::Sin(halfAngle);
    F32 c = Math::Cos(halfAngle);

    Quaternion q = { s * axis.x, s * axis.y, 0.0f, c };
    if (normalize) { return q.Normalized(); }
    return q;
}

Matrix4 Quaternion::ToMatrix4() const
{
    Matrix4 matrix = Matrix4::IDENTITY;
    Quaternion q = Normalized();

    matrix[0][0] = 1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z;
    matrix[1][0] = 2.0f * q.x * q.y - 2.0f * q.z * q.w;
    matrix[2][0] = 2.0f * q.x * q.z + 2.0f * q.y * q.w;

    matrix[0][1] = 2.0f * q.x * q.y + 2.0f * q.z * q.w;
    matrix[1][1] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
    matrix[2][1] = 2.0f * q.y * q.z - 2.0f * q.x * q.w;

    matrix[0][2] = 2.0f * q.x * q.z - 2.0f * q.y * q.w;
    matrix[1][2] = 2.0f * q.y * q.z + 2.0f * q.x * q.w;
    matrix[2][2] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;

    return matrix;
}

Matrix3 Quaternion::ToMatrix3() const
{
    Matrix3 matrix = Matrix3::IDENTITY;
    Quaternion q = Normalized();

    matrix[0][0] = 1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z;
    matrix[1][0] = 2.0f * q.x * q.y - 2.0f * q.z * q.w;
    matrix[2][0] = 2.0f * q.x * q.z + 2.0f * q.y * q.w;

    matrix[0][1] = 2.0f * q.x * q.y + 2.0f * q.z * q.w;
    matrix[1][1] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z;
    matrix[2][1] = 2.0f * q.y * q.z - 2.0f * q.x * q.w;

    matrix[0][2] = 2.0f * q.x * q.z - 2.0f * q.y * q.w;
    matrix[1][2] = 2.0f * q.y * q.z + 2.0f * q.x * q.w;
    matrix[2][2] = 1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y;

    return matrix;
}

Matrix4 Quaternion::RotationMatrix(Vector3 center) const
{
    Matrix4 matrix = Matrix4::IDENTITY;

    matrix[0][0] = (x * x) - (y * y) - (z * z) + (w * w);
    matrix[1][0] = 2.0f * ((x * y) + (z * w));
    matrix[2][0] = 2.0f * ((x * z) - (y * w));
    matrix[3][0] = center.x - center.x * matrix[0][0] - center.y * matrix[1][0] - center.z * matrix[2][0];

    matrix[0][1] = 2.0f * ((x * y) - (z * w));
    matrix[1][1] = -(x * x) + (y * y) - (z * z) + (w * w);
    matrix[2][1] = 2.0f * ((y * z) + (x * w));
    matrix[3][1] = center.y - center.x * matrix[0][1] - center.y * matrix[1][1] - center.z * matrix[2][1];

    matrix[0][2] = 2.0f * ((x * z) + (y * w));
    matrix[1][2] = 2.0f * ((y * z) - (x * w));
    matrix[2][2] = -(x * x) - (y * y) + (z * z) + (w * w);
    matrix[3][2] = center.z - center.x * matrix[0][2] - center.y * matrix[1][2] - center.z * matrix[2][2];

    return matrix;
}

Quaternion Quaternion::Slerp(const Quaternion& q, F32 t) const
{
    Quaternion out;

    Quaternion v0 = Normalized();
    Quaternion v1 = q.Normalized();

    F32 dot = v0.Dot(v1);

    if (dot < 0.0f)
    {
        v1.x = -v1.x;
        v1.y = -v1.y;
        v1.z = -v1.z;
        v1.w = -v1.w;
        dot = -dot;
    }

    const F32 DOT_THRESHOLD = 0.9995f;
    if (dot > DOT_THRESHOLD)
    {
        out = {
            v0.x + ((v1.x - v0.x) * t),
            v0.y + ((v1.y - v0.y) * t),
            v0.z + ((v1.z - v0.z) * t),
            v0.w + ((v1.w - v0.w) * t) };

        return out.Normalized();
    }

    F32 theta0 = Math::Acos(dot);
    F32 theta = theta0 * t;
    F32 sinTheta = Math::Sin(theta);
    F32 sinTheta0 = Math::Sin(theta0);

    F32 s0 = Math::Cos(theta) - dot * sinTheta / sinTheta0;
    F32 s1 = sinTheta / sinTheta0;

    return Quaternion{
        (v0.x * s0) + (v1.x * s1),
        (v0.y * s0) + (v1.y * s1),
        (v0.z * s0) + (v1.z * s1),
        (v0.w * s0) + (v1.w * s1)
        };
}