#pragma once

#include "Defines.hpp"
#include "TypeTraits.hpp"

//template<class T>
//class Function;
//
//template<class Return, typename... Params>
//class Function<Return(Params...)>
//{
//public:
//	template<class FunctionObject>
//	Function(FunctionObject funcObj) : callable(new FunctionImpl<FunctionObject>(Move(funcObj))) {}
//
//	Function(const Function& other) : callable(other.callable) {}
//	Function(Function&& other) : callable(other.callable) { other.callable = nullptr; }
//
//	~Function() { delete callable; }
//
//	Return operator()(Params... params) const
//	{
//		return callable->Call(params...);
//	}
//	
//	//TODO:
//	//bool operator==(const Function& other) const { return 3*other.callable == *callable; }
//
//private:
//	struct FunctionInterface
//	{
//		virtual Return Call(Params...) = 0;
//		virtual ~FunctionInterface() = default;
//
//		//virtual bool operator==(const FunctionInterface& other) = 0;
//	};
//
//	template<class Callable>
//	struct FunctionImpl : FunctionInterface
//	{
//		FunctionImpl(Callable c) : callable(Move(c)) {}
//		//TODO: std::invoke?
//		Return Call(Params... params) final { return callable(params...); }
//
//		Callable callable;
//	};
//
//	FunctionInterface* callable;
//};

template <typename>
struct Function;

template <typename R, typename... Args>
struct Function<R(Args...)> {
    struct ICallable {
        virtual R Invoke(Args&&... args) = 0;
        virtual ICallable* Clone() const = 0;
        virtual ~ICallable() = default;
    };

    template <typename F>
    struct Callable : ICallable {
        F func;

        Callable(F f) : func(Move(f)) {}

        R Invoke(Args&&... args) override
        {
            return func(Forward<Args>(args)...);
        }

        ICallable* Clone() const override
        {
            return new Callable<F>(func);
        }
    };

    ICallable* callable = nullptr;

public:
    Function() = default;

    ~Function() { delete callable; }

    template <typename F>
    Function(F f) : callable(new Callable<F>(Move(f))) {}

    Function(const Function& other) : callable(other.callable ? other.callable->Clone() : nullptr) {}

    Function(Function&& other) noexcept : callable(other.callable)
    {
        other.callable = nullptr;
    }

    Function& operator=(const Function& other)
    {
        if (this != &other)
        {
            delete callable;

            callable = other.callable ? other.callable->Clone() : nullptr;
        }
        return *this;
    }

    Function& operator=(Function&& other) noexcept
    {
        if (this != &other)
        {
            delete callable;
            callable = other.callable;
            other.callable = nullptr;
        }
        return *this;
    }

    R operator()(Args... args) const
    {
        return callable->Invoke(Forward<Args>(args)...);
    }

    explicit operator bool() const
    {
        return callable != nullptr;
    }
};