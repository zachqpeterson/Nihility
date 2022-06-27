#include "Resources.hpp"

#include "Shader.hpp"

#include "Memory/Memory.hpp"
#include "Core/File.hpp"
#include "Renderer/RendererFrontend.hpp"
#include "Containers/List.hpp"

#undef LoadImage

HashMap<String, Texture*> Resources::textures;
Texture* Resources::defaultTexture;
Texture* Resources::defaultDiffuse;
Texture* Resources::defaultSpecular;
Texture* Resources::defaultNormal;

Vector<Renderpass*> Resources::renderpasses;

Vector<Shader*> Resources::shaders;
Shader* Resources::defaultMaterialShader;

Vector<Material*> Resources::materials;
Material* Resources::defaultMaterial;

HashMap<String, Mesh*> Resources::meshes;
Mesh* Resources::cubeMesh;
Mesh* Resources::sphereMesh;
Mesh* Resources::capsuleMesh;
Mesh* Resources::quadMesh;

HashMap<String, Model2*> Resources::models2D;
HashMap<String, Model3*> Resources::models3D;

#define BINARIES_PATH "../assets/"
#define TEXTURES_PATH "../assets/textures/"
#define SHADERS_PATH "../assets/shaders/"
#define MATERIALS_PATH "../assets/materials/"
#define MODELS_PATH "../assets/models/"

#define DEFAULT_TEXTURE_NAME "Default.bmp"
#define DEFAULT_DIFFUSE_TEXTURE_NAME "DefaultDiffuse.bmp"
#define DEFAULT_SPECULAR_TEXTURE_NAME "DefaultSpecular.bmp"
#define DEFAULT_NORMAL_TEXTURE_NAME "DefaultNormal.bmp"

#define DEFAULT_MATERIAL_RENDERPASS_NAME "Material.rnp"

#define DEFAULT_MATERIAL_SHADER_NAME "Material.shd"
#define DEFAULT_UI_SHADER_NAME "UI.shd"

#define DEFAULT_MATERIAL_NAME "Default.mat"

#define DEFAULT_MESH_NAME "Default.msh"
#define DEFAULT_MESH2D_NAME "Default2D.msh"

#define RED_MASK    0b11100000
#define GREEN_MASK  0b00011100
#define BLUE_MASK   0b00000011

#pragma pack(push, 1)
struct BMPInfo
{
    U32 biSize;
    I32 biWidth;
    I32 biHeight;
    U16 biPlanes;
    U16 biBitCount;
    U32 biCompression;
    U32 biSizeImage;
    I32 biXPelsPerMeter;
    I32 biYPelsPerMeter;
    U32 biClrUsed;
    U32 biClrImportant;
};

struct BMPHeader
{
    U16 bfType;
    U32 bfSize;
    U16 bfReserved1;
    U16 bfReserved2;
    U32 bfOffBits;
};

struct TGAHeader
{
    U8  idlength;
    U8  colourmaptype;
    U8  datatypecode;
    I16 colourmaporigin;
    I16 colourmaplength;
    U8  colourmapdepth;
    I16 xOrigin;
    I16 yOrigin;
    I16 width;
    I16 height;
    U8  bitsperpixel;
    U8  imagedescriptor;
};
#pragma pack(pop)

bool Resources::Initialize()
{
    Texture* invalidTexture = (Texture*)Memory::Allocate(sizeof(Texture), MEMORY_TAG_RESOURCE);
    invalidTexture->name = "";
    textures = Move(HashMap<String, Texture*>(10, invalidTexture)); //TODO: Config

    Mesh* invalidMesh = (Mesh*)Memory::Allocate(sizeof(Mesh), MEMORY_TAG_RESOURCE);
    invalidMesh->name = "";
    meshes = Move(HashMap<String, Mesh*>(10, invalidMesh)); //TODO: Config

    defaultTexture = LoadTexture(DEFAULT_TEXTURE_NAME);
    defaultDiffuse = LoadTexture(DEFAULT_DIFFUSE_TEXTURE_NAME);
    defaultSpecular = LoadTexture(DEFAULT_SPECULAR_TEXTURE_NAME);
    defaultNormal = LoadTexture(DEFAULT_NORMAL_TEXTURE_NAME);

    defaultMaterialShader = LoadShader(DEFAULT_MATERIAL_SHADER_NAME);
    //defaultUiShader = LoadShader(DEFAULT_UI_SHADER_NAME);

    defaultMaterial = LoadMaterial(DEFAULT_MATERIAL_NAME);

    //TODO: Temporary
    LoadMaterial("Background.mat");
    LoadMaterial("Tile.mat");

    return true;
}

void Resources::Shutdown()
{
    for (Material* m : materials)
    {
        DestroyMaterial(m);
        Memory::Free(m, sizeof(Material), MEMORY_TAG_RESOURCE);
    }

    materials.Destroy();

    for (Shader* s : shaders)
    {
        s->Destroy();
        Memory::Free(s, sizeof(Shader), MEMORY_TAG_RESOURCE);
    }

    shaders.Destroy();

    for (List<HashMap<String, Texture*>::Node>& l : textures)
    {
        for (HashMap<String, Texture*>::Node& n : l)
        {
            n.key.Destroy();
            DestroyTexture(n.value);
            Memory::Free(n.value, sizeof(Texture), MEMORY_TAG_RESOURCE);
        }

        l.Clear();
    }

    textures.Destroy();
}

Binary* Resources::LoadBinary(const String& name)
{
    Logger::Info("Loading binary '{}'...", name);

    String path(BINARIES_PATH);
    path.Append(name);

    File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
    if (file->Open(path, FILE_MODE_READ, true))
    {
        Binary* binary = (Binary*)Memory::Allocate(sizeof(Binary), MEMORY_TAG_RESOURCE);
        binary->name = name;

        U64 size = file->Size();
        binary->data.SetArray(file->ReadAllBytes(size), size);
        file->Close();
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);

        return binary;
    }
    else
    {
        Logger::Error("Couldn't open file: {}", name);
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
    }

    return nullptr;
}

void Resources::UnloadBinary(Binary* binary)
{
    binary->name.Destroy();
    binary->data.Destroy();
    binary = nullptr;
}

Image* Resources::LoadImage(const String& name, ImageType type)
{
    String path(TEXTURES_PATH);
    path.Append(name);

    File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
    if (file->Open(path, FILE_MODE_READ, true))
    {
        Image* resource = (Image*)Memory::Allocate(sizeof(Image), MEMORY_TAG_RESOURCE);
        resource->name = name;

        bool result;
        switch (type)
        {
        case IMAGE_TYPE_BMP: { result = LoadBMP(resource, file); } break;
        case IMAGE_TYPE_PNG: { result = LoadPNG(resource, file); } break;
        case IMAGE_TYPE_JPG: { result = LoadJPG(resource, file); } break;
        case IMAGE_TYPE_TGA: { result = LoadTGA(resource, file); } break;
        }

        file->Close();
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);

        if (!result)
        {
            resource->name.Destroy();
            Memory::Free(resource, sizeof(Image), MEMORY_TAG_RESOURCE);
            return nullptr;
        }

        return resource;
    }
    else
    {
        Logger::Error("Couldn't open file: {}", name);
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
    }

    return nullptr;
}

void Resources::UnloadImage(Image* resource)
{
    resource->name.Destroy();
    resource->pixels.Destroy();
    Memory::Free(resource, sizeof(Image), MEMORY_TAG_RESOURCE);
    resource = nullptr;
}

bool Resources::LoadBMP(Image* image, File* file)
{
    BMPHeader* header = (BMPHeader*)file->ReadBytes(sizeof(BMPHeader));

    if (header->bfType != 0x4D42)
    {
        Logger::Error("Image file: '{}' is not a BMP!", image->name);
        Memory::Free(header, sizeof(BMPHeader), MEMORY_TAG_RESOURCE);
        file->Close();
        return false;
    }

    BMPInfo* info = (BMPInfo*)file->ReadBytes(sizeof(BMPInfo));
    image->width = info->biWidth;
    image->height = info->biHeight;

    file->Seek(header->bfOffBits);
    image->pixels.SetArray(file->ReadBytes(info->biSizeImage), info->biSizeImage);

    Memory::Free(header, sizeof(BMPHeader), MEMORY_TAG_RESOURCE);
    Memory::Free(info, sizeof(BMPInfo), MEMORY_TAG_RESOURCE);

    if (info->biBitCount == 32)
    {
        image->channelCount = 4;
        U8 temp;
        for (auto it0 = image->pixels.begin(), it1 = it0 + 2; it0 != image->pixels.end(); it0 += 4, it1 += 4)
        {
            temp = *it0;
            *it0 = *it1;
            *it1 = temp;
        }
    }
    else if (info->biBitCount == 24)
    {
        image->channelCount = 4;
        Vector<U8> pixels(info->biWidth * info->biHeight * 4);

        for (auto it = image->pixels.begin(); it != image->pixels.end(); ++it)
        {
            pixels.Push(*it);
            pixels.Push(*(++it));
            pixels.Push(*(++it));
            pixels.Push(255);
        }

        image->pixels = Move(pixels);
    }
    else if (info->biBitCount == 8)
    {
        image->channelCount = 4;
        Vector<U8> pixels(info->biWidth * info->biHeight * 4);

        for (U8 p : image->pixels)
        {
            pixels.Push((p & RED_MASK) * 36);
            pixels.Push((p & GREEN_MASK) * 36);
            pixels.Push((p & BLUE_MASK) * 85);
            pixels.Push(255);
        }

        image->pixels = Move(pixels);
    }
    else if (info->biBitCount == 4)
    {
        image->channelCount = 4;
        //TODO:
    }
    else
    {
        image->channelCount = 4;
        //TODO:
    }

    return true;
}

bool Resources::LoadPNG(Image* image, File* file)
{
    Logger::Warn("The PNG file format is not supported.");
    return false;
}

bool Resources::LoadJPG(Image* image, File* file)
{
    Logger::Warn("The JPG file format is not supported.");
    return false;
}

bool Resources::LoadTGA(Image* image, File* file)
{
    TGAHeader* header = (TGAHeader*)file->ReadBytes(sizeof(TGAHeader));

    if (!header)
    {
        Logger::Error("Image file: '{}' is not a TGA!", image->name);
        file->Close();
        return false;
    }

    Logger::Warn("The TGA file format is not supported.");

    image->width = header->width;
    image->height = header->height;

    Memory::Free(header, sizeof(TGAHeader), MEMORY_TAG_RESOURCE);

    return false;
}

Texture* Resources::LoadTexture(const String& name)
{
    if (name.Blank())
    {
        Logger::Error("Texture name can not be blank or nullptr!");
        return nullptr;
    }

    Texture* texture = textures[name];

    if (!texture->name.Blank()) { return texture; }

    Logger::Info("Loading texture '{}'...", name);

    Image* image = LoadImage(name, IMAGE_TYPE_BMP); //TODO: Don't hardcode this

    if (image)
    {
        texture = (Texture*)Memory::Allocate(sizeof(Texture), MEMORY_TAG_RESOURCE);

        texture->name = image->name;
        texture->width = image->width;
        texture->height = image->height;
        texture->generation = 0;
        texture->channelCount = image->channelCount;
        texture->flags = TEXTURE_FLAG_HAS_TRANSPARENCY;

        RendererFrontend::CreateTexture(texture, image->pixels);

        UnloadImage(image);
        textures.Insert(name, texture);
    }

    return texture;
}

void Resources::DestroyTexture(Texture* texture)
{
    RendererFrontend::DestroyTexture(texture);
    texture->name.Destroy();
    Memory::Free(texture, sizeof(Texture), MEMORY_TAG_RESOURCE);
}

Renderpass* Resources::LoadRenderpass(const String& name)
{
    if (name.Blank())
    {
        Logger::Error("Renderpass name can not be blank or nullptr!");
        return nullptr;
    }

    Logger::Info("Loading renderpass '{}'...", name);

    String path(SHADERS_PATH);
    path.Append(name);

    File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
    if (file->Open(path, FILE_MODE_READ, true))
    {
        Renderpass* renderpass = (Renderpass*)Memory::Allocate(sizeof(Renderpass), MEMORY_TAG_RESOURCE);
        renderpass->name = name;

        String line;
        U32 lineNumber = 1;
        while (file->ReadLine(line))
        {
            line.Trim();

            if (line.Length() < 1 || line[0] == '#')
            {
                ++lineNumber;
                continue;
            }

            I32 equalIndex = line.IndexOf('=');
            if (equalIndex == -1)
            {
                Logger::Warn("Potential formatting issue found in file '{}': '=' token not found. Skipping line {}...", path, lineNumber);
                ++lineNumber;
                continue;
            }

            String varName(Move(line.SubString(0, equalIndex)));
            varName.Trim();
            String varValue(Move(line.SubString(equalIndex + 1)));
            varValue.Trim();

            //TODO: Defaults
            if (varName == "clearColor") { renderpass->clearColor = Vector4(varValue); }
            else if (varName == "renderArea") { renderpass->renderArea = Vector4(varValue); }
            else if (varName == "depth") { renderpass->depth = varValue.ToF32(); }
            else if (varName == "stencil") { renderpass->stencil = varValue.ToU32(); }
            else if (varName == "clearFlags")
            {
                Vector<String> flags = Move(varValue.Split(',', true));

                for (String& flag : flags)
                {
                    if (flag == "COLOR_BUFFER") { renderpass->clearFlags |= RENDERPASS_CLEAR_COLOR_BUFFER_FLAG; }
                    else if (flag == "DEPTH_BUFFER") { renderpass->clearFlags |= RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG; }
                    else if (flag == "STENCIL_BUFFER") { renderpass->clearFlags |= RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG; }
                    else if (flag == "NONE") { renderpass->clearFlags = 0; break; }
                    else { Logger::Error("LoadRenderpass: Unrecognized clear flag '{}'. Skipping...", flag); }
                }
            }

            ++lineNumber;
        }

        file->Close();
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);

        renderpass->targets.Resize(RendererFrontend::WindowRenderTargetCount());

        renderpasses.Push(renderpass);

        return renderpass;
    }
    else
    {
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
        path.Destroy();
    }

    return nullptr;
}

void Resources::DestroyRenderpass(Renderpass* renderpass)
{
    RendererFrontend::DestroyRenderpass(renderpass);
    renderpass->name.Destroy();
    Memory::Free(renderpass, sizeof(Renderpass), MEMORY_TAG_RESOURCE);
}

Shader* Resources::LoadShader(const String& name)
{
    if (name.Blank())
    {
        Logger::Error("Shader name can not be blank or nullptr!");
        return nullptr;
    }

    Shader* shader;

    for (Shader* s : shaders)
    {
        if (s->name == name)
        {
            shader = s;
            break;
        }
    }

    if (shader) { return shader; }

    Logger::Info("Loading shader '{}'...", name);

    String path(SHADERS_PATH);
    path.Append(name);

    File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
    if (file->Open(path, FILE_MODE_READ, true))
    {
        shader = (Shader*)Memory::Allocate(sizeof(Shader), MEMORY_TAG_RESOURCE);
        shader->name = name;

        String renderpassName;

        String line;
        U32 lineNumber = 1;
        while (file->ReadLine(line, 511))
        {
            line.Trim();

            if (line.Length() < 1 || line[0] == '#')
            {
                ++lineNumber;
                line.Destroy();
                continue;
            }

            I32 equalIndex = line.IndexOf('=');
            if (equalIndex == -1)
            {
                Logger::Warn("Potential formatting issue found in shader '{}': '=' token not found. Skipping line {}...", shader->name, lineNumber);
                ++lineNumber;
                line.Destroy();
                continue;
            }

            String varName(Move(line.SubString(0, equalIndex)));
            varName.Trim();
            String varValue(Move(line.SubString(equalIndex + 1)));
            varValue.Trim();

            //TODO: Version
            if (varName == "stagefiles") { shader->stageFilenames = Move(varValue.Split(',', true)); }
            else if (varName == "renderpass") { renderpassName = Move(varValue); }
            else if (varName == "useInstance") { shader->useInstances = varValue.ToBool(); }
            else if (varName == "useLocal") { shader->useLocals = varValue.ToBool(); }
            else if (varName == "renderOrder") { shader->renderOrder = varValue.ToI32(); }
            else if (varName == "stages")
            {
                Vector<String> stageNames = Move(varValue.Split(',', true));

                for (String& s : stageNames)
                {
                    if (s == "fragment") { shader->stages.Push(SHADER_STAGE_FRAGMENT); }
                    else if (s == "vertex") { shader->stages.Push(SHADER_STAGE_VERTEX); }
                    else if (s == "geometry") { shader->stages.Push(SHADER_STAGE_GEOMETRY); }
                    else if (s == "compute") { shader->stages.Push(SHADER_STAGE_COMPUTE); }
                    else { Logger::Error("LoadShader: Unrecognized stage '{}'. Skipping...", s); }
                }

                for (String& s : stageNames) { s.Destroy(); }
            }
            else if (varName == "attribute")
            {
                Vector<String> fields(Move(varValue.Split(',', true)));

                if (fields.Size() != 2) { Logger::Error("LoadShader: Attribute fields must be 'type,name'. Skipping..."); }
                else
                {
                    Attribute attribute;
                    GetConfigType(fields.Front(), attribute.type, attribute.size);
                    attribute.name = fields.Back();
                    shader->AddAttribute(attribute);
                }

                for (String& s : fields) { s.Destroy(); }
            }
            else if (varName == "uniform")
            {
                Vector<String> fields(Move(varValue.Split(',', true)));

                if (fields.Size() != 4) { Logger::Error("LoadShader: Invalid file layout. Uniform fields must be 'type,set,binding,name'. Skipping..."); }
                else
                {
                    Uniform uniform;
                    GetConfigType(fields.Front(), uniform.type, uniform.size);

                    uniform.name = fields[3];
                    uniform.setIndex = fields[1].ToU8();
                    uniform.bindingIndex = fields[2].ToU8();

                    shader->AddUniform(uniform);
                }

                for (String& s : fields) { s.Destroy(); }
            }
            else if (varName == "push")
            {
                Vector<String> fields(Move(varValue.Split(',', true)));

                if (fields.Size() != 2) { Logger::Error("LoadShader: Push constant fields must be 'type,name'. Skipping..."); }
                else
                {
                    PushConstant pushConstant;
                    GetConfigType(fields.Front(), pushConstant.type, pushConstant.size);
                    pushConstant.name = fields.Back();
                    shader->AddPushConstant(pushConstant);
                }

                for (String& s : fields) { s.Destroy(); }
            }

            ++lineNumber;
            line.Destroy();
        }

        file->Close();
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);

        if (renderpassName.Blank())
        {
            Logger::Error("LoadShader: Shader '{}' must have a renderpass!", name);
            shader->Destroy();
            Memory::Free(shader, sizeof(Shader), MEMORY_TAG_RESOURCE);
            return nullptr;
        }

        Renderpass* renderpass = LoadRenderpass(renderpassName);

        if (!renderpass)
        {
            Logger::Error("LoadShader: Shader '{}' must have a valid renderpass, name provided: {}!", name, renderpassName);
            shader->Destroy();
            Memory::Free(shader, sizeof(Shader), MEMORY_TAG_RESOURCE);
            return nullptr;
        }

        shader->renderpass = renderpass;

        bool found = false;
        for (U32 i = 0; i < shaders.Size(); ++i)
        {
            if (shader->renderOrder <= shaders[i]->renderOrder)
            {
                shaders.Insert(shader, i);
                found = true;
                break;
            }
        }

        if (!found)
        {
            shaders.Push(shader);
        }
    }
    else
    {
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
    }

    return shader;
}

void Resources::GetConfigType(const String& field, FieldType& type, U16& size)
{
    //TODO: Use HashMap and switch
    if (field == "F32")
    {
        type = FIELD_TYPE_FLOAT32;
        size = 4;
    }
    else if (field == "vec2")
    {
        type = FIELD_TYPE_FLOAT32_2;
        size = 8;
    }
    else if (field == "vec3")
    {
        type = FIELD_TYPE_FLOAT32_3;
        size = 12;
    }
    else if (field == "vec4")
    {
        type = FIELD_TYPE_FLOAT32_4;
        size = 16;
    }
    else if (field == "U8")
    {
        type = FIELD_TYPE_UINT8;
        size = 1;
    }
    else if (field == "U16")
    {
        type = FIELD_TYPE_UINT16;
        size = 2;
    }
    else if (field == "U32")
    {
        type = FIELD_TYPE_UINT32;
        size = 4;
    }
    else if (field == "I8")
    {
        type = FIELD_TYPE_INT8;
        size = 1;
    }
    else if (field == "I16")
    {
        type = FIELD_TYPE_INT16;
        size = 2;
    }
    else if (field == "I32")
    {
        type = FIELD_TYPE_INT32;
        size = 4;
    }
    else if (field == "mat4")
    {
        type = FIELD_TYPE_MATRIX_4;
        size = 64;
    }
    else if (field == "samp" || field == "sampler")
    {
        type = FIELD_TYPE_SAMPLER;
        size = 0;
    }
    else
    {
        Logger::Error("LoadShader: Invalid file layout. Uniform type must be F32, vec2, vec3, vec4, I8, I16, I32, U8, U16, U32 or mat4.");
        Logger::Warn("Defaulting to F32.");
        type = FIELD_TYPE_FLOAT32;
        size = 4;
    }
}

void Resources::CreateShaders()
{
    bool first = true;
    bool last = false;
    for (auto it = shaders.begin(); it != shaders.end(); ++it)
    {
        Shader* shader = *it;

        last = (it + 1) == shaders.end();
        RendererFrontend::CreateRenderpass(shader->renderpass, !first, !last);

        if (!RendererFrontend::CreateShader(shader))
        {
            Logger::Error("LoadShader: Error creating shader '{}'!", shader->name);
            shader->Destroy();
            Memory::Free(shader, sizeof(Shader), MEMORY_TAG_RESOURCE);
        }

        if (!RendererFrontend::InitializeShader(shader))
        {
            Logger::Error("LoadShader: initialization failed for shader '{}'...", shader->name);
            shader->Destroy();
            Memory::Free(shader, sizeof(Shader), MEMORY_TAG_RESOURCE);
        }

        for (String& s : shader->stageFilenames) { s.Destroy(); }

        first = false;
    }

    for (U32 i = 0; i < materials.Size(); ++i)
    {
        materials[i]->id = i;

        if (materials[i]->shader->useInstances)
        {
            Vector<TextureMap> maps;
            maps.Resize(3);
            maps[0] = materials[i]->diffuseMap;
            maps[1] = materials[i]->specularMap;
            maps[2] = materials[i]->normalMap;
            materials[i]->internalId = RendererFrontend::AcquireInstanceResources(materials[i]->shader, maps);
        }
    }
}

Material* Resources::LoadMaterial(const String& name)
{
    if (name.Blank())
    {
        Logger::Error("Material name can not be blank or nullptr!");
        return nullptr;
    }

    Material* material;

    for (Material* m : materials)
    {
        if (m->name == name)
        {
            material = m;
            break;
        }
    }

    if (material) { return material; }

    Logger::Info("Loading material '{}'...", name);

    String path(MATERIALS_PATH);
    path.Append(name);

    File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
    if (file->Open(path, FILE_MODE_READ, true))
    {
        material = (Material*)Memory::Allocate(sizeof(Material), MEMORY_TAG_RESOURCE);
        material->name = name;

        MaterialConfig materialConfig;
        materialConfig.autoRelease = true;

        String line;
        U32 lineNumber = 1;
        while (file->ReadLine(line, 511))
        {
            line.Trim();

            if (line.Length() < 1 || line[0] == '#')
            {
                ++lineNumber;
                continue;
            }

            I32 equalIndex = line.IndexOf('=');
            if (equalIndex == -1)
            {
                Logger::Warn("LoadMaterial('{}'): Potential formatting issue found in file '{}': '=' token not found. Skipping line {}.", name, path, lineNumber);
                ++lineNumber;
                continue;
            }

            String varName(Move(line.SubString(0, equalIndex).Trim()));

            String varValue(Move(line.SubString(equalIndex + 1).Trim()));

            //TODO: Use HashTable with switch statement
            if (varName == "diffuseMap") { materialConfig.diffuseMapName = Move(varValue); }
            else if (varName == "specularMap") { materialConfig.specularMapName = Move(varValue); }
            else if (varName == "normalMap") { materialConfig.normalMapName = Move(varValue); }
            else if (varName == "color") { materialConfig.diffuseColor = Vector4(varValue); }
            else if (varName == "shader") { materialConfig.shaderName = Move(varValue); }
            else if (varName == "shininess") { materialConfig.shininess = varValue.ToF32(); }

            ++lineNumber;
        }

        file->Close();

        if (materialConfig.shaderName.Blank()) { materialConfig.shaderName = DEFAULT_MATERIAL_SHADER_NAME; }

        Shader* shader = LoadShader(materialConfig.shaderName);

        if (!shader)
        {
            Logger::Warn("Couldn't find shader '{}', using default instead", materialConfig.shaderName);
            shader = DefaultMaterialShader();
        }

        CreateMaterial(materialConfig, material);

        material->shader = shader;

        bool found = false;
        for (U32 i = 0; i < materials.Size(); ++i)
        {
            if (shader->renderOrder <= materials[i]->shader->renderOrder)
            {
                materials.Insert(material, i);
                found = true;
                break;
            }
        }

        if (!found)
        {
            materials.Push(material);
        }

        materialConfig.name.Destroy();
        materialConfig.diffuseMapName.Destroy();
        materialConfig.specularMapName.Destroy();
        materialConfig.normalMapName.Destroy();
        materialConfig.shaderName.Destroy();
    }
    else
    {
        path.Destroy();
        return nullptr;
    }

    Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);

    return material;
}

void Resources::CreateMaterial(MaterialConfig& config, Material* material)
{
    material->diffuseColor = config.diffuseColor;
    material->shininess = config.shininess;

    material->diffuseMap.use = TEXTURE_USE_MAP_DIFFUSE;
    material->diffuseMap.texture = config.diffuseMapName.Length() > 0 ? LoadTexture(config.diffuseMapName) : defaultDiffuse;
    material->diffuseMap.filterMinify = TEXTURE_FILTER_MODE_NEAREST;
    material->diffuseMap.filterMagnify = TEXTURE_FILTER_MODE_NEAREST;
    material->diffuseMap.repeatU = TEXTURE_REPEAT_REPEAT;
    material->diffuseMap.repeatV = TEXTURE_REPEAT_REPEAT;
    material->diffuseMap.repeatW = TEXTURE_REPEAT_REPEAT;
    if (!RendererFrontend::AcquireTextureMapResources(material->diffuseMap))
    {
        Logger::Error("LoadMaterial: Error loading TextureMap resources");
        return;
    }

    material->specularMap.use = TEXTURE_USE_MAP_SPECULAR;
    material->specularMap.texture = config.specularMapName.Length() > 0 ? LoadTexture(config.specularMapName) : defaultSpecular;
    material->specularMap.filterMinify = TEXTURE_FILTER_MODE_NEAREST;
    material->specularMap.filterMagnify = TEXTURE_FILTER_MODE_NEAREST;
    material->specularMap.repeatU = TEXTURE_REPEAT_REPEAT;
    material->specularMap.repeatV = TEXTURE_REPEAT_REPEAT;
    material->specularMap.repeatW = TEXTURE_REPEAT_REPEAT;
    if (!RendererFrontend::AcquireTextureMapResources(material->specularMap))
    {
        Logger::Error("LoadMaterial: Error loading TextureMap resources");
        return;
    }

    material->normalMap.use = TEXTURE_USE_MAP_NORMAL;
    material->normalMap.texture = config.normalMapName.Length() > 0 ? LoadTexture(config.normalMapName) : defaultNormal;
    material->normalMap.filterMinify = TEXTURE_FILTER_MODE_NEAREST;
    material->normalMap.filterMagnify = TEXTURE_FILTER_MODE_NEAREST;
    material->normalMap.repeatU = TEXTURE_REPEAT_REPEAT;
    material->normalMap.repeatV = TEXTURE_REPEAT_REPEAT;
    material->normalMap.repeatW = TEXTURE_REPEAT_REPEAT;
    if (!RendererFrontend::AcquireTextureMapResources(material->normalMap))
    {
        Logger::Error("LoadMaterial: Error loading TextureMap resources");
        return;
    }
}

void Resources::DestroyMaterial(Material* material)
{
    material->name.Destroy();
    RendererFrontend::ReleaseTextureMapResources(material->diffuseMap);
    material->diffuseMap.texture = nullptr;
    RendererFrontend::ReleaseTextureMapResources(material->specularMap);
    material->specularMap.texture = nullptr;
    RendererFrontend::ReleaseTextureMapResources(material->normalMap);
    material->normalMap.texture = nullptr;
    Memory::Free(material, sizeof(Material), MEMORY_TAG_RESOURCE);
}

Mesh* Resources::LoadMesh(const String& name)
{
    if (name.Blank())
    {
        Logger::Error("Mesh name can not be blank or nullptr!");
        return nullptr;
    }

    Mesh* mesh = meshes[name];

    if (!mesh->name.Blank()) { return mesh; }

    Logger::Info("Loading mesh '{}'...", name);

    String path(MODELS_PATH);
    path.Append(name);

    File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
    if (file->Open(path, FILE_MODE_READ, true))
    {
        mesh = (Mesh*)Memory::Allocate(sizeof(Mesh), MEMORY_TAG_RESOURCE);
        mesh->name = name;



        file->Close();
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
    }
    else
    {
        file->Close();
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
    }

    return nullptr;
}

Mesh* Resources::CreateMesh(MeshConfig& config)
{
    if (config.vertices.Size() < 3)
    {
        Logger::Error("Create mesh requires at least three vertices, vertex count provided: {}", config.vertices.Size());
        return nullptr;
    }

    if (config.name.Blank())
    {
        Logger::Error("Mesh name can not be blank or nullptr!");
        return nullptr;
    }

    Mesh* mesh = meshes[config.name];

    if (mesh->name.Blank())
    {
        Logger::Info("Creating mesh '{}'...", config.name);
        mesh = (Mesh*)Memory::Allocate(sizeof(Mesh), MEMORY_TAG_RESOURCE);
        mesh->name = config.name;
    }

    if (!RendererFrontend::CreateMesh(mesh, config.vertices, config.indices))
    {
        Logger::Error("Failed to create mesh '{}'", config.name);
        Memory::Free(mesh, sizeof(Mesh), MEMORY_TAG_RESOURCE);
        return nullptr;
    }

    if (config.MaterialName.Blank())
    {
        mesh->material = DefaultMaterial();
    }
    else
    {
        mesh->material = LoadMaterial(config.MaterialName);
    }

    meshes.Insert(config.name, mesh);

    return mesh;
}

Model2* Resources::LoadModel2D(const String& name)
{
    Logger::Info("Loading model '{}'...", name);

    if (name.Blank())
    {
        Logger::Error("Model name can not be blank or nullptr!");
        return nullptr;
    }

    return nullptr;
}

Model3* Resources::LoadModel3D(const String& name)
{
    Logger::Info("Loading model '{}'...", name);

    if (name.Blank())
    {
        Logger::Error("Model name can not be blank or nullptr!");
        return nullptr;
    }

    return nullptr;
}

Texture* Resources::CreateWritableTexture(const String& name, U32 width, U32 height, U8 channelCount, bool hasTransparency)
{
    Logger::Info("Creating texture '{}'...", name);

    if (name.Blank())
    {
        Logger::Error("Texture name can not be blank or nullptr!");
        return nullptr;
    }

    Texture* texture = textures[name];

    if (!texture->name.Blank())
    {
        Logger::Error("Texture named '{}' already exists!", name);
        return nullptr;
    }

    texture = (Texture*)Memory::Allocate(sizeof(texture), MEMORY_TAG_RESOURCE);

    texture->name = name;
    texture->width = width;
    texture->height = height;
    texture->generation = 0;
    texture->channelCount = channelCount;
    texture->flags |= hasTransparency ? TEXTURE_FLAG_HAS_TRANSPARENCY : 0;
    texture->flags |= TEXTURE_FLAG_IS_WRITEABLE;

    if (!RendererFrontend::CreateWritableTexture(texture))
    {
        Logger::Error("Failed to create writable texture '{}'!", name);
        Memory::Free(texture, sizeof(texture), MEMORY_TAG_RESOURCE);
        return nullptr;
    }

    textures.Insert(name, texture);

    return texture;
}

Texture* Resources::CreateTextureFromInternal(const String& name, U32 width, U32 height, U8 channelCount, bool hasTransparency, bool isWriteable, bool registerTexture, void* internalData)
{
    Logger::Info("Creating texture '{}' from internal data...", name);

    if (registerTexture)
    {
        if (name.Blank())
        {
            Logger::Error("Texture name can not be blank or nullptr!");
            return nullptr;
        }

        Texture* texture = textures[name];

        if (!texture->name.Blank())
        {
            Logger::Error("Texture named '{}' already exists!", name);
            return nullptr;
        }
    }

    if (!internalData)
    {
        Logger::Error("internalData must not be nullptr!");
        return nullptr;
    }

    Texture* texture = (Texture*)Memory::Allocate(sizeof(Texture), MEMORY_TAG_RESOURCE);

    texture->name = name;
    texture->width = width;
    texture->height = height;
    texture->generation = 0;
    texture->channelCount = channelCount;
    texture->flags |= hasTransparency ? TEXTURE_FLAG_HAS_TRANSPARENCY : 0;
    texture->flags |= isWriteable ? TEXTURE_FLAG_IS_WRITEABLE : 0;
    texture->flags |= TEXTURE_FLAG_IS_WRAPPED;
    texture->internalData = internalData;

    if (registerTexture) { textures.Insert(name, texture); }

    return texture;
}

bool Resources::SetTextureInternal(Texture* texture, void* internalData)
{
    if (!texture)
    {
        Logger::Error("Texture must not be nullptr!");
        return false;
    }

    if (!internalData)
    {
        Logger::Error("internalData must not be nullptr!");
        return false;
    }

    if (texture->internalData)
    {
        Logger::Warn("internalData in texture '{}' isn't nullptr, overwriting...", texture->name);
    }

    texture->internalData = internalData;
    ++texture->generation;
    return true;
}

bool Resources::ResizeTexture(Texture* texture, U32 width, U32 height, bool regenerateInternalData)
{
    if (!texture)
    {
        Logger::Error("Texture must not be nullptr!");
        return false;
    }

    if (!(texture->flags & TEXTURE_FLAG_IS_WRITEABLE))
    {
        Logger::Error("Can't resize texture not marked with TEXTURE_FLAG_IS_WRITEABLE");
        return false;
    }

    texture->width = width;
    texture->height = height;

    if (!(texture->flags & TEXTURE_FLAG_IS_WRAPPED) && regenerateInternalData)
    {
        RendererFrontend::ResizeTexture(texture, width, height);
        return false;
    }

    ++texture->generation;
    return true;
}
