#pragma once

import Containers;

#include "Defines.hpp"

namespace Introspection
{
	struct DummyStruct{};

	template<class Type>
	struct TypeName
	{
		static constexpr inline StringView FullName()
		{
#if defined(__clang__) || defined(__GNUC__)
			return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)	
			return __FUNCSIG__;
#else
#	error "Unsupported compiler"
#endif
		}

		static constexpr inline StringView Name()
		{
			constexpr StringView dummy = TypeName<DummyStruct>::FullName();
			constexpr StringView type = FullName();
			constexpr U64 dummyLength = dummy.Size();
			constexpr U64 typeLength = type.Size();
			constexpr U64 prefixIndex = dummy.IndexOf(" ", dummy.IndexOf("<")) + 1;
			constexpr U64 suffixIndex = dummy.IndexOf(">", prefixIndex);

			return type.SubString(prefixIndex, (suffixIndex - prefixIndex) + (typeLength - dummyLength));
		}

		constexpr static inline StringView value = Name();
	};
}

template <class Type> constexpr inline const StringView NameOf = Introspection::TypeName<Type>::value;