#pragma once

#include "Defines.hpp"

#include "Containers/Vector.hpp"
#include "Containers/Hashtable.hpp"
#include "Resources/Shader.hpp"

class ShaderSystem
{
public:
    static void Initialize();
    static void Shutdown();

    static bool CreateShader(const ShaderConfig& config);
    static U32 GetShaderId(const String& name);
    static Shader& GetShaderById(U32 id);
    static Shader& GetShaderByName(const String& name);

    static void UseShader(const String& name);
    static void UseShader(U32 id);

    static U16 UniformIndex(const Shader& shader, const String& uniformName);
    static bool SetUniform(const String& uniform_name, const void* value);
    static void SetSampler(const String& samplerName, const struct Texture& t);
    static bool SetUniformByIndex(U16 index, const void* value);
    static bool SetSamplerByIndex(U16 index, const Texture& t);

    static bool ApplyGlobal();
    static bool ApplyInstance(bool needsUpdate);
    static bool BindInstance(U32 instanceId);

private:
    ShaderSystem() = delete;

    static U32 NewShaderId();

    static Hashtable<struct String, U32> lookup;
    static Vector<struct Shader> shaders;
    static U32 currentShaderId;
};