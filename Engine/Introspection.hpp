#pragma once

import Containers;

#include "Defines.hpp"

namespace Introspection
{
	template<class Type>
	struct TypeName
	{
		static constexpr StringView FullName()
		{
#if defined(__clang__) || defined(__GNUC__)
			return __PRETTY_FUNCTION__;
#elif defined(_MSC_VER)	
			return __FUNCSIG__;
#else
#	error "Unsupported compiler"
#endif
		}

		static constexpr StringView Name()
		{
			constexpr StringView dummy = TypeName<void>::FullName();
			constexpr U64 dummyLength = dummy.Size();
			constexpr U64 prefixLength = dummy.IndexOf("void");

			U64 multiple = dummyLength - TypeName<int>::FullName().Size();
			U64 targetLength = (FullName().Size() - (dummyLength - 4 * multiple)) / multiple;
			StringView rv = FullName().SubString(prefixLength, targetLength);

			if (rv.LastIndexOf(' ') == U64_MAX) { return rv; }
			return rv.SubString(rv.LastIndexOf(' ') + 1);
		}

		constexpr static StringView value = Name();
	};
}

template <class Type> inline constexpr StringView NameOf = Introspection::TypeName<Type>::value;