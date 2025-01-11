module;

#include "Defines.hpp"
#include "TypeTraits.hpp"
#include <functional>

export module Core:Function;

export constexpr inline std::_Ph<1> Placeholder1 = std::placeholders::_1;
export constexpr inline std::_Ph<2> Placeholder2 = std::placeholders::_2;
export constexpr inline std::_Ph<3> Placeholder3 = std::placeholders::_3;
export constexpr inline std::_Ph<4> Placeholder4 = std::placeholders::_4;
export constexpr inline std::_Ph<5> Placeholder5 = std::placeholders::_5;
export constexpr inline std::_Ph<6> Placeholder6 = std::placeholders::_6;
export constexpr inline std::_Ph<7> Placeholder7 = std::placeholders::_7;
export constexpr inline std::_Ph<8> Placeholder8 = std::placeholders::_8;
export constexpr inline std::_Ph<9> Placeholder9 = std::placeholders::_9;
export constexpr inline std::_Ph<10> Placeholder10 = std::placeholders::_10;
export constexpr inline std::_Ph<11> Placeholder11 = std::placeholders::_11;
export constexpr inline std::_Ph<12> Placeholder12 = std::placeholders::_12;
export constexpr inline std::_Ph<13> Placeholder13 = std::placeholders::_13;
export constexpr inline std::_Ph<14> Placeholder14 = std::placeholders::_14;
export constexpr inline std::_Ph<15> Placeholder15 = std::placeholders::_15;
export constexpr inline std::_Ph<16> Placeholder16 = std::placeholders::_16;
export constexpr inline std::_Ph<17> Placeholder17 = std::placeholders::_17;
export constexpr inline std::_Ph<18> Placeholder18 = std::placeholders::_18;
export constexpr inline std::_Ph<19> Placeholder19 = std::placeholders::_19;
export constexpr inline std::_Ph<20> Placeholder20 = std::placeholders::_20;

export template <class Fn, class... Args>
_NODISCARD constexpr std::_Binder<std::_Unforced, Fn, Args...> Bind(Fn&& func, Args&&... args)
{
	return std::_Binder<std::_Unforced, Fn, Args...>(Forward<Fn>(func), Forward<Args>(args)...);
}

export template <class Return, class Fn, class... Args>
_NODISCARD constexpr std::_Binder<Return, Fn, Args...> Bind(Fn&& func, Args&&... args)
{
	return std::_Binder<Return, Fn, Args...>(Forward<Fn>(func), Forward<Args>(args)...);
}

#define FUNC_MOVE(value) static_cast<RemoveReference<decltype(value)>&&>(value)
#define FUNC_FORWARD(type, value) static_cast<type &&>(value)

template <U32 N>
struct PlaceHolder {
	static_assert(N > 0, "invalid placeholder index");
};

export template<class> struct Function;

struct FunctorPadding
{
private:
	U64 unused[2];
};

export struct Empty {};

template<typename Type>
struct IsInplaceAllocated
{
	static constexpr bool value = sizeof(Type) <= sizeof(FunctorPadding) &&
		Alignment<FunctorPadding> % Alignment<Type> == 0 && IsNothrowMoveConstructible<Type>;
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

export template<class, class> 
struct IsValidFunctionArg { static constexpr bool value = false; };

export template<class Result, class... Arguments>
struct IsValidFunctionArg<Function<Result(Arguments...)>, Result(Arguments...)> { static constexpr bool value = false; };

export template<class Type, class Result, class... Arguments>
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
		static_assert(IsNothrowMoveConstructible<typename std::allocator_traits<Allocator>::pointer>, "we can't offer a noexcept swap if the pointer type is not nothrow move constructible");
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
	static inline const FunctionManager CreateDefaultManager()
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
static inline const FunctionManager& DefaultManager()
{
	static const FunctionManager defaultManager = FunctionManager::CreateDefaultManager<Type, Allocator>();
	return defaultManager;
}

export template<typename Result, typename... Arguments>
struct NH_API Function<Result(Arguments...)>
{
public:
	Function() { InitializeEmpty(); }
	Function(NullPointer) { InitializeEmpty(); }
	Function(Function&& other) noexcept : Call(other.Call) { other.managerStorage.manager->CallMoveDestroy(managerStorage, FUNC_MOVE(other.managerStorage)); }
	Function(const Function& other) : Call(other.Call) { other.managerStorage.manager->CallCopy(managerStorage, other.managerStorage); }
	template<typename Type> Function(Type functor, typename Enable<IsValidFunctionArg<Type, Result(Arguments...)>::value, Empty> = Empty())
	{
		if (IsNull(functor)) { InitializeEmpty(); }
		else
		{
			typedef typename FunctorType<Type>::type FunctorType;
			Initialize(ToFunctor(FUNC_FORWARD(Type, functor)), std::allocator<FunctorType>());
		}
	}

	template<typename Allocator> Function(std::allocator_arg_t, const Allocator&) { InitializeEmpty(); }
	template<typename Allocator> Function(std::allocator_arg_t, const Allocator&, NullPointer) { InitializeEmpty(); }
	template<typename Allocator, typename Type>
	Function(std::allocator_arg_t, const Allocator& allocator, Type functor,
		typename Enable<IsValidFunctionArg<Type, Result(Arguments...)>::value, Empty>::type = Empty())
	{
		if (IsNull(functor)) { InitializeEmpty(); }
		else { Initialize(ToFunctor(FUNC_FORWARD(Type, functor)), Allocator(allocator)); }
	}
	template<typename Allocator>
	Function(std::allocator_arg_t, const Allocator& allocator, const Function& other) : Call(other.Call)
	{
		typedef typename std::allocator_traits<Allocator>::template rebind_alloc<Function> MyAllocator;

		const FunctionManager* manager_for_allocator = &DefaultManager<typename std::allocator_traits<Allocator>::value_type, Allocator>();
		if (other.managerStorage.manager == manager_for_allocator)
		{
			CreateManager<typename std::allocator_traits<Allocator>::value_type, Allocator>(managerStorage, Allocator(allocator));
			manager_for_allocator->CallCopyFuncOnly(managerStorage, other.managerStorage);
		}
		else
		{
			const FunctionManager* manager_for_function = &DefaultManager<Function, MyAllocator>();
			if (other.managerStorage.manager == manager_for_function)
			{
				CreateManager<Function, MyAllocator>(managerStorage, MyAllocator(allocator));
				manager_for_function->CallCopyFuncOnly(managerStorage, other.managerStorage);
			}
			else { Initialize(other, MyAllocator(allocator)); }
		}
	}
	template<typename Allocator>
	Function(std::allocator_arg_t, const Allocator&, Function&& other) noexcept : Call(other.Call)
	{
		other.managerStorage.manager->CallMoveDestroy(managerStorage, FUNC_MOVE(other.managerStorage));
	}

	Function& operator=(const Function& other)
	{
		Call = other.Call;
		other.managerStorage.manager->CallCopy(managerStorage, other.managerStorage);

		return *this;
	}

	Function& operator=(Function&& other) noexcept
	{
		Call = other.Call;
		other.managerStorage.manager->CallMoveDestroy(managerStorage, FUNC_MOVE(other.managerStorage));

		return *this;
	}

	~Function() { managerStorage.manager->CallDestroy(managerStorage); }

	Result operator()(Arguments... arguments) const { return Call(managerStorage.functor, static_cast<Arguments&&>(arguments)...); }

	bool operator== (const Function& other) const { return Call == other.Call; }
	bool operator!= (const Function& other) const { return Call != other.Call; }
	operator bool() const { return Call != nullptr; }

private:
	ManagerStorage managerStorage;
	Result(*Call)(const FunctorPadding&, Arguments...);

	template<typename Type, typename Allocator>
	void Initialize(Type functor, Allocator&& allocator)
	{
		Call = &FunctionManagerSpecialization<Type, Allocator>::template Call<Result, Arguments...>;
		CreateManager<Type, Allocator>(managerStorage, FUNC_FORWARD(Allocator, allocator));
		FunctionManagerSpecialization<Type, Allocator>::Store(managerStorage, FUNC_FORWARD(Type, functor));
	}

	typedef Result(*Empty_Function_Type)(Arguments...);
	void InitializeEmpty()
	{
		typedef std::allocator<Empty_Function_Type> Allocator;
		static_assert(InplaceAllocated<Empty_Function_Type>, "The empty function should benefit from small functor optimization");

		CreateManager<Empty_Function_Type, Allocator>(managerStorage, Allocator());
		FunctionManagerSpecialization<Empty_Function_Type, Allocator>::Store(managerStorage, nullptr);
		Call = nullptr;
	}
};

export template<typename T> bool operator==(NullPointer, const Function<T>& rhs) { return !rhs; }
export template<typename T> bool operator==(const Function<T>& lhs, NullPointer) { return !lhs; }
export template<typename T> bool operator!=(NullPointer, const Function<T>& rhs) { return rhs; }
export template<typename T> bool operator!=(const Function<T>& lhs, NullPointer) { return lhs; }