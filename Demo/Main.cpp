#include "Defines.hpp"

#include "Engine.hpp"

#include <crtdbg.h>

int main()
{
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG | _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);

	Engine::Initialize();

	return 0;
}