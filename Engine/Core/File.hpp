#pragma once

#include "Defines.hpp"

struct String;

struct File
{
public:
	File();
	File(const String& path);

	~File(); //calls close
	void Close();

	bool Opened() const;

private:

	//TODO: additional info
	void* handle;
	bool opened;
};