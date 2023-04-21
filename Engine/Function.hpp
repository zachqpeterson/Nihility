#pragma once

#include "Defines.hpp"

#include <utility>
#include <type_traits>
#include <functional>
#include <exception>
#include <typeinfo>
#include <memory>

#define FUNC_NO_EXCEPTIONS

#define FUNC_MOVE(value) static_cast<RemovedReference<decltype(value)>&&>(value)
#define FUNC_FORWARD(type, value) static_cast<type &&>(value)

namespace func
{
	template<typename Type> struct ForceFunctionHeapAlloc : FalseConstant { };

	template<typename> class function;

	namespace detail
	{
		struct manager_storage_type;
		struct function_manager;
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
		bool IsNull(Result(* const& function_pointer)(Arguments...)) { return function_pointer == nullptr; }

		template<typename Result, typename Class, typename... Arguments>
		bool IsNull(Result(Class::* const& function_pointer)(Arguments...)) { return function_pointer == nullptr; }

		template<typename Result, typename Class, typename... Arguments>
		bool IsNull(Result(Class::* const& function_pointer)(Arguments...) const) { return function_pointer == nullptr; }

		template<typename, typename> struct IsValidFunctionArg { static constexpr bool value = false; };

		template<typename Result, typename... Arguments>
		struct IsValidFunctionArg<function<Result(Arguments...)>, Result(Arguments...)> { static constexpr bool value = false; };

		template<typename T, typename Result, typename... Arguments>
		struct IsValidFunctionArg<T, Result(Arguments...)> { static constexpr bool value = true; };

		typedef const function_manager* manager_type;

		struct manager_storage_type
		{
			template<typename Allocator> Allocator& GetAllocator() { return reinterpret_cast<Allocator&>(manager); }
			template<typename Allocator> const Allocator& GetAllocator() const { return reinterpret_cast<const Allocator&>(manager); }

			FunctorPadding functor;
			manager_type manager;
		};

		template<typename T, typename Allocator, typename Enable = void>
		struct function_manager_inplace_specialization
		{
			template<typename Result, typename... Arguments> static Result call(const FunctorPadding& storage, Arguments... arguments)
			{
				return const_cast<T&>(reinterpret_cast<const T&>(storage))(FUNC_FORWARD(Arguments, arguments)...);
			}

			static void store_functor(manager_storage_type& storage, T to_store) { new (&get_functor_ref(storage)) T(FUNC_FORWARD(T, to_store)); }
			static void move_functor(manager_storage_type& lhs, manager_storage_type&& rhs) { new (&get_functor_ref(lhs)) T(FUNC_MOVE(get_functor_ref(rhs))); }
			static void destroy_functor(Allocator&, manager_storage_type& storage) { get_functor_ref(storage).~T(); }
			static T& get_functor_ref(const manager_storage_type& storage) { return const_cast<T&>(reinterpret_cast<const T&>(storage.functor)); }
		};
		template<typename T, typename Allocator>
		struct function_manager_inplace_specialization<T, Allocator, Enable<!InplaceAllocated<T, Allocator>>>
		{
			template<typename Result, typename... Arguments>
			static Result call(const FunctorPadding& storage, Arguments... arguments)
			{
				return (*reinterpret_cast<const typename std::allocator_traits<Allocator>::pointer&>(storage))(FUNC_FORWARD(Arguments, arguments)...);
			}

			static void store_functor(manager_storage_type& self, T to_store)
			{
				Allocator& allocator = self.GetAllocator<Allocator>();;
				static_assert(sizeof(typename std::allocator_traits<Allocator>::pointer) <= sizeof(self.functor), "The allocator's pointer type is too big");
				typename std::allocator_traits<Allocator>::pointer* ptr = new (&get_functor_ptr_ref(self)) typename std::allocator_traits<Allocator>::pointer(std::allocator_traits<Allocator>::allocate(allocator, 1));
				std::allocator_traits<Allocator>::construct(allocator, *ptr, FUNC_FORWARD(T, to_store));
			}
			static void move_functor(manager_storage_type& lhs, manager_storage_type&& rhs) 
			{
				static_assert(NothrowMoveConstructible<typename std::allocator_traits<Allocator>::pointer>, "we can't offer a noexcept swap if the pointer type is not nothrow move constructible");
				new (&get_functor_ptr_ref(lhs)) typename std::allocator_traits<Allocator>::pointer(FUNC_MOVE(get_functor_ptr_ref(rhs)));
				get_functor_ptr_ref(rhs) = nullptr;
			}
			static void destroy_functor(Allocator& allocator, manager_storage_type& storage) 
			{
				typename std::allocator_traits<Allocator>::pointer& pointer = get_functor_ptr_ref(storage);
				if (!pointer) return;
				std::allocator_traits<Allocator>::destroy(allocator, pointer);
				std::allocator_traits<Allocator>::deallocate(allocator, pointer, 1);
			}
			static T& get_functor_ref(const manager_storage_type& storage) { return *get_functor_ptr_ref(storage); }
			static typename std::allocator_traits<Allocator>::pointer& get_functor_ptr_ref(manager_storage_type& storage) 
			{
				return reinterpret_cast<typename std::allocator_traits<Allocator>::pointer&>(storage.functor);
			}
			static const typename std::allocator_traits<Allocator>::pointer& get_functor_ptr_ref(const manager_storage_type& storage) 
			{
				return reinterpret_cast<const typename std::allocator_traits<Allocator>::pointer&>(storage.functor);
			}
		};

		template<typename T, typename Allocator> static const function_manager& get_default_manager();

		template<typename T, typename Allocator>
		static void create_manager(manager_storage_type& storage, Allocator&& allocator)
		{
			new (&storage.GetAllocator<Allocator>()) Allocator(FUNC_MOVE(allocator));
			storage.manager = &get_default_manager<T, Allocator>();
		}

		struct function_manager
		{
			template<typename T, typename Allocator>
			inline static const function_manager create_default_manager()
			{
				function_manager result =
				{
					&templated_call_move_and_destroy<T, Allocator>,
					&templated_call_copy<T, Allocator>,
					&templated_call_copy_functor_only<T, Allocator>,
					&templated_call_destroy<T, Allocator>,
					&templated_call_type_id<T, Allocator>,
					&templated_call_target<T, Allocator>
				};

				return result;
			}

			void (* const call_move_and_destroy)(manager_storage_type& lhs, manager_storage_type&& rhs);
			void (* const call_copy)(manager_storage_type& lhs, const manager_storage_type& rhs);
			void (* const call_copy_functor_only)(manager_storage_type& lhs, const manager_storage_type& rhs);
			void (* const call_destroy)(manager_storage_type& manager);
			const std::type_info& (* const call_type_id)();
			void* (* const call_target)(const manager_storage_type& manager, const std::type_info& type);

			template<typename T, typename Allocator>
			static void templated_call_move_and_destroy(manager_storage_type& lhs, manager_storage_type&& rhs)
			{
				typedef function_manager_inplace_specialization<T, Allocator> specialization;
				specialization::move_functor(lhs, FUNC_MOVE(rhs));
				specialization::destroy_functor(rhs.GetAllocator<Allocator>(), rhs);
				create_manager<T, Allocator>(lhs, FUNC_MOVE(rhs.GetAllocator<Allocator>()));
				rhs.GetAllocator<Allocator>().~Allocator();
			}
			template<typename T, typename Allocator>
			static void templated_call_copy(manager_storage_type& lhs, const manager_storage_type& rhs)
			{
				typedef function_manager_inplace_specialization<T, Allocator> specialization;
				create_manager<T, Allocator>(lhs, Allocator(rhs.GetAllocator<Allocator>()));
				specialization::store_functor(lhs, specialization::get_functor_ref(rhs));
			}
			template<typename T, typename Allocator>
			static void templated_call_destroy(manager_storage_type& self)
			{
				typedef function_manager_inplace_specialization<T, Allocator> specialization;
				specialization::destroy_functor(self.GetAllocator<Allocator>(), self);
				self.GetAllocator<Allocator>().~Allocator();
			}
			template<typename T, typename Allocator>
			static void templated_call_copy_functor_only(manager_storage_type& lhs, const manager_storage_type& rhs)
			{
				typedef function_manager_inplace_specialization<T, Allocator> specialization;
				specialization::store_functor(lhs, specialization::get_functor_ref(rhs));
			}
			template<typename T, typename>
			static const std::type_info& templated_call_type_id()
			{
				return typeid(T);
			}
			template<typename T, typename Allocator>
			static void* templated_call_target(const manager_storage_type& self, const std::type_info& type)
			{
				typedef function_manager_inplace_specialization<T, Allocator> specialization;
				if (type == typeid(T)) { return &specialization::get_functor_ref(self); }
				else { return nullptr; }
			}
		};
		template<typename T, typename Allocator>
		inline static const function_manager& get_default_manager()
		{
			static const function_manager default_manager = function_manager::create_default_manager<T, Allocator>();
			return default_manager;
		}

		template<typename Result, typename...>
		struct typedeffer
		{
			typedef Result result_type;
		};
		template<typename Result, typename Argument>
		struct typedeffer<Result, Argument>
		{
			typedef Result result_type;
			typedef Argument argument_type;
		};
		template<typename Result, typename First_Argument, typename Second_Argument>
		struct typedeffer<Result, First_Argument, Second_Argument>
		{
			typedef Result result_type;
			typedef First_Argument first_argument_type;
			typedef Second_Argument second_argument_type;
		};
	}

	template<typename Result, typename... Arguments>
	class function<Result(Arguments...)> : public detail::typedeffer<Result, Arguments...>
	{
	public:
		function() { initialize_empty(); }
		function(NullPointer) { initialize_empty(); }
		function(function&& other) { initialize_empty(); swap(other); }
		function(const function& other) : call(other.call) { other.manager_storage.manager->call_copy(manager_storage, other.manager_storage); }
		template<typename T> function(T functor,typename Enable<detail::IsValidFunctionArg<T, Result(Arguments...)>::value, detail::Empty> = detail::Empty())
		{
			if (detail::IsNull(functor)) { initialize_empty(); }
			else
			{
				typedef typename detail::FunctorType<T>::type functor_type;
				initialize(detail::ToFunctor(FUNC_FORWARD(T, functor)), std::allocator<functor_type>());
			}
		}
		template<typename Allocator> function(std::allocator_arg_t, const Allocator&) { initialize_empty(); }
		template<typename Allocator> function(std::allocator_arg_t, const Allocator&, NullPointer) { initialize_empty(); }
		template<typename Allocator, typename T>
		function(std::allocator_arg_t, const Allocator& allocator, T functor,
			typename Enable<detail::IsValidFunctionArg<T, Result(Arguments...)>::value, detail::Empty> = detail::Empty())
		{
			if (detail::IsNull(functor)) { initialize_empty(); }
			else { initialize(detail::ToFunctor(FUNC_FORWARD(T, functor)), Allocator(allocator)); }
		}
		template<typename Allocator>
		function(std::allocator_arg_t, const Allocator& allocator, const function& other) : call(other.call)
		{
			typedef typename std::allocator_traits<Allocator>::template rebind_alloc<function> MyAllocator;

			detail::manager_type manager_for_allocator = &detail::get_default_manager<typename std::allocator_traits<Allocator>::value_type, Allocator>();
			if (other.manager_storage.manager == manager_for_allocator)
			{
				detail::create_manager<typename std::allocator_traits<Allocator>::value_type, Allocator>(manager_storage, Allocator(allocator));
				manager_for_allocator->call_copy_functor_only(manager_storage, other.manager_storage);
			}
			else
			{
				detail::manager_type manager_for_function = &detail::get_default_manager<function, MyAllocator>();
				if (other.manager_storage.manager == manager_for_function)
				{
					detail::create_manager<function, MyAllocator>(manager_storage, MyAllocator(allocator));
					manager_for_function->call_copy_functor_only(manager_storage, other.manager_storage);
				}
				else { initialize(other, MyAllocator(allocator)); }
			}
		}

		template<typename Allocator> function(std::allocator_arg_t, const Allocator&, function&& other) { initialize_empty(); swap(other); }

		function& operator=(function other) { swap(other); return *this; }

		~function() { manager_storage.manager->call_destroy(manager_storage); }

		Result operator()(Arguments... arguments) const { return call(manager_storage.functor, FUNC_FORWARD(Arguments, arguments)...); }

		template<typename T, typename Allocator> void assign(T&& functor, const Allocator& allocator) { function(std::allocator_arg, allocator, functor).swap(*this); }

		void swap(function& other) 
		{
			detail::manager_storage_type temp_storage;
			other.manager_storage.manager->call_move_and_destroy(temp_storage, FUNC_MOVE(other.manager_storage));
			manager_storage.manager->call_move_and_destroy(other.manager_storage, FUNC_MOVE(manager_storage));
			temp_storage.manager->call_move_and_destroy(manager_storage, FUNC_MOVE(temp_storage));

			std::swap(call, other.call);
		}

		const std::type_info& target_type() const { return manager_storage.manager->call_type_id(); }

		template<typename T> T* target() { return static_cast<T*>(manager_storage.manager->call_target(manager_storage, typeid(T))); }
		template<typename T> const T* target() const { return static_cast<const T*>(manager_storage.manager->call_target(manager_storage, typeid(T))); }

		operator bool() const { return call != nullptr; }

	private:
		detail::manager_storage_type manager_storage;
		Result(*call)(const detail::FunctorPadding&, Arguments...);

		template<typename T, typename Allocator>
		void initialize(T functor, Allocator&& allocator)
		{
			call = &detail::function_manager_inplace_specialization<T, Allocator>::template call<Result, Arguments...>;
			detail::create_manager<T, Allocator>(manager_storage, FUNC_FORWARD(Allocator, allocator));
			detail::function_manager_inplace_specialization<T, Allocator>::store_functor(manager_storage, FUNC_FORWARD(T, functor));
		}

		typedef Result(*Empty_Function_Type)(Arguments...);
		void initialize_empty() 
		{
			typedef std::allocator<Empty_Function_Type> Allocator;
			static_assert(detail::InplaceAllocated<Empty_Function_Type, Allocator>, "The empty function should benefit from small functor optimization");

			detail::create_manager<Empty_Function_Type, Allocator>(manager_storage, Allocator());
			detail::function_manager_inplace_specialization<Empty_Function_Type, Allocator>::store_functor(manager_storage, nullptr);
			call = nullptr;
		}
	};

	template<typename T> bool operator==(NullPointer, const function<T>& rhs) { return !rhs; }
	template<typename T> bool operator==(const function<T>& lhs, NullPointer) { return !lhs; }
	template<typename T> bool operator!=(NullPointer, const function<T>& rhs) { return rhs; }
	template<typename T> bool operator!=(const function<T>& lhs, NullPointer) { return lhs; }
	template<typename T> void swap(function<T>& lhs, function<T>& rhs) { lhs.swap(rhs); }
}

namespace std
{
	template<typename Result, typename... Arguments, typename Allocator>
	struct uses_allocator<func::function<Result(Arguments...)>, Allocator> : std::true_type {};
}