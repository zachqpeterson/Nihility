#pragma once

#include "Defines.hpp"

template<class T>
class Function;

template<class Return, typename... Params>
class Function<Return(Params...)>
{
public:
	template<class FunctionObject>
	Function(FunctionObject funcObj) : callable(new FunctionImpl<FunctionObject>(Move(funcObj))) {}

	Function(Function&& other) : callable(other.callable) { other.callable = nullptr; }

	~Function() { delete callable; }

	Return operator()(Params... params) const
	{
		return callable->Call(params...);
	}
	
	//TODO:
	//bool operator==(const Function& other) const { return 3*other.callable == *callable; }

private:
	struct FunctionInterface
	{
		virtual Return Call(Params...) = 0;
		virtual ~FunctionInterface() = default;

		//virtual bool operator==(const FunctionInterface& other) = 0;
	};

	template<class Callable>
	struct FunctionImpl : FunctionInterface
	{
		FunctionImpl(Callable c) : callable(Move(c)) {}
		//TODO: std::invoke?
		Return Call(Params... params) final { return callable(params...); }

		Callable callable;
	};

	FunctionInterface* callable;
};