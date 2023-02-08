#pragma once

#include "Defines.hpp"

#include "String.hpp"

#include <io.h>

struct File
{
public:
	File();
	File(const String& path);

	~File(); //calls close
	void Close();

	bool Open(const String& path);
	bool Opened() const;

private:

	//TODO: additional info
	int* handle;
	bool opened;
};

bool File::Open(const String& path)
{
	//TODO: https://learn.microsoft.com/en-us/cpp/c-runtime-library/low-level-i-o?view=msvc-170
	//TODO: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/sopen-s-wsopen-s?view=msvc-170
	_sopen_s(handle, path.Data(), 0, 0, 0);
}