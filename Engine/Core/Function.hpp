#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

#include <functional>

constexpr inline std::_Ph<1> Placeholder1 = std::placeholders::_1;
constexpr inline std::_Ph<2> Placeholder2 = std::placeholders::_2;
constexpr inline std::_Ph<3> Placeholder3 = std::placeholders::_3;
constexpr inline std::_Ph<4> Placeholder4 = std::placeholders::_4;
constexpr inline std::_Ph<5> Placeholder5 = std::placeholders::_5;
constexpr inline std::_Ph<6> Placeholder6 = std::placeholders::_6;
constexpr inline std::_Ph<7> Placeholder7 = std::placeholders::_7;
constexpr inline std::_Ph<8> Placeholder8 = std::placeholders::_8;
constexpr inline std::_Ph<9> Placeholder9 = std::placeholders::_9;
constexpr inline std::_Ph<10> Placeholder10 = std::placeholders::_10;
constexpr inline std::_Ph<11> Placeholder11 = std::placeholders::_11;
constexpr inline std::_Ph<12> Placeholder12 = std::placeholders::_12;
constexpr inline std::_Ph<13> Placeholder13 = std::placeholders::_13;
constexpr inline std::_Ph<14> Placeholder14 = std::placeholders::_14;
constexpr inline std::_Ph<15> Placeholder15 = std::placeholders::_15;
constexpr inline std::_Ph<16> Placeholder16 = std::placeholders::_16;
constexpr inline std::_Ph<17> Placeholder17 = std::placeholders::_17;
constexpr inline std::_Ph<18> Placeholder18 = std::placeholders::_18;
constexpr inline std::_Ph<19> Placeholder19 = std::placeholders::_19;
constexpr inline std::_Ph<20> Placeholder20 = std::placeholders::_20;

template <class Fn, class... Args>
_NODISCARD constexpr std::_Binder<std::_Unforced, Fn, Args...> Bind(Fn&& func, Args&&... args)
{
	return std::_Binder<std::_Unforced, Fn, Args...>(Forward<Fn>(func), Forward<Args>(args)...);
}

template <class Return, class Fn, class... Args>
_NODISCARD constexpr std::_Binder<Return, Fn, Args...> Bind(Fn&& func, Args&&... args)
{
	return std::_Binder<Return, Fn, Args...>(Forward<Fn>(func), Forward<Args>(args)...);
}

template <class Fn>
using Function = std::function<Fn>;