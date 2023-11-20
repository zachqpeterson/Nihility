#pragma once

template<class A, class B>
struct Pair
{
	using TypeA = A;
	using TypeB = B;

	Pair() {}
	Pair(const A& a, const B& b) : a{ a }, b{ b } {}

private:
	A a{};
	B b{};
};