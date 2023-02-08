#pragma once

#include "Defines.hpp"

#include "String.hpp"

#include <io.h>
#include <fcntl.h>

enum FileOpenParam
{
	FILE_OPEN_READ = _O_RDONLY | _O_SEQUENTIAL,
	FILE_OPEN_WRITE = _O_WRONLY,
	FILE_OPEN_WRITE_NEW = _O_CREAT | _O_TRUNC | _O_WRONLY,
};

//TODO: https://learn.microsoft.com/en-us/cpp/c-runtime-library/low-level-i-o?view=msvc-170
//TODO: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/sopen-s-wsopen-s?view=msvc-170
struct File
{
public:
	File();
	File(const String& path, FileOpenParam param);

	~File(); //calls close
	void Close();

	bool Open(const String& path, FileOpenParam param);
	bool Opened() const;

private:

	//TODO: additional info
	int handle;
};

File::File() : handle{ 0 } {}

File::File(const String& path, FileOpenParam param)
{
	
}

File::~File() { Close(); }

bool File::Open(const String& path, FileOpenParam param)
{
	_sopen_s(&handle, path.Data(), param, 0, 0);
}

void File::Close() { if (handle) { _close(handle); } }

bool File::Opened() const { return handle; }