#include "Resources.hpp"

#include "Shader.hpp"

#include "Memory/Memory.hpp"
#include "Core/File.hpp"
#include "Renderer/RendererFrontend.hpp"
#include "Containers/List.hpp"
#include "Core/Settings.hpp"

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

HashMap<String, Model*> Resources::models;

#define BINARIES_PATH "../assets/"
#define TEXTURES_PATH "../assets/textures/"
#define SHADERS_PATH "../assets/shaders/"
#define MATERIALS_PATH "../assets/materials/"
#define MODELS_PATH "../assets/models/"
#define CONFIG_PATH "../assets/config/"

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

#define BYTECAST(x) ((U8)((x) & 255))

#pragma pack(push, 1)
struct BMPHeader
{
    U16 signature;
    U32 fileSize;
    U16 reserved1;
    U16 reserved2;
    U32 imageOffset;
};

struct BMPInfo
{
    U32 infoSize;
    I32 imageWidth;
    I32 imageHeight;
    U16 imagePlanes;
    U16 imageBitCount;
    U32 imageCompression;
    U32 imageSize;
    I32 biXPelsPerMeter;
    I32 biYPelsPerMeter;
    U32 colorsUsed;
    U32 importantColor;
    U32 extraRead;
    U32 redMask;
    U32 greenMask;
    U32 blueMask;
    U32 alphaMask;
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

    Model* invalidModel = (Model*)Memory::Allocate(sizeof(Model), MEMORY_TAG_RESOURCE);
    invalidModel->name = "";
    models = Move(HashMap<String, Model*>(10, invalidModel)); //TODO: Config

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
        }

        l.Clear();
    }

    textures.Destroy();

    for (List<HashMap<String, Mesh*>::Node>& l : meshes)
    {
        for (HashMap<String, Mesh*>::Node& n : l)
        {
            n.key.Destroy();
            DestroyMesh(n.value);
        }

        l.Clear();
    }

    meshes.Destroy();

    for (List<HashMap<String, Model*>::Node>& l : models)
    {
        for (HashMap<String, Model*>::Node& n : l)
        {
            n.key.Destroy();
            n.value->name.Destroy();
            n.value->meshes.Clear();
        }

        l.Clear();
    }

    models.Destroy();
}

void Resources::LoadSettings()
{
    Logger::Info("Loading engine settings...");

    String path = CONFIG_PATH;
    path.Append("Settings.cfg");

    File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
    if (file->Open(path, FILE_MODE_READ, true))
    {
        String line;
        U32 lineNumber = 1;
        while (file->ReadLine(line))
        {
            line.Trim();

            if (line.Blank() || line[0] == '#')
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

            if (varName == "borderless") { Settings::BORDERLESS = varValue.ToBool(); }
            else if (varName == "channels") { Settings::CHANNEL_COUNT = varValue.ToU8(); }
            else if (varName == "master") { Settings::MASTER_VOLUME = varValue.ToF32(); }
            else if (varName == "music") { Settings::MUSIC_VOLUME = varValue.ToF32(); }
            else if (varName == "sfx") { Settings::SFX_VOLUME = varValue.ToF32(); }
            else if (varName == "framerate") 
            { 
                U16 rate = varValue.ToU16();
                if(rate) { Settings::TARGET_FRAMETIME = 1.0 / rate; }
                else { Settings::TARGET_FRAMETIME = 0.0; }
            }
            else if (varName == "resolution")
            {
                Vector<String> dimensions = Move(varValue.Split(',', true));
                if (dimensions.Size() != 2) { Logger::Warn("Settings.cfg: resolution isn't in format width,height, setting to default..."); }
                else
                {
                    Settings::WINDOW_WIDTH = dimensions[0].ToU16();
                    Settings::WINDOW_HEIGHT = dimensions[1].ToU16();
                }
            }
            else if (varName == "position")
            {
                Vector<String> position = Move(varValue.Split(',', true));
                if (position.Size() != 2) { Logger::Warn("Settings.cfg: position isn't in format x,y, setting to default..."); }
                else
                {
                    Settings::WINDOW_POSITION_X = position[0].ToU16();
                    Settings::WINDOW_POSITION_Y = position[1].ToU16();
                }
            }
            else
            {
                Logger::Warn("Unknown setting: {}, skipping...", varName);
            }

            ++lineNumber;
        }

        file->Close();
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
    }
}

void Resources::WriteSettings()
{

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

Image* Resources::LoadImage(const String& name)
{
    String path(TEXTURES_PATH);
    path.Append(name);

    File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
    if (file->Open(path, FILE_MODE_READ, true))
    {
        Image* resource = (Image*)Memory::Allocate(sizeof(Image), MEMORY_TAG_RESOURCE);
        resource->name = name;

        Vector<String> sections = name.Split('.', true);

        bool result;
        if (sections.Back() == "bmp") { result = LoadBMP(resource, file); }
        else if (sections.Back() == "png") { result = LoadPNG(resource, file); }
        else if (sections.Back() == "jpg" || sections.Back() == "jpeg") { result = LoadJPG(resource, file); }
        else if (sections.Back() == "tga") { result = LoadTGA(resource, file); }
        else { Logger::Error("Unkown file extention '{}'", sections.Back()); result = false; }

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
    BMPHeader header;
    BMPInfo info;
    if (!ReadBMPHeader(header, info, file)) { file->Close(); return false; }

    image->width = info.imageWidth;
    image->height = info.imageHeight;
    image->channelCount = 4;
    image->layout = IMAGE_LAYOUT_RGBA32;

    U32 pSize = 0;
    U32 width;
    I32 pad;

    if (info.infoSize == 12) { if (info.imageBitCount < 24) { pSize = (header.imageOffset - info.extraRead - 24) / 3; } }
    else { if (info.imageBitCount < 16) { pSize = (header.imageOffset - info.extraRead - info.infoSize) >> 2; } }

    image->pixels.Reserve(info.imageWidth * info.imageHeight * 4);

    if (info.imageBitCount < 16)
    {
        if (pSize == 0 || pSize > 256)
        {
            Logger::Error("Corrupted BMP!");
            file->Close();
            return false;
        }

        Vector<U8> palette(pSize);

        if (info.infoSize != 12)
        {
            for (U32 i = 0; i < pSize; ++i)
            {
                palette.Push(file->ReadU8());
                palette.Push(file->ReadU8());
                palette.Push(file->ReadU8());
                file->ReadU8();
            }
        }
        else
        {
            for (U32 i = 0; i < pSize; ++i)
            {
                palette.Push(file->ReadU8());
                palette.Push(file->ReadU8());
                palette.Push(file->ReadU8());
            }
        }

        file->Seek(header.imageOffset - info.extraRead - info.infoSize - pSize * (info.infoSize == 12 ? 3 : 4));

        if (info.imageBitCount == 1) { width = (info.imageWidth + 7) >> 3; }
        else if (info.imageBitCount == 4) { width = (info.imageWidth + 1) >> 1; }
        else if (info.imageBitCount == 8) { width = info.imageWidth; }
        else
        {
            Logger::Error("Corrupted BMP!");
            file->Close();
            return false;
        }

        pad = (-width) & 3;

        switch (info.imageBitCount)
        {
        case 1:
        {
            for (U32 j = 0; j < info.imageHeight; ++j)
            {
                I8 bitOffset = 7;
                U8 v = file->ReadU8();
                for (U32 i = 0; i < info.imageWidth; ++i)
                {
                    U8 color = (v >> bitOffset) & 0x1;
                    image->pixels.Push(palette[color * 3]);
                    image->pixels.Push(palette[color * 3 + 1]);
                    image->pixels.Push(palette[color * 3 + 2]);
                    image->pixels.Push(255);
                    if ((--bitOffset) < 0 && i + 1 != info.imageWidth)
                    {
                        bitOffset = 7;
                        v = file->ReadU8();
                    }
                }
                file->Seek(pad);
            }
        } break;
        case 4:
        {
            for (U32 j = 0; j < info.imageHeight; ++j)
            {
                for (U32 i = 0; i < info.imageWidth; i += 2)
                {
                    U8 v = file->ReadU8();
                    U8 v2 = v & 15;
                    v >>= 4;
                    image->pixels.Push(palette[v * 3]);
                    image->pixels.Push(palette[v * 3 + 1]);
                    image->pixels.Push(palette[v * 3 + 2]);
                    image->pixels.Push(255);
                    if (i + 1 >= info.imageWidth) { break; }
                    image->pixels.Push(palette[v2 * 3]);
                    image->pixels.Push(palette[v2 * 3 + 1]);
                    image->pixels.Push(palette[v2 * 3 + 2]);
                    image->pixels.Push(255);
                }
                file->Seek(pad);
            }
        } break;
        case 8:
        {
            for (U32 j = 0; j < info.imageHeight; ++j)
            {
                for (U32 i = 0; i < info.imageWidth; ++i)
                {
                    U8 v = file->ReadU8();
                    image->pixels.Push(palette[v * 3]);
                    image->pixels.Push(palette[v * 3 + 1]);
                    image->pixels.Push(palette[v * 3 + 2]);
                    image->pixels.Push(255);
                }
                file->Seek(pad);
            }
        } break;
        }
    }
    else
    {
        int rshift = 0, gshift = 0, bshift = 0, ashift = 0, rcount = 0, gcount = 0, bcount = 0, acount = 0;
        U8 easy = 0;

        file->Seek(header.imageOffset - info.extraRead - info.infoSize);

        if (info.imageBitCount == 24) { width = 3 * info.imageWidth; }
        else if (info.imageBitCount == 16) { width = 2 * info.imageWidth; }
        else { width = 0; }

        pad = (-width) & 3;

        if (info.imageBitCount == 24) { easy = 1; }
        else if (info.imageBitCount == 32) { if (info.blueMask == 0xff && info.greenMask == 0xff00 && info.redMask == 0x00ff0000 && info.alphaMask == 0xff000000) { easy = 2; } }

        if (!easy)
        {
            if (!info.redMask || !info.greenMask || !info.blueMask)
            {
                Logger::Error("Corrupted BMP!");
                file->Close();
                return false;
            }

            rshift = Memory::HighBit(info.redMask) - 7;
            gshift = Memory::HighBit(info.greenMask) - 7;
            bshift = Memory::HighBit(info.blueMask) - 7;
            ashift = Memory::HighBit(info.alphaMask) - 7;

            rcount = Memory::BitCount(info.redMask);
            gcount = Memory::BitCount(info.greenMask);
            bcount = Memory::BitCount(info.blueMask);
            acount = Memory::BitCount(info.alphaMask);

            if (rcount > 8 || gcount > 8 || bcount > 8 || acount > 8)
            {
                Logger::Error("Corrupted BMP!");
                file->Close();
                return false;
            }
        }

        for (U32 j = 0; j < info.imageHeight; ++j)
        {
            if (easy)
            {
                for (U32 i = 0; i < info.imageWidth; ++i)
                {
                    U8 alpha;
                    U8 blue = file->ReadU8();
                    U8 green = file->ReadU8();
                    image->pixels.Push(file->ReadU8());
                    image->pixels.Push(green);
                    image->pixels.Push(blue);
                    alpha = (easy == 2 ? file->ReadU8() : 255);
                    image->pixels.Push(alpha);
                }
            }
            else
            {
                for (U32 i = 0; i < info.imageWidth; ++i)
                {
                    U32 v = (info.imageBitCount == 16 ? (U32)file->ReadU16() : file->ReadU32());
                    U32 alpha;
                    image->pixels.Push(BYTECAST(Memory::ShiftSigned(v & info.redMask, rshift, rcount)));
                    image->pixels.Push(BYTECAST(Memory::ShiftSigned(v & info.greenMask, gshift, gcount)));
                    image->pixels.Push(BYTECAST(Memory::ShiftSigned(v & info.blueMask, bshift, bcount)));
                    alpha = (info.alphaMask ? Memory::ShiftSigned(v & info.alphaMask, ashift, acount) : 255);
                    image->pixels.Push(BYTECAST(alpha));
                }
            }

            file->Seek(pad);
        }
    }

    return true;
}

bool Resources::ReadBMPHeader(BMPHeader& header, BMPInfo& info, File* file)
{
    header.signature = file->ReadU16();

    if (header.signature != 0x4D42)
    {
        Logger::Error("Image file is not a BMP!");
        file->Close();
        return false;
    }

    header.fileSize = file->ReadU32();
    header.reserved1 = file->ReadU16();
    header.reserved2 = file->ReadU16();
    header.imageOffset = file->ReadU32();

    info.infoSize = file->ReadU32();
    info.extraRead = 14;
    info.redMask = info.greenMask = info.blueMask = info.alphaMask = 0;

    switch (info.infoSize)
    {
    case 12:
    {
        info.imageWidth = file->ReadI16();
        info.imageHeight = file->ReadI16();

        info.imagePlanes = file->ReadU16();
        if (info.imagePlanes != 1) { Logger::Error("Corrupted BMP!"); return false; }

        info.imageBitCount = file->ReadU16();
    } break;

    case 40:
    {
        info.imageWidth = file->ReadI32();
        info.imageHeight = file->ReadI32();

        info.imagePlanes = file->ReadU16();
        if (info.imagePlanes != 1) { Logger::Error("Corrupted BMP!"); return false; }

        info.imageBitCount = file->ReadU16();
        info.imageCompression = file->ReadU32();

        if (info.imageCompression != 0 && (info.imageCompression != 3 || (info.imageBitCount != 16 && info.imageBitCount != 32)))
        {
            Logger::Error("RLE Compressed BMPs not supported!");
            return false;
        }

        info.imageSize = file->ReadU32();
        info.biXPelsPerMeter = file->ReadI32();
        info.biYPelsPerMeter = file->ReadI32();
        info.colorsUsed = file->ReadU32();
        info.importantColor = file->ReadU32();

        if (info.imageBitCount == 16 || info.imageBitCount == 32)
        {
            if (info.imageCompression == 0)
            {
                SetBmpColorMasks(info);
            }
            else if (info.imageCompression == 3)
            {
                info.redMask = file->ReadU32();
                info.greenMask = file->ReadU32();
                info.blueMask = file->ReadU32();
                info.extraRead += 12;

                if (info.redMask == info.greenMask && info.greenMask == info.blueMask) { Logger::Error("Corrupted BMP!"); return false; }
            }
            else { Logger::Error("Corrupted BMP!"); return false; }
        }
    } break;

    case 56:
    {
        info.imageWidth = file->ReadI32();
        info.imageHeight = file->ReadI32();

        info.imagePlanes = file->ReadU16();
        if (info.imagePlanes != 1) { Logger::Error("Corrupted BMP!"); return false; }

        info.imageBitCount = file->ReadU16();
        info.imageCompression = file->ReadU32();
        if (info.imageCompression != 0 && (info.imageCompression != 3 || (info.imageBitCount != 16 && info.imageBitCount != 32)))
        {
            Logger::Error("RLE Compressed BMPs not supported!");
            return false;
        }

        info.imageSize = file->ReadU32();
        info.biXPelsPerMeter = file->ReadI32();
        info.biYPelsPerMeter = file->ReadI32();
        info.colorsUsed = file->ReadU32();
        info.importantColor = file->ReadU32();

        file->ReadU32();
        file->ReadU32();
        file->ReadU32();
        file->ReadU32();

        if (info.imageBitCount == 16 || info.imageBitCount == 32)
        {
            if (info.imageCompression == 0)
            {
                SetBmpColorMasks(info);
            }
            else if (info.imageCompression == 3)
            {
                info.redMask = file->ReadU32();
                info.greenMask = file->ReadU32();
                info.blueMask = file->ReadU32();
                info.extraRead += 12;

                if (info.redMask == info.greenMask && info.greenMask == info.blueMask) { Logger::Error("Corrupted BMP!"); return false; }
            }
            else { Logger::Error("Corrupted BMP!"); return false; }
        }
    } break;

    case 108:
    {
        info.imageWidth = file->ReadI32();
        info.imageHeight = file->ReadI32();

        info.imagePlanes = file->ReadU16();
        if (info.imagePlanes != 1) { Logger::Error("Corrupted BMP!"); return false; }

        info.imageBitCount = file->ReadU16();
        info.imageCompression = file->ReadU32();
        if (info.imageCompression != 0 && (info.imageCompression != 3 || (info.imageBitCount != 16 && info.imageBitCount != 32)))
        {
            Logger::Error("RLE Compressed BMPs not supported!");
            return false;
        }

        info.imageSize = file->ReadU32();
        info.biXPelsPerMeter = file->ReadI32();
        info.biYPelsPerMeter = file->ReadI32();
        info.colorsUsed = file->ReadU32();
        info.importantColor = file->ReadU32();

        info.redMask = file->ReadU32();
        info.greenMask = file->ReadU32();
        info.blueMask = file->ReadU32();
        info.alphaMask = file->ReadU32();

        if (info.imageCompression != 3) { SetBmpColorMasks(info); }

        file->ReadU32(); // discard color space
        for (U32 i = 0; i < 12; ++i) { file->ReadU32(); } // discard color space parameters
    } break;

    case 124:
    {
        info.imageWidth = file->ReadI32();
        info.imageHeight = file->ReadI32();

        info.imagePlanes = file->ReadU16();
        if (info.imagePlanes != 1) { Logger::Error("Corrupted BMP!"); return false; }

        info.imageBitCount = file->ReadU16();
        info.imageCompression = file->ReadU32();
        if (info.imageCompression != 0 && (info.imageCompression != 3 || (info.imageBitCount != 16 && info.imageBitCount != 32)))
        {
            Logger::Error("RLE Compressed BMPs not supported!");
            return false;
        }

        info.imageSize = file->ReadU32();
        info.biXPelsPerMeter = file->ReadI32();
        info.biYPelsPerMeter = file->ReadI32();
        info.colorsUsed = file->ReadU32();
        info.importantColor = file->ReadU32();

        info.redMask = file->ReadU32();
        info.greenMask = file->ReadU32();
        info.blueMask = file->ReadU32();
        info.alphaMask = file->ReadU32();

        if (info.imageCompression != 3) { SetBmpColorMasks(info); }

        file->ReadU32(); // discard color space
        for (U32 i = 0; i < 12; ++i) { file->ReadU32(); } // discard color space parameters

        file->ReadU32(); // discard rendering intent
        file->ReadU32(); // discard offset of profile data
        file->ReadU32(); // discard size of profile data
        file->ReadU32(); // discard reserved
    } break;

    default:
    {
        Logger::Error("Corrupted BMP!");
        file->Close();
    } return false;
    }

    return true;
}

void Resources::SetBmpColorMasks(BMPInfo& info)
{
    if (info.imageCompression == 3) { return; }

    if (info.imageCompression == 0)
    {
        if (info.imageBitCount == 16)
        {
            info.redMask = 0x7C00U;
            info.greenMask = 0x3E0U;
            info.blueMask = 0x1FU;
        }
        else if (info.imageBitCount == 32)
        {
            info.redMask = 0xFF0000U;
            info.greenMask = 0xFF00U;
            info.blueMask = 0xFFU;
            info.alphaMask = 0xFF000000U;
        }
        else
        {
            info.redMask = info.greenMask = info.blueMask = info.alphaMask = 0;
        }
    }
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

    Image* image = LoadImage(name);

    if (image)
    {
        texture = (Texture*)Memory::Allocate(sizeof(Texture), MEMORY_TAG_RESOURCE);

        texture->name = image->name;
        texture->width = image->width;
        texture->height = image->height;
        texture->generation = 0;
        texture->channelCount = image->channelCount;
        texture->layout = image->layout;
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

            if (line.Blank() || line[0] == '#')
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

            if (line.Blank() || line[0] == '#')
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

            if (line.Blank() || line[0] == '#')
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
                materials.Insert(material, i); //TODO: shader name breaks
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
    material->diffuseMap.texture = config.diffuseMapName.Blank() ? defaultDiffuse : LoadTexture(config.diffuseMapName);
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
    material->specularMap.texture = config.specularMapName.Blank() ? defaultSpecular : LoadTexture(config.specularMapName);
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
    material->normalMap.texture = config.normalMapName.Blank() ? defaultNormal : LoadTexture(config.normalMapName);
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

        //Load Msh file

        file->Close();
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);

        meshes.Insert(name, mesh);

        return mesh;
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

void Resources::DestroyMesh(Mesh* mesh)
{
    RendererFrontend::DestroyMesh(mesh);

    mesh->material = nullptr;
    mesh->name.Destroy();
}

Model* Resources::LoadModel(const String& name)
{
    if (name.Blank())
    {
        Logger::Error("Model name can not be blank or nullptr!");
        return nullptr;
    }

    Model* model = models[name];

    if (!model->name.Blank()) { return model; }

    Logger::Info("Loading model '{}'...", name);

    String path(MODELS_PATH);
    path.Append(name);

    File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
    if (file->Open(path, FILE_MODE_READ, true))
    {
        model = (Model*)Memory::Allocate(sizeof(Model), MEMORY_TAG_RESOURCE);
        model->name = name;

        Vector<String> sections = name.Split('.', true);

        if (sections.Back() == "obj") { LoadOBJ(model, file); }
        else if (sections.Back() == "ksm") { LoadKSM(model, file); }
        else
        {
            Logger::Error("Unkown file extention '{}'", sections.Back());
            file->Close();
            Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
            Memory::Free(model, sizeof(Model), MEMORY_TAG_RESOURCE);
            return nullptr;
        }

        file->Close();
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);

        models.Insert(name, model);

        return model;
    }
    else
    {
        file->Close();
        Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
    }

    return nullptr;
}

void Resources::LoadOBJ(Model* mesh, struct File* file)
{

}

void Resources::LoadKSM(Model* mesh, struct File* file)
{

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
