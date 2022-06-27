#include <Engine.hpp>

#include <Containers/String.hpp>

bool init()
{
    return true;
}

bool update()
{
    return true;
}

void cleanup()
{
    
}

int main()
{
    Engine::Initialize("TestBed", init, update, cleanup);
}