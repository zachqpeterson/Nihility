#include <Engine.hpp>

#include <Containers/String.hpp>
#include <Math/Math.hpp>
#include <Core/Logger.hpp>

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