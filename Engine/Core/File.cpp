#include "File.hpp"

#include <io.h>
#include <sys/stat.h>
#include <share.h>

File::File() { Memory::AllocateArray(&streamBuffer, bufferSize, bufferSize); streamPtr = streamBuffer; }

File::~File() { Destroy(); }

bool File::Open(const C8* path, I32 mode)
{
	if (opened) { Close(); }

	_sopen_s(&handle, path, mode, _SH_DENYNO, _S_IREAD | _S_IWRITE);

	if (handle < 0 || _fstat64(handle, (struct _stat64*)&stats)) { return false; }

	switch (mode & READ_WRITE_MASK)
	{
	case 0: { streamFlag = READ_MODE; } break;
	case 1: { streamFlag = WRITE_MODE; bufferRemaining = bufferSize; } break;
	case 2: { streamFlag = READ_MODE | WRITE_MODE; } break;
	}

	if (mode & FILE_OPEN_FLUSH_IMMEDIATE) { streamFlag |= FILE_OPEN_FLUSH_IMMEDIATE; }

	opened = true;

	return true;
}

void File::Destroy()
{
	Close();
	if (streamBuffer) { Memory::FreeArray(&streamBuffer); }
}

void File::Close()
{
	if (opened)
	{
		Flush();
		_close(handle);

		streamFlag = 0;
		opened = false;
	}
}

bool File::Opened() const { return opened; }

U32 File::Read(void* buffer, U32 size)
{
	U32 total = size;
	U32 nBytes;
	I32 nRead;
	U8* data = (U8*)buffer;

	if (!(streamFlag & READ_MODE)) { return 0; }

	while (size)
	{
		if (bufferRemaining)
		{
			nBytes = size < bufferRemaining ? size : bufferRemaining;
			Memory::Copy(data, streamPtr, nBytes);

			size -= nBytes;
			bufferRemaining -= nBytes;
			streamPtr += nBytes;
			data += nBytes;
			pointer += nBytes;
		}
		else if (size >= bufferSize)
		{
			nRead = _read(handle, data, size);

			if (nRead <= 0) { return total - size; }

			size -= nRead;
			data += nRead;
			pointer += nRead;
		}
		else if (!FillBuffer()) { return total - size; }
	}

	return total;
}

void File::ReadString(String& string)
{
	string.Clear();

	bool end = false;

	while (!end)
	{
		C8* it = (C8*)streamPtr;

		while (bufferRemaining && *it != '\0') { ++it; }

		if (bufferRemaining && *it == '\0')
		{
			string.Append((C8*)streamPtr);
			streamPtr += string.Size() + 1;
			bufferRemaining -= string.Size() + 1;
			pointer += string.Size() + 1;
			end = true;
		}
		else if (!FillBuffer())
		{
			end = true;
		}
	}
}

void File::ReadLine(String& string)
{
	string.Clear();

	bool end = false;

	while (!end)
	{
		C8* it = (C8*)streamPtr;

		while (bufferRemaining && *it != '\n' && *it != '\r') { ++it; }

		if (bufferRemaining && *it == '\n' || *it != '\r')
		{
			string.Append((C8*)streamPtr);
			streamPtr += string.Size() + 1;
			bufferRemaining -= string.Size() + 1;
			pointer += string.Size() + 1;
			end = true;
		}
		else if(!FillBuffer())
		{
			end = true;
		}
	}
}

//TODO: update file size
U32 File::Write(const void* buffer, U32 size)
{
	const U8* data = (const U8*)buffer;
	U32 total = size;
	U32 nBytes;
	I32 nWritten;

	if (!(streamFlag & WRITE_MODE)) { return 0; }

	if (streamFlag & FILE_OPEN_FLUSH_IMMEDIATE) { size -= _write(handle, data, size); }
	else
	{
		while (size)
		{
			if (bufferRemaining)
			{
				nBytes = size < bufferRemaining ? size : bufferRemaining;
				Memory::Copy(streamPtr, data, nBytes);

				size -= nBytes;
				bufferRemaining -= nBytes;
				streamPtr += nBytes;
				data += nBytes;
				pointer += nBytes;
			}
			else if (size >= bufferSize)
			{
				if (!Flush() || (nWritten = _write(handle, data, size)) < 0) { return total - size; }

				size -= nWritten;
				data += nWritten;
				pointer += nWritten;

				if ((U32)nWritten < size) { return total - size; }
			}
			else if (!EmptyBuffer()) { return total - size; }
		}
	}

	return total - size;
}

bool File::FillBuffer()
{
	streamPtr = streamBuffer;
	bufferRemaining = _read(handle, streamBuffer, bufferSize);

	if (bufferRemaining <= 0) { bufferRemaining = 0; }

	return bufferRemaining;
}

bool File::EmptyBuffer()
{
	U32 count = (U32)(streamPtr - streamBuffer);
	I32 written = 0;

	streamPtr = streamBuffer;
	bufferRemaining = bufferSize;

	if (count > 0) { written = _write(handle, streamBuffer, count); }

	if (written != count) { return false; }

	return true;
}

bool File::Flush()
{
	U32 count;
	I32 written;

	if (streamFlag & WRITE_MODE && (count = (U32)(streamPtr - streamBuffer)) > 0)
	{
		written = _write(handle, streamBuffer, count);

		if (written != count)
		{
			streamPtr = streamBuffer;
			bufferRemaining = 0;
			return false;
		}

		streamPtr = streamBuffer;
		bufferRemaining = bufferSize;
	}

	return true;
}

void File::Reset()
{
	Flush();
	pointer = _lseeki64(handle, 0, 0);
}

void File::Seek(I64 offset)
{
	if (streamFlag & READ_MODE) { offset -= bufferRemaining; }
	Flush();
	pointer = _lseeki64(handle, offset, 1);
	bufferRemaining = 0;
}

void File::SeekFromStart(I64 offset)
{
	Flush();
	pointer = _lseeki64(handle, offset, 0);
	bufferRemaining = 0;
}

void File::SeekToEnd()
{
	Flush();
	pointer = _lseeki64(handle, 0, 2);
	bufferRemaining = 0;
}

I64 File::Pointer() const { return pointer; }

I64 File::Size() { return stats.size; }

const String& File::WorkingDirectory()
{
	static String wd;
	static bool b = GetWorkingDirectory(wd);

	return wd;
}

#if defined PLATFORM_WINDOWS

#include <Windows.h>

bool File::GetWorkingDirectory(String& str)
{
	UL32 length = 1024;

	UL32 i = GetCurrentDirectoryA(length, str.Data());
	str.Resize();

	return i;
}

#endif 

//TODO: Check if file is open
bool File::Delete(const String& path) { return remove(path.Data()) == 0; }