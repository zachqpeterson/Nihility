#pragma once

#include "Defines.hpp"

class Renderer
{
public:
    virtual ~Renderer() {};

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

    void operator delete(void* p);
};