module;

export module Containers:Pair;

export template<class A, class B>
struct Pair
{
	using TypeA = A;
	using TypeB = B;

	constexpr Pair() {}
	constexpr Pair(const A& a, const B& b) : a(a), b(b) {}

private:
	A a;
	B b;
};