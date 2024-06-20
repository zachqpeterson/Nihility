import Containers;

#include "Core\Logger.hpp"
#include "Core\Time.hpp"
#include "Math\Math.hpp"

#define BEGIN_TEST Timer timer; timer.Start()
#define END_TEST(b) timer.Stop(); if(b) {Logger::Info("{}	{}", __FUNCTION__, timer.CurrentTime());} else {Logger::Error("{}	{}", __FUNCTION__, timer.CurrentTime());}

#pragma region Vector Tests

struct SimpleData
{
	I32 i{ 10 };
	F32 f{ 3.14f };
	bool b{ true };

	bool operator==(const SimpleData& other) const { return i == other.i && f == other.f && b == other.b; }
};

struct ComplexData
{
	ComplexData() : data{ new SimpleData() } {}
	~ComplexData() { delete data; }

	bool operator==(const ComplexData& other) const { return data == other.data; }

	SimpleData* data;
};

struct NoDefaultData
{
	NoDefaultData(I32 i) : i{ i } {}

	bool operator==(const NoDefaultData& other) const { return i == other.i; }

	I32 i;
};

void Vector_ConstructorBlank()
{
	BEGIN_TEST;

	Vector<I32> v;

	bool passed = v.Size() == 0 && v.Capacity() == 0 && v.Data() == nullptr;

	END_TEST(passed)
}

void Vector_ConstructorCapacity()
{
	BEGIN_TEST;

	Vector<I32> v(10);

	bool passed = v.Size() == 0 && v.Capacity() >= 10 && v.Data() != nullptr;

	END_TEST(passed)
}

void Vector_ConstructorSizePrimitive()
{
	BEGIN_TEST;

	Vector<I32> v(10, 45);

	bool passed = v.Size() == 10 && v.Capacity() >= 10 && v.Data() != nullptr;

	for (I32 i : v) { passed &= i == 45; }

	END_TEST(passed)
}

void Vector_ConstructorSizeSimple()
{
	BEGIN_TEST;

	SimpleData sample{};

	Vector<SimpleData> v(10, sample);

	bool passed = v.Size() == 10 && v.Capacity() >= 10 && v.Data() != nullptr;

	for (SimpleData& i : v) { passed &= i == sample; }

	END_TEST(passed)
}

void Vector_ConstructorSizeComplex()
{
	BEGIN_TEST;

	ComplexData sample{};

	Vector<ComplexData> v(10, sample);

	bool passed = v.Size() == 10 && v.Capacity() >= 10 && v.Data() != nullptr;

	for (ComplexData& i : v) { passed &= i == sample; }

	END_TEST(passed)
}

void Vector_ConstructorInitializer()
{
	BEGIN_TEST;

	Vector<I32> v({ 10, 12, 6, 12, 67 });

	bool passed = v.Size() == 5 && v.Capacity() >= 5 && v.Data() != nullptr &&
		v[0] == 10 && v[1] == 12 && v[2] == 6 && v[3] == 12 && v[4] == 67;

	END_TEST(passed)
}



void Vector_Push1000000()
{
	BEGIN_TEST;

	Vector<I32> v;

	for (U32 i = 0; i < 1000000; ++i) { v.Push(i); }

	bool passed = v.Size() == 1000000 && v.Capacity() >= 1000000 && v.Data() != nullptr;

	END_TEST(passed)
}
#pragma endregion

int main()
{
	Vector2 v;
	Vector2 v1 = Vector2Zero;

	Vector_ConstructorBlank();
	Vector_ConstructorCapacity();
	Vector_ConstructorSizePrimitive();
	Vector_ConstructorSizeSimple();
	Vector_ConstructorSizeComplex();
	Vector_ConstructorInitializer();

	Vector_Push1000000();

	BreakPoint;
}

