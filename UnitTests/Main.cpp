#include "Containers\String.hpp"

bool StringCopying()
{
	String str = "abcdefg12345";
	String str1 = str;

	return Compare(str1.Data(), "abcdefg12345") && Compare(str.Data(), str1.Data()) && str.Data() != str1.Data();
}

bool StringMoving()
{
	String str = "abcdefg12345";
	String str1 = Move(str);

	return str.Data() == nullptr && Compare(str1.Data(), "abcdefg12345");
}

int main()
{
	ASSERT(StringCopying());
	ASSERT(StringMoving());
}

