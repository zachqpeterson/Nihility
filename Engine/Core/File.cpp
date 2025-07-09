#include "File.hpp"

#include "Platform/Memory.hpp"

#include <io.h>
#include <sys\stat.h>
#include <share.h>

#ifdef NH_PLATFORM_WINDOWS
#include "Platform/WindowsInclude.hpp"
#endif

static constexpr I32 READ_MODE = 0x0001;
static constexpr I32 WRITE_MODE = 0x0002;
static constexpr I32 READ_WRITE_MASK = 0x0003;

File::File()
{
	bufferSize = Memory::Allocate(&streamBuffer, bufferSize);
	streamPtr = streamBuffer;
}

File::File(const String& path, I32 mode)
{
	bufferSize = Memory::Allocate(&streamBuffer, bufferSize);
	streamPtr = streamBuffer;

	Open(path, mode);
}

File::~File()
{
	Destroy();
}

void File::Destroy()
{
	Close();
	if (streamBuffer) { Memory::Free(&streamBuffer); }
}

bool File::Open(const String& path, I32 mode)
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

	streamPtr = streamBuffer;
	opened = true;

	return true;
}

bool File::Opened() const
{
	return opened;
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

String File::ReadString()
{
	String string;

	while (true)
	{
		C8* it = (C8*)streamPtr;

		while (bufferRemaining && *it != '\0') { ++it; }

		if (bufferRemaining && *it == '\0')
		{
			string.Append((C8*)streamPtr);
			streamPtr += string.Size() + 1;
			bufferRemaining -= (U32)string.Size() + 1;
			pointer += string.Size() + 1;
			return string;
		}
		else if (!FillBuffer())
		{
			return string;
		}
	}
}

String File::ReadLine()
{
	String string;

	while (true)
	{
		C8* it = (C8*)streamPtr;

		while (bufferRemaining && *it != '\n' && *it != '\r') { ++it; }

		if (bufferRemaining && *it == '\n' || *it != '\r')
		{
			string.Append((C8*)streamPtr);
			streamPtr += string.Size() + 1;
			bufferRemaining -= (U32)string.Size() + 1;
			pointer += string.Size() + 1;
			return string;
		}
		else if (!FillBuffer())
		{
			return string;
		}
	}
}

String File::ReadAll()
{
	String string;
	string.Resize(stats.size);
	Read(string.Data(), stats.size);
	return string;
}

U64 File::Write(const String& data)
{
	if (!data.Empty())
	{
		return Write(data.Data(), data.Size() + 1);
	}

	return 0;
}

U64 File::Read(void* buffer, U64 size)
{
	U64 total = size;
	U64 nBytes;
	I32 nRead;
	U8* data = (U8*)buffer;

	if (!(streamFlag & READ_MODE)) { return 0; }

	while (size)
	{
		if (bufferRemaining)
		{
			nBytes = size < bufferRemaining ? size : bufferRemaining;
			memcpy(data, streamPtr, nBytes);

			size -= nBytes;
			bufferRemaining -= nBytes;
			streamPtr += nBytes;
			data += nBytes;
			pointer += nBytes;
		}
		else if (size >= bufferSize)
		{
			nRead = _read(handle, data, (U32)size);

			if (nRead <= 0) { return total - size; }

			size -= nRead;
			data += nRead;
			pointer += nRead;
		}
		else if (!FillBuffer()) { return total - size; }
	}

	return total;
}

U64 File::Write(const void* buffer, U64 size)
{
	const U8* data = (const U8*)buffer;
	U64 total = size;
	U64 nBytes;
	I32 nWritten;

	if (!(streamFlag & WRITE_MODE)) { return 0; }

	if (streamFlag & FILE_OPEN_FLUSH_IMMEDIATE) { size -= _write(handle, data, (U32)size); }
	else
	{
		while (size)
		{
			if (bufferRemaining)
			{
				nBytes = size < bufferRemaining ? size : bufferRemaining;
				memcpy(streamPtr, data, nBytes);

				size -= nBytes;
				bufferRemaining -= nBytes;
				streamPtr += nBytes;
				data += nBytes;
				pointer += nBytes;
			}
			else if (size >= bufferSize)
			{
				if (!Flush() || (nWritten = _write(handle, data, (U32)size)) < 0) { return total - size; }

				size -= nWritten;
				data += nWritten;
				pointer += nWritten;

				if ((U32)nWritten < size) { return total - size; }
			}
			else if (!EmptyBuffer()) { return total - size; }
		}
	}

	stats.size += total - size;

	return total - size;
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

bool File::FillBuffer()
{
	streamPtr = streamBuffer;
	bufferRemaining = _read(handle, streamBuffer, (U32)bufferSize);

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

I64 File::Pointer() const
{
	return pointer;
}

I64 File::Size()
{
	return stats.size;
}

bool GetWorkingDirectory(String& str)
{
#if defined NH_PLATFORM_WINDOWS
	UL32 length = 1024;

	UL32 i = GetCurrentDirectoryA(length, str.Data());
	str.Resize();

	return i;
#endif
}

const String& File::WorkingDirectory()
{
	static String wd;
	static bool b = GetWorkingDirectory(wd);

	return wd;
}

bool File::Delete(const String& path)
{
	return remove(path.Data()) == 0;
}

bool File::Exists(const String& path)
{
	FileStats stats;
	return _stat64(path.Data(), (struct _stat64*)&stats) == 0;
}