#pragma once

#include "Defines.hpp"

#include <functional>
#include <memory>

#define FUNC_MOVE(value) static_cast<RemovedReference<decltype(value)>&&>(value)
#define FUNC_FORWARD(type, value) static_cast<type &&>(value)

template<typename> class Function;

namespace detail
{
	template<typename Type> struct ForceFunctionHeapAlloc : FalseConstant { };

	struct ManagerStorage;
	struct FunctionManager;
	struct FunctorPadding { private: U64 unused[2]; };
	struct Empty { };

	template<typename Type, typename Allocator>
	struct IsInplaceAllocated
	{
		static constexpr bool value = sizeof(Type) <= sizeof(FunctorPadding) && Alignment<FunctorPadding> % Alignment<Type> == 0
			&& NothrowMoveConstructible<Type> && !ForceFunctionHeapAlloc<Type>::value;
	};

	template<typename Type, typename Allocator> inline constexpr bool InplaceAllocated = IsInplaceAllocated<Type, Allocator>::value;

	template<typename Type> Type ToFunctor(Type&& func) { return FUNC_FORWARD(Type, func); }

	template<typename Result, typename Class, typename... Arguments>
	auto ToFunctor(Result(Class::* func)(Arguments...)) -> decltype(std::mem_fn(func)) { return std::mem_fn(func); }

	template<typename Result, typename Class, typename... Arguments>
	auto ToFunctor(Result(Class::* func)(Arguments...) const) -> decltype(std::mem_fn(func)) { return std::mem_fn(func); }

	template<typename Type> struct FunctorType { typedef decltype(ToFunctor(GetReference<Type>())) type; };

	template<typename Type> bool IsNull(const Type&) { return false; }

	template<typename Result, typename... Arguments>
	bool IsNull(Result(* const& functionPtr)(Arguments...)) { return functionPtr == nullptr; }

	template<typename Result, typename Class, typename... Arguments>
	bool IsNull(Result(Class::* const& functionPtr)(Arguments...)) { return functionPtr == nullptr; }

	template<typename Result, typename Class, typename... Arguments>
	bool IsNull(Result(Class::* const& functionPtr)(Arguments...) const) { return functionPtr == nullptr; }

	template<typename, typename> struct IsValidFunctionArg { static constexpr bool value = false; };

	template<typename Result, typename... Arguments>
	struct IsValidFunctionArg<Function<Result(Arguments...)>, Result(Arguments...)> { static constexpr bool value = false; };

	template<typename Type, typename Result, typename... Arguments>
	struct IsValidFunctionArg<Type, Result(Arguments...)> { static constexpr bool value = true; };

	typedef const FunctionManager* Manager;

	struct ManagerStorage
	{
		template<typename Allocator> Allocator& GetAllocator() { return reinterpret_cast<Allocator&>(manager); }
		template<typename Allocator> const Allocator& GetAllocator() const { return reinterpret_cast<const Allocator&>(manager); }

		FunctorPadding functor;
		Manager manager;
	};

	template<typename T, typename Allocator, typename Enable = void>
	struct FunctionManagerSpecialization
	{
		template<typename Result, typename... Arguments> static Result Call(const FunctorPadding& storage, Arguments... arguments)
		{
			return const_cast<T&>(reinterpret_cast<const T&>(storage))(FUNC_FORWARD(Arguments, arguments)...);
		}

		static void Store(ManagerStorage& storage, T toStore) { new (&FunctorReference(storage)) T(FUNC_FORWARD(T, toStore)); }
		static void Move(ManagerStorage& lhs, ManagerStorage&& rhs) { new (&FunctorReference(lhs)) T(FUNC_MOVE(FunctorReference(rhs))); }
		static void Destroy(Allocator&, ManagerStorage& storage) { FunctorReference(storage).~T(); }
		static T& FunctorReference(const ManagerStorage& storage) { return const_cast<T&>(reinterpret_cast<const T&>(storage.functor)); }
	};
	template<typename T, typename Allocator>
	struct FunctionManagerSpecialization<T, Allocator, Enable<!InplaceAllocated<T, Allocator>>>
	{
		template<typename Result, typename... Arguments>
		static Result Call(const FunctorPadding& storage, Arguments... arguments)
		{
			return (*reinterpret_cast<const typename std::allocator_traits<Allocator>::pointer&>(storage))(FUNC_FORWARD(Arguments, arguments)...);
		}

		static void Store(ManagerStorage& self, T toStore)
		{
			Allocator& allocator = self.GetAllocator<Allocator>();;
			static_assert(sizeof(typename std::allocator_traits<Allocator>::pointer) <= sizeof(self.functor), "The allocator's pointer type is too big");
			typename std::allocator_traits<Allocator>::pointer* ptr = new (&FunctorPtrReference(self)) typename std::allocator_traits<Allocator>::pointer(std::allocator_traits<Allocator>::allocate(allocator, 1));
			std::allocator_traits<Allocator>::construct(allocator, *ptr, FUNC_FORWARD(T, toStore));
		}
		static void Move(ManagerStorage& lhs, ManagerStorage&& rhs)
		{
			static_assert(NothrowMoveConstructible<typename std::allocator_traits<Allocator>::pointer>, "we can't offer a noexcept swap if the pointer type is not nothrow move constructible");
			new (&FunctorPtrReference(lhs)) typename std::allocator_traits<Allocator>::pointer(FUNC_MOVE(FunctorPtrReference(rhs)));
			FunctorPtrReference(rhs) = nullptr;
		}
		static void Destroy(Allocator& allocator, ManagerStorage& storage)
		{
			typename std::allocator_traits<Allocator>::pointer& pointer = FunctorPtrReference(storage);
			if (!pointer) return;
			std::allocator_traits<Allocator>::destroy(allocator, pointer);
			std::allocator_traits<Allocator>::deallocate(allocator, pointer, 1);
		}
		static T& FunctorReference(const ManagerStorage& storage) { return *FunctorPtrReference(storage); }
		static typename std::allocator_traits<Allocator>::pointer& FunctorPtrReference(ManagerStorage& storage)
		{
			return reinterpret_cast<typename std::allocator_traits<Allocator>::pointer&>(storage.functor);
		}
		static const typename std::allocator_traits<Allocator>::pointer& FunctorPtrReference(const ManagerStorage& storage)
		{
			return reinterpret_cast<const typename std::allocator_traits<Allocator>::pointer&>(storage.functor);
		}
	};

	template<typename T, typename Allocator> static const FunctionManager& DefaultManager();

	template<typename T, typename Allocator>
	static void CreateManager(ManagerStorage& storage, Allocator&& allocator)
	{
		new (&storage.GetAllocator<Allocator>()) Allocator(FUNC_MOVE(allocator));
		storage.manager = &DefaultManager<T, Allocator>();
	}

	struct FunctionManager
	{
		template<typename T, typename Allocator>
		inline static const FunctionManager CreateDefaultManager()
		{
			FunctionManager result =
			{
				&TemplatedCallMoveDestroy<T, Allocator>,
				&TemplatedCallCopy<T, Allocator>,
				&TemplatedCallCopyFuncOnly<T, Allocator>,
				&TemplatedCallDestroy<T, Allocator>,
				&TemplatedCallTypeId<T, Allocator>,
				&TemplatedCallTarget<T, Allocator>
			};

			return result;
		}

		void (* const CallMoveDestroy)(ManagerStorage& lhs, ManagerStorage&& rhs);
		void (* const CallCopy)(ManagerStorage& lhs, const ManagerStorage& rhs);
		void (* const CallCopyFuncOnly)(ManagerStorage& lhs, const ManagerStorage& rhs);
		void (* const CallDestroy)(ManagerStorage& manager);
		const std::type_info& (* const CallTypeId)();
		void* (* const CallTarget)(const ManagerStorage& manager, const std::type_info& type);

		template<typename T, typename Allocator>
		static void TemplatedCallMoveDestroy(ManagerStorage& lhs, ManagerStorage&& rhs)
		{
			typedef FunctionManagerSpecialization<T, Allocator> specialization;
			specialization::Move(lhs, FUNC_MOVE(rhs));
			specialization::Destroy(rhs.GetAllocator<Allocator>(), rhs);
			CreateManager<T, Allocator>(lhs, FUNC_MOVE(rhs.GetAllocator<Allocator>()));
			rhs.GetAllocator<Allocator>().~Allocator();
		}
		template<typename T, typename Allocator>
		static void TemplatedCallCopy(ManagerStorage& lhs, const ManagerStorage& rhs)
		{
			typedef FunctionManagerSpecialization<T, Allocator> specialization;
			CreateManager<T, Allocator>(lhs, Allocator(rhs.GetAllocator<Allocator>()));
			specialization::Store(lhs, specialization::FunctorReference(rhs));
		}
		template<typename T, typename Allocator>
		static void TemplatedCallDestroy(ManagerStorage& self)
		{
			typedef FunctionManagerSpecialization<T, Allocator> specialization;
			specialization::Destroy(self.GetAllocator<Allocator>(), self);
			self.GetAllocator<Allocator>().~Allocator();
		}
		template<typename T, typename Allocator>
		static void TemplatedCallCopyFuncOnly(ManagerStorage& lhs, const ManagerStorage& rhs)
		{
			typedef FunctionManagerSpecialization<T, Allocator> specialization;
			specialization::Store(lhs, specialization::FunctorReference(rhs));
		}
		template<typename T, typename>
		static const std::type_info& TemplatedCallTypeId()
		{
			return typeid(T);
		}
		template<typename T, typename Allocator>
		static void* TemplatedCallTarget(const ManagerStorage& self, const std::type_info& type)
		{
			typedef FunctionManagerSpecialization<T, Allocator> specialization;
			if (type == typeid(T)) { return &specialization::FunctorReference(self); }
			else { return nullptr; }
		}
	};
	template<typename T, typename Allocator>
	inline static const FunctionManager& DefaultManager()
	{
		static const FunctionManager defaultManager = FunctionManager::CreateDefaultManager<T, Allocator>();
		return defaultManager;
	}

	template<typename Result, typename...>
	struct typedeffer
	{
		typedef Result result;
	};
	template<typename Result, typename Argument>
	struct typedeffer<Result, Argument>
	{
		typedef Result result;
		typedef Argument arg;
	};
	template<typename Result, typename FirstArg, typename SecondArg>
	struct typedeffer<Result, FirstArg, SecondArg>
	{
		typedef Result result;
		typedef FirstArg firstArg;
		typedef SecondArg secondArg;
	};
}

template<typename Result, typename... Arguments>
class Function<Result(Arguments...)> : public detail::typedeffer<Result, Arguments...>
{
public:
	Function() { InitializeEmpty(); }
	Function(NullPointer) { InitializeEmpty(); }
	Function(Function&& other) { InitializeEmpty(); Swap(other); }
	Function(const Function& other) : Call(other.Call) { other.managerStorage.manager->CallCopy(managerStorage, other.managerStorage); }
	template<typename Type> Function(Type functor, typename Enable<detail::IsValidFunctionArg<Type, Result(Arguments...)>::value, detail::Empty> = detail::Empty())
	{
		if (detail::IsNull(functor)) { InitializeEmpty(); }
		else
		{
			typedef typename detail::FunctorType<Type>::type FunctorType;
			Initialize(detail::ToFunctor(FUNC_FORWARD(Type, functor)), std::allocator<FunctorType>());
		}
	}
	template<typename Allocator> Function(std::allocator_arg_t, const Allocator&) { InitializeEmpty(); }
	template<typename Allocator> Function(std::allocator_arg_t, const Allocator&, NullPointer) { InitializeEmpty(); }
	template<typename Allocator, typename T>
	Function(std::allocator_arg_t, const Allocator& allocator, T functor,
		typename Enable<detail::IsValidFunctionArg<T, Result(Arguments...)>::value, detail::Empty> = detail::Empty())
	{
		if (detail::IsNull(functor)) { InitializeEmpty(); }
		else { Initialize(detail::ToFunctor(FUNC_FORWARD(T, functor)), Allocator(allocator)); }
	}
	template<typename Allocator>
	Function(std::allocator_arg_t, const Allocator& allocator, const Function& other) : Call(other.Call)
	{
		typedef typename std::allocator_traits<Allocator>::template rebind_alloc<Function> MyAllocator;

		detail::Manager managerForAllocator = &detail::DefaultManager<typename std::allocator_traits<Allocator>::value_type, Allocator>();
		if (other.managerStorage.manager == managerForAllocator)
		{
			detail::CreateManager<typename std::allocator_traits<Allocator>::value_type, Allocator>(managerStorage, Allocator(allocator));
			managerForAllocator->CallCopyFuncOnly(managerStorage, other.managerStorage);
		}
		else
		{
			detail::Manager managerForFunction = &detail::DefaultManager<Function, MyAllocator>();
			if (other.managerStorage.manager == managerForFunction)
			{
				detail::CreateManager<Function, MyAllocator>(managerStorage, MyAllocator(allocator));
				managerForFunction->CallCopyFuncOnly(managerStorage, other.managerStorage);
			}
			else { Initialize(other, MyAllocator(allocator)); }
		}
	}

	template<typename Allocator> Function(std::allocator_arg_t, const Allocator&, Function&& other) { InitializeEmpty(); Swap(other); }

	Function& operator=(Function other) { Swap(other); return *this; }

	~Function() { managerStorage.manager->CallDestroy(managerStorage); }

	Result operator()(Arguments... arguments) const { return Call(managerStorage.functor, FUNC_FORWARD(Arguments, arguments)...); }

	template<typename T, typename Allocator> void assign(T&& functor, const Allocator& allocator) { Function(std::allocator_arg, allocator, functor).Swap(*this); }

	void Swap(Function& other)
	{
		detail::ManagerStorage tempStore;
		other.managerStorage.manager->CallMoveDestroy(tempStore, FUNC_MOVE(other.managerStorage));
		managerStorage.manager->CallMoveDestroy(other.managerStorage, FUNC_MOVE(managerStorage));
		tempStore.manager->CallMoveDestroy(managerStorage, FUNC_MOVE(tempStore));

		std::swap(Call, other.Call);
	}

	const std::type_info& target_type() const { return managerStorage.manager->CallTypeId(); }

	template<typename T> T* target() { return static_cast<T*>(managerStorage.manager->CallTarget(managerStorage, typeid(T))); }
	template<typename T> const T* target() const { return static_cast<const T*>(managerStorage.manager->CallTarget(managerStorage, typeid(T))); }

	operator bool() const { return Call != nullptr; }

private:
	detail::ManagerStorage managerStorage;
	Result(*Call)(const detail::FunctorPadding&, Arguments...);

	template<typename T, typename Allocator>
	void Initialize(T functor, Allocator&& allocator)
	{
		Call = &detail::FunctionManagerSpecialization<T, Allocator>::template Call<Result, Arguments...>;
		detail::CreateManager<T, Allocator>(managerStorage, FUNC_FORWARD(Allocator, allocator));
		detail::FunctionManagerSpecialization<T, Allocator>::Store(managerStorage, FUNC_FORWARD(T, functor));
	}

	typedef Result(*Empty_Function_Type)(Arguments...);
	void InitializeEmpty()
	{
		typedef std::allocator<Empty_Function_Type> Allocator;
		static_assert(detail::InplaceAllocated<Empty_Function_Type, Allocator>, "The empty function should benefit from small functor optimization");

		detail::CreateManager<Empty_Function_Type, Allocator>(managerStorage, Allocator());
		detail::FunctionManagerSpecialization<Empty_Function_Type, Allocator>::Store(managerStorage, nullptr);
		Call = nullptr;
	}
};

template<typename T> bool operator==(NullPointer, const Function<T>& rhs) { return !rhs; }
template<typename T> bool operator==(const Function<T>& lhs, NullPointer) { return !lhs; }
template<typename T> bool operator!=(NullPointer, const Function<T>& rhs) { return rhs; }
template<typename T> bool operator!=(const Function<T>& lhs, NullPointer) { return lhs; }
template<typename T> void Swap(Function<T>& lhs, Function<T>& rhs) { lhs.Swap(rhs); }

namespace std
{
	template<typename Result, typename... Arguments, typename Allocator>
	struct uses_allocator<Function<Result(Arguments...)>, Allocator> : TrueConstant {};
}