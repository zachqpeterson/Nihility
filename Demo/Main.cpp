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

int main(int argc, char** argv)
{
	Engine::Initialize("Nihility Demo", 1, init, update, shutdown);

	return 0;
}