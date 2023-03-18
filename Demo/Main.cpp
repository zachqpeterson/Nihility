#include "Engine.hpp"

bool init()
{
	return true;
}

void update()
{
	
}

void shutdown()
{

}

int main()
{
	Engine::Initialize("Nihility Demo", init, update, shutdown);

	return 0;
}