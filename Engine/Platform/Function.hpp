#pragma once

#include "Defines.hpp"

#include <functional>
#include <memory>

#define FUNC_MOVE(value) static_cast<RemovedReference<decltype(value)>&&>(value)
#define FUNC_FORWARD(type, value) static_cast<type &&>(value)

template<typename> struct Function;

namespace detail
{
	struct FunctorPadding { private: U64 unused[2]; };
	struct Empty {};

	template<typename Type>
	struct IsInplaceAllocated
	{
		static constexpr bool value = sizeof(Type) <= sizeof(FunctorPadding) && 
			Alignment<FunctorPadding> % Alignment<Type> == 0 && NothrowMoveConstructible<Type>;
	};

	template<typename Type> inline constexpr bool InplaceAllocated = IsInplaceAllocated<Type>::value;

	template<typename Type> Type ToFunctor(Type&& func) { return FUNC_FORWARD(Type, func); }

	template<typename Result, typename Class, typename... Arguments>
	auto ToFunctor(Result(Class::* func)(Arguments...)) -> decltype(std::mem_fn(func)) { return std::mem_fn(func); }

	template<typename Result, typename Class, typename... Arguments>
	auto ToFunctor(Result(Class::* func)(Arguments...) const) -> decltype(std::mem_fn(func)) { return std::mem_fn(func); }

	template<typename Type> struct FunctorType { using type = decltype(ToFunctor(DeclValue<Type>())); };

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

	struct FunctionManager;

	struct ManagerStorage
	{
		template<typename Allocator> Allocator& GetAllocator() { return reinterpret_cast<Allocator&>(manager); }
		template<typename Allocator> const Allocator& GetAllocator() const { return reinterpret_cast<const Allocator&>(manager); }

		FunctorPadding functor;
		const FunctionManager* manager;
	};

	template<typename Type, typename Allocator, typename Enable = void>
	struct FunctionManagerSpecialization
	{
		template<typename Result, typename... Arguments> 
		static Result Call(const FunctorPadding& storage, Arguments... arguments)
		{
			return const_cast<Type&>(reinterpret_cast<const Type&>(storage)) (FUNC_FORWARD(Arguments, arguments)...);
		}

		static void Store(ManagerStorage& storage, Type toStore) { new (&FunctorReference(storage)) Type(FUNC_FORWARD(Type, toStore)); }
		static void Move(ManagerStorage& lhs, ManagerStorage&& rhs) { new (&FunctorReference(lhs)) Type(FUNC_MOVE(FunctorReference(rhs))); }
		static void Destroy(Allocator&, ManagerStorage& storage) { FunctorReference(storage).~Type(); }
		static Type& FunctorReference(const ManagerStorage& storage) { return const_cast<Type&>(reinterpret_cast<const Type&>(storage.functor)); }
	};

	template<typename Type, typename Allocator>
	struct FunctionManagerSpecialization<Type, Allocator, typename std::enable_if<!InplaceAllocated<Type>>::type>
	{
		template<typename Result, typename... Arguments>
		static Result Call(const FunctorPadding& storage, Arguments... arguments)
		{
			return (*reinterpret_cast<const typename std::allocator_traits<Allocator>::pointer&>(storage))(FUNC_FORWARD(Arguments, arguments)...);
		}

		static void Store(ManagerStorage& self, Type toStore)
		{
			Allocator& allocator = self.GetAllocator<Allocator>();
			static_assert(sizeof(typename std::allocator_traits<Allocator>::pointer) <= sizeof(self.functor), "The allocator's pointer type is too big");
			typename std::allocator_traits<Allocator>::pointer* ptr = new (&FunctorPtrReference(self)) typename std::allocator_traits<Allocator>::pointer(std::allocator_traits<Allocator>::allocate(allocator, 1));
			std::allocator_traits<Allocator>::construct(allocator, *ptr, FUNC_FORWARD(Type, toStore));
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
			if (!pointer) { return; }
			std::allocator_traits<Allocator>::destroy(allocator, pointer);
			std::allocator_traits<Allocator>::deallocate(allocator, pointer, 1);
		}
		static Type& FunctorReference(const ManagerStorage& storage)
		{
			return *FunctorPtrReference(storage);
		}
		static typename std::allocator_traits<Allocator>::pointer& FunctorPtrReference(ManagerStorage& storage)
		{
			return reinterpret_cast<typename std::allocator_traits<Allocator>::pointer&>(storage.functor);
		}
		static const typename std::allocator_traits<Allocator>::pointer& FunctorPtrReference(const ManagerStorage& storage)
		{
			return reinterpret_cast<const typename std::allocator_traits<Allocator>::pointer&>(storage.functor);
		}
	};

	template<typename Type, typename Allocator> static const FunctionManager& DefaultManager();

	template<typename Type, typename Allocator>
	static void CreateManager(ManagerStorage& storage, Allocator&& allocator)
	{
		new (&storage.GetAllocator<Allocator>()) Allocator(FUNC_MOVE(allocator));
		storage.manager = &DefaultManager<Type, Allocator>();
	}

	struct FunctionManager
	{
		template<typename Type, typename Allocator>
		inline static const FunctionManager CreateDefaultManager()
		{
			FunctionManager result =
			{
				&TemplatedCallMoveDestroy<Type, Allocator>,
				&TemplatedCallCopy<Type, Allocator>,
				&TemplatedCallCopyFuncOnly<Type, Allocator>,
				&TemplatedCallDestroy<Type, Allocator>,
				&TemplatedCallTypeId<Type, Allocator>,
				&TemplatedCallTarget<Type, Allocator>
			};

			return result;
		}

		void (* const CallMoveDestroy)(ManagerStorage& lhs, ManagerStorage&& rhs);
		void (* const CallCopy)(ManagerStorage& lhs, const ManagerStorage& rhs);
		void (* const CallCopyFuncOnly)(ManagerStorage& lhs, const ManagerStorage& rhs);
		void (* const CallDestroy)(ManagerStorage& manager);
		const std::type_info& (* const CallTypeId)();
		void* (* const CallTarget)(const ManagerStorage& manager, const std::type_info& type);

		template<typename Type, typename Allocator>
		static void TemplatedCallMoveDestroy(ManagerStorage& lhs, ManagerStorage&& rhs)
		{
			typedef FunctionManagerSpecialization<Type, Allocator> specialization;
			specialization::Move(lhs, FUNC_MOVE(rhs));
			specialization::Destroy(rhs.GetAllocator<Allocator>(), rhs);
			CreateManager<Type, Allocator>(lhs, FUNC_MOVE(rhs.GetAllocator<Allocator>()));
			rhs.GetAllocator<Allocator>().~Allocator();
		}
		template<typename Type, typename Allocator>
		static void TemplatedCallCopy(ManagerStorage& lhs, const ManagerStorage& rhs)
		{
			typedef FunctionManagerSpecialization<Type, Allocator> specialization;
			CreateManager<Type, Allocator>(lhs, Allocator(rhs.GetAllocator<Allocator>()));
			specialization::Store(lhs, specialization::FunctorReference(rhs));
		}
		template<typename Type, typename Allocator>
		static void TemplatedCallDestroy(ManagerStorage& self)
		{
			typedef FunctionManagerSpecialization<Type, Allocator> specialization;
			specialization::Destroy(self.GetAllocator<Allocator>(), self);
			self.GetAllocator<Allocator>().~Allocator();
		}
		template<typename Type, typename Allocator>
		static void TemplatedCallCopyFuncOnly(ManagerStorage& lhs, const ManagerStorage& rhs)
		{
			typedef FunctionManagerSpecialization<Type, Allocator> specialization;
			specialization::Store(lhs, specialization::FunctorReference(rhs));
		}
		template<typename Type, typename>
		static const std::type_info& TemplatedCallTypeId()
		{
			return typeid(Type);
		}
		template<typename Type, typename Allocator>
		static void* TemplatedCallTarget(const ManagerStorage& self, const std::type_info& type)
		{
			typedef FunctionManagerSpecialization<Type, Allocator> specialization;
			if (type == typeid(Type)) { return &specialization::FunctorReference(self); }
			else { return nullptr; }
		}
	};

	template<typename Type, typename Allocator>
	inline static const FunctionManager& DefaultManager()
	{
		static const FunctionManager defaultManager = FunctionManager::CreateDefaultManager<Type, Allocator>();
		return defaultManager;
	}
}

template<typename Result, typename... Arguments>
struct Function<Result(Arguments...)>
{
public:
	Function() { InitializeEmpty(); }
	Function(NullPointer) { InitializeEmpty(); }
	Function(Function&& other) noexcept : Call(other.Call) { other.managerStorage.manager->CallMoveDestroy(managerStorage, FUNC_MOVE(other.managerStorage)); }
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
	template<typename Allocator, typename Type>
	Function(std::allocator_arg_t, const Allocator& allocator, Type functor,
		typename std::enable_if<detail::IsValidFunctionArg<Type, Result(Arguments...)>::value, detail::Empty>::type = detail::Empty())
	{
		if (detail::IsNull(functor)) { InitializeEmpty(); }
		else { Initialize(detail::ToFunctor(FUNC_FORWARD(Type, functor)), Allocator(allocator)); }
	}
	template<typename Allocator>
	Function(std::allocator_arg_t, const Allocator& allocator, const Function& other) : Call(other.Call)
	{
		typedef typename std::allocator_traits<Allocator>::template rebind_alloc<Function> MyAllocator;

		const detail::FunctionManager* manager_for_allocator = &detail::DefaultManager<typename std::allocator_traits<Allocator>::value_type, Allocator>();
		if (other.managerStorage.manager == manager_for_allocator)
		{
			detail::CreateManager<typename std::allocator_traits<Allocator>::value_type, Allocator>(managerStorage, Allocator(allocator));
			manager_for_allocator->CallCopyFuncOnly(managerStorage, other.managerStorage);
		}
		else
		{
			const detail::FunctionManager* manager_for_function = &detail::DefaultManager<Function, MyAllocator>();
			if (other.managerStorage.manager == manager_for_function)
			{
				detail::CreateManager<Function, MyAllocator>(managerStorage, MyAllocator(allocator));
				manager_for_function->CallCopyFuncOnly(managerStorage, other.managerStorage);
			}
			else { Initialize(other, MyAllocator(allocator)); }
		}
	}
	template<typename Allocator>
	Function(std::allocator_arg_t, const Allocator&, Function&& other)
	{
		Call = other.Call;
		other.managerStorage.manager->CallMoveDestroy(managerStorage, FUNC_MOVE(other.managerStorage));
	}

	Function& operator=(const Function& other)
	{
		Call = other.Call;
		other.managerStorage.manager->CallCopy(managerStorage, other.managerStorage);

		return *this;
	}

	Function& operator=(Function&& other)
	{
		Call = other.Call;
		other.managerStorage.manager->CallMoveDestroy(managerStorage, FUNC_MOVE(other.managerStorage));

		return *this;
	}

	~Function() { managerStorage.manager->CallDestroy(managerStorage); }

	Result operator()(Arguments... arguments) const { return Call(managerStorage.functor, FUNC_FORWARD(Arguments, arguments)...); }

	bool operator== (const Function& other) const { return Call == other.Call; }
	bool operator!= (const Function& other) const { return Call != other.Call; }
	operator bool() const { return Call != nullptr; }

private:
	detail::ManagerStorage managerStorage;
	Result(*Call)(const detail::FunctorPadding&, Arguments...);

	template<typename Type, typename Allocator>
	void Initialize(Type functor, Allocator&& allocator)
	{
		Call = &detail::FunctionManagerSpecialization<Type, Allocator>::template Call<Result, Arguments...>;
		detail::CreateManager<Type, Allocator>(managerStorage, FUNC_FORWARD(Allocator, allocator));
		detail::FunctionManagerSpecialization<Type, Allocator>::Store(managerStorage, FUNC_FORWARD(Type, functor));
	}

	typedef Result(*Empty_Function_Type)(Arguments...);
	void InitializeEmpty()
	{
		typedef std::allocator<Empty_Function_Type> Allocator;
		static_assert(detail::InplaceAllocated<Empty_Function_Type>, "The empty function should benefit from small functor optimization");

		detail::CreateManager<Empty_Function_Type, Allocator>(managerStorage, Allocator());
		detail::FunctionManagerSpecialization<Empty_Function_Type, Allocator>::Store(managerStorage, nullptr);
		Call = nullptr;
	}
};

template<typename T> bool operator==(NullPointer, const Function<T>& rhs) { return !rhs; }
template<typename T> bool operator==(const Function<T>& lhs, NullPointer) { return !lhs; }
template<typename T> bool operator!=(NullPointer, const Function<T>& rhs) { return rhs; }
template<typename T> bool operator!=(const Function<T>& lhs, NullPointer) { return lhs; }