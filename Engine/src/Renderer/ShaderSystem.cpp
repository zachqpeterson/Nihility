#include "ShaderSystem.hpp"

struct shader_system_state
{
    ShaderSystemConfig config;
    Hashtable lookup;
    U32 currentShaderId;
    Vector<Shader> shaders;
};