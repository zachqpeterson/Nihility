#include "Resources.hpp"

#include "Shader.hpp"

#include "Memory/Memory.hpp"
#include "Core/File.hpp"
#include "Core/Time.hpp"
#include "Renderer/RendererFrontend.hpp"
#include "Containers/Heap.hpp"
#include "Core/Settings.hpp"
#include "Physics/Physics.hpp"

#include <Containers/List.hpp>

#undef LoadImage

HashTable<String, Texture*> Resources::textures;
Texture* Resources::defaultTexture;
Texture* Resources::defaultDiffuse;
Texture* Resources::defaultSpecular;
Texture* Resources::defaultNormal;

HashTable<String, AudioFull*> Resources::audio;

HashTable<String, TTFInfo*> Resources::fonts;

Vector<Renderpass*> Resources::renderpasses;

Vector<Shader*> Resources::shaders;
Shader* Resources::defaultMaterialShader;

Vector<Material*> Resources::materials;
Material* Resources::defaultMaterial;

HashTable<String, Mesh*> Resources::meshes;
Mesh* Resources::cubeMesh;
Mesh* Resources::sphereMesh;
Mesh* Resources::capsuleMesh;
Mesh* Resources::quadMesh;

HashTable<String, Model*> Resources::models;

HashTable<U64, GameObject2D*> Resources::gameObjects2D;
HashTable<U64, GameObject3D*> Resources::gameObjects3D;
U64 Resources::gameObjectId;

#define BINARIES_PATH "../assets/"
#define TEXTURES_PATH "../assets/textures/"
#define AUDIO_PATH "../assets/audio/"
#define SHADERS_PATH "../assets/shaders/"
#define MATERIALS_PATH "../assets/materials/"
#define MODELS_PATH "../assets/models/"
#define CONFIG_PATH "../assets/config/"
#define FONTS_PATH "../assets/fonts/"

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

struct WAVHeader
{
	U32	riffId;
	U32	size;
	U32	waveId;
};

struct WAVChunk
{
	U32 id;
	U32	size;
};

struct WAVFormat
{
	U16	formatTag;
	U16	channels;
	U32	samplesPerSec;
	U32	avgBytesPerSec;
	U16	blockAlign;
	U16	bitsPerSample;
	U16	size;
	U16	validBitsPerSample;
	U32	channelMask;
	U8	subFormat[16];
};

struct RiffIterator
{
	U8* at;
	U8* stop;
};

#define RIFF_CODE(a, b, c, d) (((U32)(a) << 0) | ((U32)(b) << 8) | ((U32)(c) << 16) | ((U32)(d) << 24))
enum WavChunkId
{
	WAV_CHUNK_ID_FMT = RIFF_CODE('f', 'm', 't', ' '),
	WAV_CHUNK_ID_DATA = RIFF_CODE('d', 'a', 't', 'a'),
	WAV_CHUNK_ID_RIFF = RIFF_CODE('R', 'I', 'F', 'F'),
	WAV_CHUNK_ID_WAVE = RIFF_CODE('W', 'A', 'V', 'E'),
};
#pragma pack(pop)

bool Resources::Initialize()
{
	textures(1009);
	audio(1009);
	fonts(29);
	meshes(1009);
	models(1009);
	gameObjects2D(1009);
	gameObjects3D(1009);

	defaultTexture = LoadTexture(DEFAULT_TEXTURE_NAME);
	defaultDiffuse = LoadTexture(DEFAULT_DIFFUSE_TEXTURE_NAME);
	defaultSpecular = LoadTexture(DEFAULT_SPECULAR_TEXTURE_NAME);
	defaultNormal = LoadTexture(DEFAULT_NORMAL_TEXTURE_NAME);

	defaultMaterialShader = LoadShader(DEFAULT_MATERIAL_SHADER_NAME);

	defaultMaterial = LoadMaterial(DEFAULT_MATERIAL_NAME);

	//TODO: Temporary, Load all materials, create shaders
	LoadMaterial("Background.mat");
	LoadMaterial("Wall.mat");
	LoadMaterial("Block.mat");
	LoadMaterial("Decoration.mat");
	LoadMaterial("Liquid.mat");
	LoadMaterial("UI.mat");

	CreateShaders();

	return true;
}

void Resources::Shutdown()
{
	for (Renderpass* r : renderpasses)
	{
		RendererFrontend::DestroyRenderpass(r);
		r->name.Destroy();
		Memory::Free(r, sizeof(Renderpass), MEMORY_TAG_RESOURCE);
	}

	renderpasses.Destroy();

	for (HashTable<String, AudioFull*>::Node& n : audio)
	{
		n.key.Destroy();
		DestroyAudio(n.value);
	}

	audio.Destroy();

	for (HashTable<String, TTFInfo*>::Node& n : fonts)
	{
		n.key.Destroy();
		DestroyTTF(n.value);
	}

	fonts.Destroy();

	for (HashTable<U64, GameObject2D*>::Node& n : gameObjects2D)
	{
		DestroyGameObject2D(n.value);
	}

	gameObjects2D.Destroy();

	for (HashTable<U64, GameObject3D*>::Node& n : gameObjects3D)
	{
		n.value->name.Destroy();
		Memory::Free(n.value, sizeof(GameObject3D), MEMORY_TAG_RESOURCE);
	}

	gameObjects3D.Destroy();

	for (HashTable<String, Texture*>::Node& n : textures)
	{
		n.key.Destroy();
		DestroyTexture(n.value);
	}

	textures.Destroy();

	for (HashTable<String, Mesh*>::Node& n : meshes)
	{
		n.key.Destroy();
		DestroyMesh(n.value);
	}

	meshes.Destroy();

	for (HashTable<String, Model*>::Node& n : models)
	{
		n.key.Destroy();
		n.value->name.Destroy();
		n.value->meshes.Clear();
	}

	models.Destroy();

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

			I64 equalIndex = line.IndexOf('=');
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

			if (varName == "fullscreen") { Settings::FULLSCREEN = varValue.ToBool(); }
			else if (varName == "lockCursor") { Settings::LOCK_CURSOR = varValue.ToBool(); }
			else if (varName == "framerate") { Settings::TARGET_FRAMETIME = varValue.ToF64(); }
			else if (varName == "channels") { Settings::CHANNEL_COUNT = varValue.ToU8(); }
			else if (varName == "master") { Settings::MASTER_VOLUME = varValue.ToF32(); }
			else if (varName == "music") { Settings::MUSIC_VOLUME = varValue.ToF32(); }
			else if (varName == "sfx") { Settings::SFX_VOLUME = varValue.ToF32(); }
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
			else if (varName == "resolutionSmall")
			{
				Vector<String> dimensions = Move(varValue.Split(',', true));
				if (dimensions.Size() != 2) { Logger::Warn("Settings.cfg: resolution isn't in format width,height, setting to default..."); }
				else
				{
					Settings::WINDOW_WIDTH_SMALL = dimensions[0].ToU16();
					Settings::WINDOW_HEIGHT_SMALL = dimensions[1].ToU16();
				}
			}
			else if (varName == "position")
			{
				Vector<String> position = Move(varValue.Split(',', true));
				if (position.Size() != 2) { Logger::Warn("Settings.cfg: position isn't in format x,y, setting to default..."); }
				else
				{
					Settings::WINDOW_POSITION_X = position[0].ToI16();
					Settings::WINDOW_POSITION_Y = position[1].ToI16();
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
	Logger::Info("Saving engine settings...");

	String path = CONFIG_PATH;
	path.Append("Settings.cfg");

	File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
	if (file->Open(path, FILE_MODE_WRITE, true))
	{
		String settings("#graphics\r\nresolution={},{}\r\nresolutionSmall={},{}\r\nposition={},{}\r\nfullscreen={}\r\nlockCursor={}\r\nframerate={}\r\n\r\n#audio\r\nchannels={}\r\nmaster={}\r\nmusic={}\r\nsfx={}",
			Settings::WindowWidth, Settings::WindowHeight, Settings::WindowWidthSmall, Settings::WindowHeightSmall, Settings::WindowPositionX, Settings::WindowPositionY,
			Settings::Fullscreen, Settings::LockCursor, Settings::TargetFrametime, Settings::ChannelCount, Settings::MasterVolume, Settings::MusicVolume, Settings::SfxVolume);

		file->Write(settings);
	}

	file->Close();
	Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
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
		binary->data.SetArray(file->ReadAllBytes(size, MEMORY_TAG_DATA_STRUCT), size);
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
}

Image* Resources::LoadImage(const String& name)
{
	String path(TEXTURES_PATH);
	path.Append(name);

	File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
	if (file->Open(path, FILE_MODE_READ, true))
	{
		Image* image = (Image*)Memory::Allocate(sizeof(Image), MEMORY_TAG_RESOURCE);
		image->name = name;

		Vector<String> sections = name.Split('.', true);

		bool result;
		if (sections.Back() == "bmp") { result = LoadBMP(image, file); }
		else if (sections.Back() == "png") { result = LoadPNG(image, file); }
		else if (sections.Back() == "jpg" || sections.Back() == "jpeg") { result = LoadJPG(image, file); }
		else if (sections.Back() == "tga") { result = LoadTGA(image, file); }
		else { Logger::Error("Unkown file extention '{}'", sections.Back()); result = false; }

		file->Close();
		Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);

		if (!result)
		{
			image->name.Destroy();
			Memory::Free(image, sizeof(Image), MEMORY_TAG_RESOURCE);
			return nullptr;
		}

		return image;
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
	I32 width;
	I32 pad;

	if (info.infoSize == 12 && info.imageBitCount < 24) { pSize = (header.imageOffset - info.extraRead - 24) / 3; }
	else if (info.imageBitCount < 16) { pSize = (header.imageOffset - info.extraRead - info.infoSize) >> 2; }

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
			for (I32 j = 0; j < info.imageHeight; ++j)
			{
				I8 bitOffset = 7;
				U8 v = file->ReadU8();
				for (I32 i = 0; i < info.imageWidth; ++i)
				{
					U8 index = (v >> bitOffset) & 0x1;
					image->pixels.Push(palette[index * 3]);
					image->pixels.Push(palette[index * 3 + 1]);
					image->pixels.Push(palette[index * 3 + 2]);
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
			for (I32 j = 0; j < info.imageHeight; ++j)
			{
				for (I32 i = 0; i < info.imageWidth; i += 2)
				{
					U8 index0 = file->ReadU8();
					U8 index1 = index0 & 15;
					index0 >>= 4;
					image->pixels.Push(palette[index0 * 3]);
					image->pixels.Push(palette[index0 * 3 + 1]);
					image->pixels.Push(palette[index0 * 3 + 2]);
					image->pixels.Push(255);
					if (i + 1 >= info.imageWidth) { break; }
					image->pixels.Push(palette[index1 * 3]);
					image->pixels.Push(palette[index1 * 3 + 1]);
					image->pixels.Push(palette[index1 * 3 + 2]);
					image->pixels.Push(255);
				}
				file->Seek(pad);
			}
		} break;
		case 8:
		{
			for (I32 j = 0; j < info.imageHeight; ++j)
			{
				for (I32 i = 0; i < info.imageWidth; ++i)
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
		else if (info.imageBitCount == 32 && info.blueMask == 0xff && info.greenMask == 0xff00 && info.redMask == 0x00ff0000 && info.alphaMask == 0xff000000) { easy = 2; }

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

		for (I32 j = 0; j < info.imageHeight; ++j)
		{
			if (easy)
			{
				for (I32 i = 0; i < info.imageWidth; ++i)
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
				for (I32 i = 0; i < info.imageWidth; ++i)
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
		Logger::Error("File is not a BMP!");
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
	TGAHeader* header = (TGAHeader*)file->ReadBytes(sizeof(TGAHeader), MEMORY_TAG_RESOURCE);

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

	if (texture) { return texture; }

	Logger::Info("Loading texture '{}'...", name);

	Image* image = LoadImage(name);

	if (image)
	{
		texture = (Texture*)Memory::Allocate(sizeof(Texture), MEMORY_TAG_TEXTURE);

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
	Memory::Free(texture, sizeof(Texture), MEMORY_TAG_TEXTURE);
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

			I64 equalIndex = line.IndexOf('=');
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

	Shader* shader = nullptr;

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

			I64 equalIndex = line.IndexOf('=');
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
					GetConfigType(fields[0], attribute.type, attribute.size);
					attribute.name = fields[1];
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
					GetConfigType(fields[0], uniform.type, uniform.size);

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
					GetConfigType(fields[0], pushConstant.type, pushConstant.size);
					pushConstant.name = fields[1];
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

void Resources::GetConfigType(const String& field, FieldType& type, U32& size)
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

		for (U32 i = 0; i < materials.Size(); ++i) { materials[i]->id = i; }

		for (String& s : shader->stageFilenames) { s.Destroy(); }

		first = false;
	}
}

Material* Resources::LoadMaterial(const String& name)
{
	if (name.Blank()) { return nullptr; }
	for (Material* m : materials) { if (m->name == name) { return nullptr; } }

	Logger::Info("Loading material '{}'...", name);

	String path(MATERIALS_PATH);
	path.Append(name);

	File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
	if (file->Open(path, FILE_MODE_READ, true))
	{
		Material* material = (Material*)Memory::Allocate(sizeof(Material), MEMORY_TAG_RESOURCE);
		material->name = name;

		MaterialConfig materialConfig;

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

			I64 equalIndex = line.IndexOf('=');
			if (equalIndex == -1)
			{
				Logger::Warn("LoadMaterial('{}'): Potential formatting issue found in file '{}': '=' token not found. Skipping line {}.", name, path, lineNumber);
				++lineNumber;
				continue;
			}

			String varName(Move(line.SubString(0, equalIndex).Trim()));

			String varValue(Move(line.SubString(equalIndex + 1).Trim()));

			//TODO: Use HashTable with switch statement
			if (varName == "color") { materialConfig.diffuseColor = Vector4(varValue); }
			else if (varName == "shader") { materialConfig.shaderName = Move(varValue); }
			else if (varName == "shininess") { materialConfig.shininess = varValue.ToF32(); }
			else if (varName == "globalTextureMaps") { materialConfig.textureMapNames = Move(varValue.Split(',', true)); }

			++lineNumber;
		}

		file->Close();
		Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);

		if (materialConfig.shaderName.Blank()) { materialConfig.shaderName = DEFAULT_MATERIAL_SHADER_NAME; }

		Shader* shader = LoadShader(materialConfig.shaderName);

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

		if (!found) { materials.Push(material); }

		for (String& s : materialConfig.textureMapNames) { s.Destroy(); }

		return material;
	}

	Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);

	return nullptr;
}

void Resources::CreateMaterial(MaterialConfig& config, Material* material)
{
	material->diffuseColor = config.diffuseColor;
	material->shininess = config.shininess;
	material->globalTextureMaps.Reserve(config.textureMapNames.Size());

	for (String& s : config.textureMapNames)
	{
		//TODO: Config
		TextureMap map{};
		map.texture = LoadTexture(s);
		map.filterMinify = TEXTURE_FILTER_MODE_NEAREST;
		map.filterMagnify = TEXTURE_FILTER_MODE_NEAREST;
		map.repeatU = TEXTURE_REPEAT_REPEAT;
		map.repeatV = TEXTURE_REPEAT_REPEAT;
		map.repeatW = TEXTURE_REPEAT_REPEAT;

		if (!RendererFrontend::AcquireTextureMapResources(map))
		{
			Logger::Error("LoadMaterial: Error loading TextureMap resources");
			return;
		}

		material->globalTextureMaps.Push(Move(map));
	}
}

void Resources::DestroyMaterial(Material* material)
{
	for (TextureMap& map : material->globalTextureMaps)
	{
		RendererFrontend::ReleaseTextureMapResources(map);
		map.texture = nullptr;
	}

	material->name.Destroy();
	material->globalTextureMaps.Destroy();
	material->instanceTextureMaps.Destroy();
	Memory::Free(material, sizeof(Material), MEMORY_TAG_RESOURCE);
}

void Resources::GetMaterialInstance(const String& name, Vector<Texture*>& instanceTextures, Material& instance)
{
	Material* material = nullptr;

	for (Material* m : materials)
	{
		if (m->name == name)
		{
			material = m;
			break;
		}
	}

	if (!material)
	{
		Logger::Error("Material '{}' doesn't exist or is in wrong directory", name);
		return;
	}

	instance = *material;

	if (instance.shader->useInstances)
	{
		for (Texture* t : instanceTextures)
		{
			//TODO: Config
			TextureMap map{};
			map.texture = t;
			map.filterMinify = TEXTURE_FILTER_MODE_NEAREST;
			map.filterMagnify = TEXTURE_FILTER_MODE_NEAREST;
			map.repeatU = TEXTURE_REPEAT_REPEAT;
			map.repeatV = TEXTURE_REPEAT_REPEAT;
			map.repeatW = TEXTURE_REPEAT_REPEAT;

			if (!RendererFrontend::AcquireTextureMapResources(map))
			{
				Logger::Error("LoadMaterial: Error loading TextureMap resources");
			}

			instance.instanceTextureMaps.Push(Move(map));
		}

		instance.instance = RendererFrontend::AcquireInstanceResources(instance.shader, instance.instanceTextureMaps);

		if (instance.instance == INVALID_ID) { return; }
	}
}

void Resources::DestroyMaterialInstance(Material& material)
{
	for (TextureMap& map : material.instanceTextureMaps)
	{
		RendererFrontend::ReleaseInstanceResources(material.shader, material.instance);
		RendererFrontend::ReleaseTextureMapResources(map);
		map.texture = nullptr;
	}

	material.name.Destroy();
	material.globalTextureMaps.Destroy();
	material.instanceTextureMaps.Destroy();
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
	if (config.name.Blank())
	{
		Logger::Error("Mesh name can not be blank or nullptr!");
		return nullptr;
	}

	Mesh* mesh = meshes[config.name];

	if (!mesh)
	{
		mesh = (Mesh*)Memory::Allocate(sizeof(Mesh), MEMORY_TAG_RESOURCE);
		mesh->name = config.name;
	}

	mesh->vertices = config.vertices;
	mesh->vertexCount = config.vertexCount;
	mesh->vertexSize = config.vertexSize;
	mesh->indices = config.indices;

	if (!RendererFrontend::CreateMesh(mesh))
	{
		Logger::Error("Failed to create mesh '{}'", config.name);
		Memory::Free(mesh, sizeof(Mesh), MEMORY_TAG_RESOURCE);
		return nullptr;
	}

	if (config.MaterialName.Blank()) { GetMaterialInstance(DEFAULT_MATERIAL_NAME, config.instanceTextures, mesh->material); }
	else { GetMaterialInstance(config.MaterialName, config.instanceTextures, mesh->material); }

	meshes.Insert(config.name, mesh);

	return mesh;
}

Mesh* Resources::CreateFreeMesh(MeshConfig& config)
{
	Mesh* mesh = (Mesh*)Memory::Allocate(sizeof(Mesh), MEMORY_TAG_RESOURCE);
	mesh->name = config.name;
	mesh->vertices = config.vertices;
	mesh->vertexCount = config.vertexCount;
	mesh->vertexSize = config.vertexSize;
	mesh->indices = config.indices;

	if (!RendererFrontend::CreateMesh(mesh))
	{
		Logger::Error("Failed to create mesh '{}'", config.name);
		Memory::Free(mesh, sizeof(Mesh), MEMORY_TAG_RESOURCE);
		return nullptr;
	}

	if (config.MaterialName.Blank()) { GetMaterialInstance(DEFAULT_MATERIAL_NAME, config.instanceTextures, mesh->material); }
	else { GetMaterialInstance(config.MaterialName, config.instanceTextures, mesh->material); }

	return mesh;
}

void Resources::BatchCreateFreeMeshes(Vector<MeshConfig>& configs, Vector<Mesh*>& meshes)
{
	meshes.Reserve(configs.Size());

	for (MeshConfig& config : configs)
	{
		Mesh* mesh = (Mesh*)Memory::Allocate(sizeof(Mesh), MEMORY_TAG_RESOURCE);
		mesh->name = config.name;
		mesh->vertices = config.vertices;
		mesh->vertexCount = config.vertexCount;
		mesh->vertexSize = config.vertexSize;
		mesh->indices = config.indices;

		if (config.MaterialName.Blank()) { GetMaterialInstance(DEFAULT_MATERIAL_NAME, config.instanceTextures, mesh->material); }
		else { GetMaterialInstance(config.MaterialName, config.instanceTextures, mesh->material); }

		meshes.Push(mesh);
	}

	if (!RendererFrontend::BatchCreateMeshes(meshes))
	{
		Logger::Error("Failed to batch create meshes!");

		for (Mesh* mesh : meshes)
		{
			Memory::Free(mesh, sizeof(Mesh), MEMORY_TAG_RESOURCE);
		}
	}
}

void Resources::DestroyMesh(Mesh* mesh)
{
	meshes.Remove(mesh->name, nullptr);

	DestroyMaterialInstance(mesh->material);
	RendererFrontend::DestroyMesh(mesh);
	mesh->name.Destroy();
	Memory::Free(mesh->vertices, mesh->vertexCount * mesh->vertexSize, MEMORY_TAG_RESOURCE);
	mesh->indices.Destroy();
	Memory::Free(mesh, sizeof(Mesh), MEMORY_TAG_RESOURCE);
}

void Resources::DestroyFreeMesh(Mesh* mesh)
{
	DestroyMaterialInstance(mesh->material);
	RendererFrontend::DestroyMesh(mesh);
	mesh->name.Destroy();
	Memory::Free(mesh->vertices, mesh->vertexCount * mesh->vertexSize, MEMORY_TAG_RESOURCE);
	mesh->indices.Destroy();
	Memory::Free(mesh, sizeof(Mesh), MEMORY_TAG_RESOURCE);
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

void Resources::DestroyModel(Model* model)
{
	models.Remove(model->name, nullptr);

	for (Mesh* mesh : model->meshes) { DestroyMesh(mesh); }
	model->meshes.Destroy();
	model->name.Destroy();
	Memory::Free(model, sizeof(Model), MEMORY_TAG_RESOURCE);
}

Model* Resources::CreateModel(const String& name, const Vector<Mesh*>& meshes)
{
	if (name.Blank())
	{
		Logger::Error("Model name can not be blank or nullptr!");
		return nullptr;
	}

	if (!meshes.Size())
	{
		Logger::Error("Model must have at least one mesh!");
		return nullptr;
	}

	Model* model = models[name];

	if (model)
	{
		Logger::Error("Model with name '{}' has already been created!", name);
		return nullptr;
	}

	model = (Model*)Memory::Allocate(sizeof(Model), MEMORY_TAG_RESOURCE);
	model->name = name;
	model->meshes = meshes;

	models.Insert(name, model);

	return model;
}

void Resources::LoadOBJ(Model* mesh, struct File* file)
{

}

void Resources::LoadKSM(Model* mesh, struct File* file)
{

}

GameObject2D* Resources::CreateGameObject2D(const GameObject2DConfig& config)
{
	if (config.name.Blank())
	{
		Logger::Error("GameObject name can not be blank or nullptr!");
		return nullptr;
	}

	Logger::Info("Creating GameObject '{}'...", config.name);

	GameObject2D* go = (GameObject2D*)Memory::Allocate(sizeof(GameObject2D), MEMORY_TAG_GAMEOBJECT);
	go->id = gameObjectId;
	++gameObjectId;
	go->model = config.model;
	go->name = config.name;
	go->physics = config.physics;
	go->transform = config.transform;
	go->enabled = true;

	gameObjects2D.Insert(go->id, go);

	return go;
}

void Resources::DestroyGameObject2D(GameObject2D* gameObject)
{
	gameObjects2D.Remove(gameObject->id, nullptr);

	gameObject->name.Destroy();
	if (gameObject->transform) { delete gameObject->transform; }

	Memory::Free(gameObject, sizeof(GameObject2D), MEMORY_TAG_GAMEOBJECT);
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

	texture = (Texture*)Memory::Allocate(sizeof(Texture), MEMORY_TAG_TEXTURE);

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
		Memory::Free(texture, sizeof(texture), MEMORY_TAG_TEXTURE);
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

	Texture* texture = (Texture*)Memory::Allocate(sizeof(Texture), MEMORY_TAG_TEXTURE);

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

AudioFull* Resources::LoadAudio(const String& name)
{
	if (name.Blank())
	{
		Logger::Error("Model name can not be blank or nullptr!");
		return nullptr;
	}

	AudioFull* audioFull = audio[name];

	if (audioFull) { return audioFull; }

	String path(AUDIO_PATH);
	path.Append(name);

	File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
	if (file->Open(path, FILE_MODE_READ, true))
	{
		audioFull = (AudioFull*)Memory::Allocate(sizeof(AudioFull), MEMORY_TAG_AUDIO);
		audioFull->name = name;
		audioFull->data = file->ReadAllBytes(audioFull->dataSize, MEMORY_TAG_AUDIO);

		WAVHeader* header = (WAVHeader*)audioFull->data;
		if (header->riffId != WAV_CHUNK_ID_RIFF)
		{
			Logger::Error("File is not a WAV!");
			Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
			Memory::Free(audioFull->data, audioFull->dataSize, MEMORY_TAG_AUDIO);
			Memory::Free(audioFull, sizeof(AudioFull), MEMORY_TAG_AUDIO);
			return nullptr;
		}

		for (RiffIterator it = ParseChunkAt(header + 1, (U8*)(header + 1) + header->size - 4); IsValid(it); it = NextChunk(it))
		{
			switch (GetType(it))
			{
			case WAV_CHUNK_ID_FMT:
			{
				WAVFormat* fmt = (WAVFormat*)GetChunkData(it);
				if (fmt->formatTag != 1 || fmt->samplesPerSec != 48000 || fmt->bitsPerSample != 16 || fmt->blockAlign != (fmt->channels * sizeof(I16)) || !fmt->channels)
				{
					Logger::Error("WAV file {} is invalid!", audioFull->name);
					Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
					Memory::Free(audioFull->data, audioFull->dataSize, MEMORY_TAG_AUDIO);
					Memory::Free(audioFull, sizeof(AudioFull), MEMORY_TAG_AUDIO);
					return nullptr;
				}
				audioFull->channelCount = fmt->channels;
			} break;
			case WAV_CHUNK_ID_DATA:
			{
				audioFull->sampleData = (I16*)GetChunkData(it);
				audioFull->sampleDataSize = GetChunkSize(it);
				if (!audioFull->sampleData)
				{
					Logger::Error("WAV file {} is invalid!", audioFull->name);
					Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
					Memory::Free(audioFull->data, audioFull->dataSize, MEMORY_TAG_AUDIO);
					Memory::Free(audioFull, sizeof(AudioFull), MEMORY_TAG_AUDIO);
					return nullptr;
				}
			} break;
			default: break;
			}
		}

		Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);

		audio.Insert(audioFull->name, audioFull);
	}
	else
	{
		Logger::Error("Couldn't open file: {}", name);
		Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
		Memory::Free(audioFull, sizeof(AudioFull), MEMORY_TAG_AUDIO);
		return nullptr;
	}

	AudioChunk* chunk = (AudioChunk*)Memory::Allocate(sizeof(AudioChunk), MEMORY_TAG_AUDIO);
	chunk->sampleCount = 0;
	chunk->firstSampleIndex = 0;
	chunk->samples = (I16**)Memory::Allocate(sizeof(I16*) * audioFull->channelCount, MEMORY_TAG_AUDIO);

	LoadWAV(audioFull, chunk);
	audioFull->chunks = chunk;

	return audioFull;
}

void Resources::LoadAudioChunk(AudioFull* full, AudioChunk* chunk)
{
	AudioChunk* next = (AudioChunk*)Memory::Allocate(sizeof(AudioChunk), MEMORY_TAG_AUDIO);
	next->firstSampleIndex = chunk->firstSampleIndex + (AUDIO_CHUNK_LENGTH * full->channelCount);
	next->samples = (I16**)Memory::Allocate(sizeof(I16*) * full->channelCount, MEMORY_TAG_AUDIO);

	LoadWAV(full, next);

	chunk->next = next;
}

void Resources::LoadWAV(AudioFull* full, AudioChunk* chunk)
{
	U32 sampleCount = full->sampleDataSize / (full->channelCount * sizeof(I16));
	U32 sectionSampleCount = AUDIO_CHUNK_LENGTH;
	I32 samplesLeft = (I32)sampleCount - (I32)(chunk->firstSampleIndex / full->channelCount);

	if (sectionSampleCount >= (U32)samplesLeft)
	{
		sectionSampleCount = (U32)Math::Max(samplesLeft, 0);
		chunk->last = true;
	}

	sampleCount = sectionSampleCount;

	for (U32 channelIndex = 0; channelIndex < full->channelCount; ++channelIndex)
	{
		chunk->samples[channelIndex] = (I16*)Memory::Allocate(sizeof(I16) * sectionSampleCount, MEMORY_TAG_AUDIO);

		for (U32 sampleIndex = 0, dataIndex = chunk->firstSampleIndex + channelIndex; sampleIndex < sectionSampleCount; ++sampleIndex, dataIndex += full->channelCount)
		{
			chunk->samples[channelIndex][sampleIndex] = full->sampleData[dataIndex];
			//chunk->samples[channelIndex][sampleIndex] = (I16)(Math::Sin((F32)dataIndex / 50.0f) * 1000.0f);
		}
	}

	chunk->sampleCount = sampleCount;
}

void Resources::DestroyAudio(AudioFull* full)
{
	Memory::Free(full->data, full->dataSize, MEMORY_TAG_AUDIO);
	full->name.Destroy();

	AudioChunk* chunk = full->chunks;
	while (chunk)
	{
		for (U32 i = 0; i < full->channelCount; ++i)
		{
			Memory::Free(chunk->samples[i], sizeof(I16) * chunk->sampleCount, MEMORY_TAG_AUDIO);
		}

		Memory::Free(chunk->samples, sizeof(I16*) * full->channelCount, MEMORY_TAG_AUDIO);
		AudioChunk* temp = chunk;
		chunk = chunk->next;
		Memory::Free(temp, sizeof(AudioChunk), MEMORY_TAG_AUDIO);
	}

	Memory::Free(full, sizeof(AudioFull), MEMORY_TAG_AUDIO);
}

RiffIterator Resources::ParseChunkAt(void* at, void* stop)
{
	RiffIterator it;

	it.at = (U8*)at;
	it.stop = (U8*)stop;

	return it;
}

RiffIterator Resources::NextChunk(RiffIterator& it)
{
	WAVChunk* chunk = (WAVChunk*)it.at;
	U32 size = (chunk->size + 1) & ~1;
	it.at += sizeof(WAVChunk) + size;

	return it;
}

void* Resources::GetChunkData(const RiffIterator& it)
{
	return it.at + sizeof(WAVChunk);
}

bool Resources::IsValid(const RiffIterator& it)
{
	return it.at < it.stop;
}

U32 Resources::GetType(const RiffIterator& it)
{
	WAVChunk* chunk = (WAVChunk*)it.at;
	return chunk->id;
}

U32 Resources::GetChunkSize(const RiffIterator& it)
{
	WAVChunk* chunk = (WAVChunk*)it.at;
	return chunk->size;
}

#pragma region Fonts

#define STBTT_RASTERIZER_VERSION 1
#define ttBYTE(p)     (* (U8 *) (p))
#define ttCHAR(p)     (* (I8 *) (p))
#define ttFixed(p)    ttLONG(p)
static U16 ttUSHORT(U8* p) { return p[0] * 256 + p[1]; }
static I16 ttSHORT(U8* p) { return p[0] * 256 + p[1]; }
static U32 ttULONG(U8* p) { return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; }
static I32 ttLONG(U8* p) { return (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3]; }
#define stbtt_tag4(p,c0,c1,c2,c3)	((p)[0] == (c0) && (p)[1] == (c1) && (p)[2] == (c2) && (p)[3] == (c3))
#define stbtt_tag(p,str)			stbtt_tag4(p,str[0],str[1],str[2],str[3])
#define STBTT_FIXSHIFT   10
#define STBTT_FIX        (1 << STBTT_FIXSHIFT)
#define STBTT_FIXMASK    (STBTT_FIX-1)

enum PlatformID
{
	STBTT_PLATFORM_ID_UNICODE = 0,
	STBTT_PLATFORM_ID_MAC = 1,
	STBTT_PLATFORM_ID_ISO = 2,
	STBTT_PLATFORM_ID_MICROSOFT = 3
};

enum UnicodeEncodingID
{
	STBTT_UNICODE_EID_UNICODE_1_0 = 0,
	STBTT_UNICODE_EID_UNICODE_1_1 = 1,
	STBTT_UNICODE_EID_ISO_10646 = 2,
	STBTT_UNICODE_EID_UNICODE_2_0_BMP = 3,
	STBTT_UNICODE_EID_UNICODE_2_0_FULL = 4
};

enum MicrosoftEncodingID
{
	STBTT_MS_EID_SYMBOL = 0,
	STBTT_MS_EID_UNICODE_BMP = 1,
	STBTT_MS_EID_SHIFTJIS = 2,
	STBTT_MS_EID_UNICODE_FULL = 10
};

enum MacEncodingID
{
	STBTT_MAC_EID_ROMAN = 0,
	STBTT_MAC_EID_ARABIC = 4,
	STBTT_MAC_EID_JAPANESE = 1,
	STBTT_MAC_EID_HEBREW = 5,
	STBTT_MAC_EID_CHINESE_TRAD = 2,
	STBTT_MAC_EID_GREEK = 6,
	STBTT_MAC_EID_KOREAN = 3,
	STBTT_MAC_EID_RUSSIAN = 7
};

enum MicrosoftLanguageID
{
	STBTT_MS_LANG_ENGLISH = 0x0409,
	STBTT_MS_LANG_ITALIAN = 0x0410,
	STBTT_MS_LANG_CHINESE = 0x0804,
	STBTT_MS_LANG_JAPANESE = 0x0411,
	STBTT_MS_LANG_DUTCH = 0x0413,
	STBTT_MS_LANG_KOREAN = 0x0412,
	STBTT_MS_LANG_FRENCH = 0x040c,
	STBTT_MS_LANG_RUSSIAN = 0x0419,
	STBTT_MS_LANG_GERMAN = 0x0407,
	STBTT_MS_LANG_SPANISH = 0x0409,
	STBTT_MS_LANG_HEBREW = 0x040d,
	STBTT_MS_LANG_SWEDISH = 0x041D
};

enum MacLanguageID
{
	STBTT_MAC_LANG_ENGLISH = 0,
	STBTT_MAC_LANG_JAPANESE = 11,
	STBTT_MAC_LANG_ARABIC = 12,
	STBTT_MAC_LANG_KOREAN = 23,
	STBTT_MAC_LANG_DUTCH = 4,
	STBTT_MAC_LANG_RUSSIAN = 32,
	STBTT_MAC_LANG_FRENCH = 1,
	STBTT_MAC_LANG_SPANISH = 6,
	STBTT_MAC_LANG_GERMAN = 2,
	STBTT_MAC_LANG_SWEDISH = 5,
	STBTT_MAC_LANG_HEBREW = 10,
	STBTT_MAC_LANG_CHINESE_SIMPLIFIED = 33,
	STBTT_MAC_LANG_ITALIAN = 3,
	STBTT_MAC_LANG_CHINESE_TRAD = 19
};

enum
{
	STBTT_vmove = 1,
	STBTT_vline,
	STBTT_vcurve,
	STBTT_vcubic
};

struct FontBitmap
{
	Vector<U8> pixels;
	I32 w, h, stride;
};

struct FontVertex
{
	I16 x, y, cx, cy, cx1, cy1;
	U8 type, padding;
};

struct FontCsctx
{
	FontCsctx(I32 bounds) : bounds{ bounds }, started{ 0 }, first_x{ 0 }, first_y{ 0 }, x{ 0 }, y{ 0 }, min_x{ 0 }, max_x{ 0 }, min_y{ 0 }, max_y{ 0 }, vertices{ }, num_vertices{ 0 } {}

	I32 bounds;
	I32 started;
	F32 first_x, first_y;
	F32 x, y;
	I32 min_x, max_x, min_y, max_y;

	Vector<FontVertex> vertices;
	I32 num_vertices;
};

struct FontEdge
{
	F32 x0, y0, x1, y1;
	I32 invert;
};

struct FontActiveEdge
{
	FontActiveEdge* next;
#if STBTT_RASTERIZER_VERSION==1
	I32 x, dx;
	F32 ey;
	I32 direction;
#elif STBTT_RASTERIZER_VERSION==2
	F32 fx, fdx, fdy;
	F32 direction;
	F32 sy;
	F32 ey;
#else
#error "Unrecognized value of STBTT_RASTERIZER_VERSION"
#endif
};

void Resources::LoadFont(const String& name)
{
	if (name.Blank()) { Logger::Error("Font name can not be blank or nullptr!"); return; }

	TTFInfo* font = fonts[name];

	if (font) { Logger::Error("Font '{}' has already been loaded!", font->name); return; }

	Logger::Info("Loading font '{}'...", name);

	String path(FONTS_PATH);
	path.Append(name);

	File* file = (File*)Memory::Allocate(sizeof(File), MEMORY_TAG_RESOURCE);
	if (file->Open(path, FILE_MODE_READ, true))
	{
		font = (TTFInfo*)Memory::Allocate(sizeof(TTFInfo), MEMORY_TAG_RESOURCE);
		font->name = name;

		U64 size = file->Size();
		font->data.SetArray(file->ReadAllBytes(size, MEMORY_TAG_DATA_STRUCT), size);

		Vector<String> sections = name.Split('.', true);

		bool result;
		if (sections.Back() == "ttf") { result = LoadTTF(font); }
		else { Logger::Error("Unkown file extention '{}'", sections.Back()); result = false; }

		file->Close();
		Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);

		if (!result)
		{
			font->name.Destroy();
			font->data.Destroy();
			Memory::Free(font, sizeof(TTFInfo), MEMORY_TAG_RESOURCE);
			return;
		}

		fonts.Insert(font->name, font);
	}
	else
	{
		Logger::Error("Couldn't open file: {}", name);
		Memory::Free(file, sizeof(File), MEMORY_TAG_RESOURCE);
	}
}

bool Resources::LoadTTF(TTFInfo* info)
{
	U32 cmap, t;
	I32 i, numTables;

	info->fontstart = GetFontOffset(info->data, 0); //TODO: Config

	cmap = FindTTFTable(info->data, info->fontstart, "cmap");       // required
	info->loca = FindTTFTable(info->data, info->fontstart, "loca"); // required
	info->head = FindTTFTable(info->data, info->fontstart, "head"); // required
	info->glyf = FindTTFTable(info->data, info->fontstart, "glyf"); // required
	info->hhea = FindTTFTable(info->data, info->fontstart, "hhea"); // required
	info->hmtx = FindTTFTable(info->data, info->fontstart, "hmtx"); // required
	info->kern = FindTTFTable(info->data, info->fontstart, "kern"); // not required
	info->gpos = FindTTFTable(info->data, info->fontstart, "GPOS"); // not required

	if (!cmap || !info->head || !info->hhea || !info->hmtx) { return false; }

	if (info->glyf) { if (!info->loca) { return false; } }
	else
	{
		FontBuffer b, topdict, topdictidx;
		U32 cstype = 2, charstrings = 0, fdarrayoff = 0, fdselectoff = 0;
		U32 cff;

		cff = FindTTFTable(info->data, info->fontstart, "CFF ");
		if (!cff) { return false; }

		// @TODO this should use size from table (not 512MB)
		info->cff = FontBuffer(info->data.Data() + cff, Megabytes(8));
		b = info->cff;

		b.Skip(2);
		b.Seek(b.Get8());

		// @TODO the name INDEX could list multiple fonts,
		// but we just use the first one.
		b.GetIndex();
		topdictidx = b.GetIndex();
		topdict = b.IndexGet(0);
		b.GetIndex();
		info->gsubrs = b.GetIndex();

		topdict.DictGetInts(17, 1, &charstrings);
		topdict.DictGetInts(0x100 | 6, 1, &cstype);
		topdict.DictGetInts(0x100 | 36, 1, &fdarrayoff);
		topdict.DictGetInts(0x100 | 37, 1, &fdselectoff);
		info->subrs = b.GetSubrs(topdict);

		if (cstype != 2) { return false; }
		if (charstrings == 0) { return false; }

		if (fdarrayoff)
		{
			if (!fdselectoff) { return false; }
			b.Seek(fdarrayoff);
			info->fontdicts = b.GetIndex();
			info->fdselect = b.Range(fdselectoff, b.size - fdselectoff);
		}

		b.Seek(charstrings);
		info->charstrings = b.GetIndex();
	}

	t = FindTTFTable(info->data, info->fontstart, "maxp");
	if (t) { info->numGlyphs = ttUSHORT(info->data.Data() + t + 4); }
	else { info->numGlyphs = 0xffff; }

	info->svg = -1;

	// find a cmap encoding table we understand *now* to avoid searching
	// later. (todo: could make this installable)
	// the same regardless of glyph.
	numTables = ttUSHORT(info->data.Data() + cmap + 2);
	info->index_map = 0;
	for (i = 0; i < numTables; ++i)
	{
		U32 encoding_record = cmap + 4 + 8 * i;

		switch (ttUSHORT(info->data.Data() + encoding_record))
		{
		case STBTT_PLATFORM_ID_MICROSOFT:
			switch (ttUSHORT(info->data.Data() + encoding_record + 2))
			{
			case STBTT_MS_EID_UNICODE_BMP:
			case STBTT_MS_EID_UNICODE_FULL:
				info->index_map = cmap + ttULONG(info->data.Data() + encoding_record + 4);
				break;
			} break;
		case STBTT_PLATFORM_ID_UNICODE:
			info->index_map = cmap + ttULONG(info->data.Data() + encoding_record + 4);
			break;
		}
	}

	if (info->index_map == 0) { return false; }

	info->indexToLocFormat = ttUSHORT(info->data.Data() + info->head + 50);
	info->ascent = ttSHORT(info->data.Data() + info->hhea + 4);
	info->descent = ttSHORT(info->data.Data() + info->hhea + 6);
	info->lineGap = ttSHORT(info->data.Data() + info->hhea + 8);

	info->letterTextures(199);

	return true;
}

F32 Resources::FontRatio(const String& name)
{
	return 0.0f;
}

void Resources::DestroyTTF(TTFInfo* info)
{
	info->name.Destroy();
	info->data.Destroy();

	for (HashTable<U64, Texture*>::Node& n : info->letterTextures)
	{
		DestroyTexture(n.value);
	}

	info->letterTextures.Destroy();
}

Texture* Resources::CreateFontCharacter(const String& fontName, I32 c, F32 heightPixels, const Vector3& color, I32& xOff, I32& yOff) //TODO: Color
{
	if (c == 32) { return nullptr; }

	TTFInfo* font = fonts[fontName];

	if (!font) { Logger::Error("Font '{}' isn't loaded!", fontName); return nullptr; }

	F32 height = ScaleForPixelHeight(font, heightPixels);
	U64 id = (U64)(c * (height * 10000.0f));

	Texture* texture = font->letterTextures[id];

	if (texture) { return texture; }

	Vector2Int dimensions;
	Vector<U8> alphas = GetCodepointBitmap(font, 0.0f, height, c, dimensions.x, dimensions.y, xOff, yOff);

	if (!alphas.Size())
	{
		Logger::Error("Failed to generate font bitmap!");
		return nullptr;
	}

	texture = (Texture*)Memory::Allocate(sizeof(Texture), MEMORY_TAG_TEXTURE);
	texture->name = font->name + c;
	texture->width = dimensions.x;
	texture->height = dimensions.y;
	texture->generation = 0;
	texture->channelCount = 4;
	texture->flags = TEXTURE_FLAG_HAS_TRANSPARENCY | TEXTURE_FLAG_IS_WRITEABLE;

	if (!RendererFrontend::CreateWritableTexture(texture))
	{
		Logger::Error("Failed to create writable texture '{}'!", texture->name);
		Memory::Free(texture, sizeof(texture), MEMORY_TAG_RESOURCE);
		return nullptr;
	}

	if (!texture)
	{
		Logger::Error("Failed to generate texture!");
		return nullptr;
	}

	Vector<U8> pixels(dimensions.x * dimensions.y * 4, 0);

	for (U64 i = 0, j = 0; i < pixels.Size(); ++i, ++j)
	{
		pixels[i] = (U8)color.x;
		pixels[++i] = (U8)color.y;
		pixels[++i] = (U8)color.z;
		pixels[++i] = alphas[j];
	}

	RendererFrontend::WriteTextureData(texture, pixels);

	font->letterTextures.Insert(id, texture);

	return texture;
}

// @OPTIMIZE: binary search
U32 Resources::FindTTFTable(Vector<U8>& data, U32 fontstart, const char* tag)
{
	I32 num_tables = ttUSHORT(data.Data() + fontstart + 4);
	U32 tabledir = fontstart + 12;
	I32 i;

	for (i = 0; i < num_tables; ++i)
	{
		U32 loc = tabledir + 16 * i;
		if (stbtt_tag(data.Data() + loc, tag)) { return ttULONG(data.Data() + loc + 8); }
	}

	return 0;
}

I32 Resources::GetFontOffset(Vector<U8>& data, I32 index)
{
	if (IsFont(data)) { return index == 0 ? 0 : -1; }

	if (stbtt_tag(data, "ttcf"))
	{
		if (ttULONG(data.Data() + 4) == 0x00010000 || ttULONG(data.Data() + 4) == 0x00020000)
		{
			I32 n = ttLONG(data.Data() + 8);
			if (index >= n) { return -1; }
			return ttULONG(data.Data() + 12 + index * 4);
		}
	}
	return -1;
}

bool Resources::IsFont(Vector<U8>& data)
{
	// TrueType 1
	// TrueType with type 1 font -- we don't support this!
	// OpenType with CFF
	// OpenType 1.0
	// Apple specification for TrueType fonts
	return stbtt_tag4(data, '1', 0, 0, 0) || stbtt_tag(data, "typ1") || stbtt_tag(data, "OTTO") || stbtt_tag4(data, 0, 1, 0, 0) || stbtt_tag(data, "true");
}

F32 Resources::ScaleForPixelHeight(TTFInfo* info, F32 height)
{
	return (F32)height / (info->ascent - info->descent);
}

//TODO: Move Vector
Vector<U8> Resources::GetCodepointBitmap(TTFInfo* info, F32 scaleX, F32 scaleY, I32 codepoint, I32& width, I32& height, I32& xOff, I32& yOff)
{
	return GetCodepointBitmapSubpixel(info, scaleX, scaleY, 0.0f, 0.0f, codepoint, width, height, xOff, yOff);
}

Vector<U8> Resources::GetCodepointBitmapSubpixel(TTFInfo* info, F32 scaleX, F32 scaleY, F32 shiftX, F32 shiftY, I32 codepoint, I32& width, I32& height, I32& xOff, I32& yOff)
{
	return GetGlyphBitmapSubpixel(info, scaleX, scaleY, shiftX, shiftY, FindGlyphIndex(info, codepoint), width, height, xOff, yOff);
}

Vector<U8> Resources::GetGlyphBitmapSubpixel(TTFInfo* info, F32 scaleX, F32 scaleY, F32 shiftX, F32 shiftY, I32 glyph, I32& width, I32& height, I32& xOff, I32& yOff)
{
	I32 ix0, iy0, ix1, iy1;
	FontBitmap gbm;
	Vector<FontVertex> vertices;
	I32 numVerts = GetGlyphShape(info, glyph, vertices);

	if (scaleX == 0) { scaleX = scaleY; }
	if (scaleY == 0)
	{
		if (scaleX == 0) { return gbm.pixels; }
		scaleY = scaleX;
	}

	GetGlyphBitmapBoxSubpixel(info, glyph, scaleX, scaleY, shiftX, shiftY, ix0, iy0, ix1, iy1);

	gbm.w = (ix1 - ix0);
	gbm.h = (iy1 - iy0);
	gbm.pixels = NULL;

	width = gbm.w;
	height = gbm.h;
	xOff = ix0;
	yOff = iy0;

	if (gbm.w && gbm.h)
	{
		gbm.pixels.Resize(gbm.w * gbm.h);
		gbm.stride = gbm.w;

		RasterizeFont(gbm, 0.35f, vertices, numVerts, scaleX, scaleY, shiftX, shiftY, ix0, iy0, 1);
	}

	return gbm.pixels;
}

void Resources::MakeCodepointBitmap(TTFInfo* info, Vector<U8>& output, I32 outW, I32 outH, I32 outStride, F32 scaleX, F32 scaleY, I32 codepoint)
{
	MakeCodepointBitmapSubpixel(info, output, outW, outH, outStride, scaleX, scaleY, 0.0f, 0.0f, codepoint);
}

void Resources::MakeCodepointBitmapSubpixel(TTFInfo* info, Vector<U8>& output, I32 outW, I32 outH, I32 outStride, F32 scaleX, F32 scaleY, F32 shiftX, F32 shiftY, I32 codepoint)
{
	MakeGlyphBitmapSubpixel(info, output, outW, outH, outStride, scaleX, scaleY, shiftX, shiftY, FindGlyphIndex(info, codepoint));
}

void Resources::GetGlyphBitmapBoxSubpixel(TTFInfo* font, I32 glyph, F32 scaleX, F32 scaleY, F32 shiftX, F32 shiftY, I32& ix0, I32& iy0, I32& ix1, I32& iy1)
{
	I32 x0 = 0, y0 = 0, x1, y1;

	if (!GetGlyphBox(font, glyph, x0, y0, x1, y1))
	{
		ix0 = 0;
		iy0 = 0;
		ix1 = 0;
		iy1 = 0;
	}
	else
	{
		ix0 = Math::Floor(x0 * scaleX + shiftX);
		iy0 = Math::Floor(-y1 * scaleY + shiftY);
		ix1 = Math::Ceiling(x1 * scaleX + shiftX);
		iy1 = Math::Ceiling(-y0 * scaleY + shiftY);
	}
}

void Resources::MakeGlyphBitmapSubpixel(TTFInfo* info, Vector<U8>& output, I32 outW, I32 outH, I32 outStride, F32 scaleX, F32 scaleY, F32 shiftX, F32 shiftY, I32 glyph)
{
	I32 ix0, iy0, ix1, iy1;
	Vector<FontVertex> vertices;
	I32 numVerts = GetGlyphShape(info, glyph, vertices);
	FontBitmap gbm;

	GetGlyphBitmapBoxSubpixel(info, glyph, scaleX, scaleY, shiftX, shiftY, ix0, iy0, ix1, iy1);
	gbm.pixels = output;
	gbm.w = outW;
	gbm.h = outH;
	gbm.stride = outStride;

	if (gbm.w && gbm.h) { RasterizeFont(gbm, 0.35f, vertices, numVerts, scaleX, scaleY, shiftX, shiftY, ix0, iy0, 1); }
}

I32 Resources::FindGlyphIndex(TTFInfo* info, I32 unicodeCodepoint)
{
	U8* data = info->data.Data();
	U32 index_map = info->index_map;

	U16 format = ttUSHORT(data + index_map + 0);
	if (format == 0)
	{
		I32 bytes = ttUSHORT(data + index_map + 2);
		if (unicodeCodepoint < bytes - 6) { return ttBYTE(data + index_map + 6 + unicodeCodepoint); }
		return 0;
	}
	else if (format == 6)
	{
		U32 first = ttUSHORT(data + index_map + 6);
		U32 count = ttUSHORT(data + index_map + 8);
		if ((U32)unicodeCodepoint >= first && (U32)unicodeCodepoint < first + count) { return ttUSHORT(data + index_map + 10 + (unicodeCodepoint - first) * 2); }
		return 0;
	}
	else if (format == 2)
	{
		ASSERT(false); // @TODO: high-byte mapping for japanese/chinese/korean
		return 0;
	}
	else if (format == 4)
	{
		U16 segcount = ttUSHORT(data + index_map + 6) >> 1;
		U16 searchRange = ttUSHORT(data + index_map + 8) >> 1;
		U16 entrySelector = ttUSHORT(data + index_map + 10);
		U16 rangeShift = ttUSHORT(data + index_map + 12) >> 1;

		U32 endCount = index_map + 14;
		U32 search = endCount;

		if (unicodeCodepoint > 0xffff) { return 0; }

		if (unicodeCodepoint >= ttUSHORT(data + search + rangeShift * 2)) { search += rangeShift * 2; }

		search -= 2;
		while (entrySelector)
		{
			U16 end;
			searchRange >>= 1;
			end = ttUSHORT(data + search + searchRange * 2);
			if (unicodeCodepoint > end) { search += searchRange * 2; }
			--entrySelector;
		}
		search += 2;

		{
			U16 offset, start, last;
			U16 item = (U16)((search - endCount) >> 1);

			start = ttUSHORT(data + index_map + 14 + segcount * 2 + 2 + 2 * item);
			last = ttUSHORT(data + endCount + 2 * item);
			if (unicodeCodepoint < start || unicodeCodepoint > last) { return 0; }

			offset = ttUSHORT(data + index_map + 14 + segcount * 6 + 2 + 2 * item);
			if (offset == 0) { return (U16)(unicodeCodepoint + ttSHORT(data + index_map + 14 + segcount * 4 + 2 + 2 * item)); }

			return ttUSHORT(data + offset + (unicodeCodepoint - start) * 2 + index_map + 14 + segcount * 6 + 2 + 2 * item);
		}
	}
	else if (format == 12 || format == 13)
	{
		U32 ngroups = ttULONG(data + index_map + 12);
		I32 low, high;
		low = 0; high = (I32)ngroups;

		while (low < high)
		{
			I32 mid = low + ((high - low) >> 1);
			U32 start_char = ttULONG(data + index_map + 16 + mid * 12);
			U32 end_char = ttULONG(data + index_map + 16 + mid * 12 + 4);
			if ((U32)unicodeCodepoint < start_char) { high = mid; }
			else if ((U32)unicodeCodepoint > end_char) { low = mid + 1; }
			else
			{
				U32 start_glyph = ttULONG(data + index_map + 16 + mid * 12 + 8);
				if (format == 12) { return start_glyph + unicodeCodepoint - start_char; }
				else { return start_glyph; }
			}
		}
		return 0;
	}

	// @TODO
	ASSERT(false);
	return 0;
}

I32 Resources::GetGlyfOffset(TTFInfo* info, I32 glyphIndex)
{
	I32 g1, g2;

	ASSERT(!info->cff.size);

	if (glyphIndex >= info->numGlyphs) { return -1; }
	if (info->indexToLocFormat >= 2) { return -1; }

	if (info->indexToLocFormat == 0)
	{
		g1 = info->glyf + ttUSHORT(info->data.Data() + info->loca + glyphIndex * 2) * 2;
		g2 = info->glyf + ttUSHORT(info->data.Data() + info->loca + glyphIndex * 2 + 2) * 2;
	}
	else
	{
		g1 = info->glyf + ttULONG(info->data.Data() + info->loca + glyphIndex * 4);
		g2 = info->glyf + ttULONG(info->data.Data() + info->loca + glyphIndex * 4 + 4);
	}

	return g1 == g2 ? -1 : g1;
}

I32 Resources::GetGlyphShape(TTFInfo* info, I32 glyphIndex, Vector<FontVertex>& vertices)
{
	if (!info->cff.size) { return GetGlyphShapeTT(info, glyphIndex, vertices); }
	else { return GetGlyphShapeT2(info, glyphIndex, vertices); }
}

I32 Resources::GetGlyphShapeTT(TTFInfo* info, I32 glyphIndex, Vector<FontVertex>& pvertices)
{
	I16 numberOfContours;
	U8* endPtsOfContours;
	U8* data = info->data.Data();
	FontVertex* vertices = nullptr;
	I32 numVertices = 0;
	I32 g = GetGlyfOffset(info, glyphIndex);

	if (g < 0) { return 0; }

	numberOfContours = ttSHORT(data + g);

	if (numberOfContours > 0)
	{
		U8 flags = 0, flagcount;
		I32 ins, i, j = 0, m, n, nextMove, wasOff = 0, off, startOff = 0;
		I32 x, y, cx, cy, sx, sy, scx, scy;
		U8* points;

		endPtsOfContours = (data + g + 10);
		ins = ttUSHORT(data + g + 10 + numberOfContours * 2);
		points = data + g + 10 + numberOfContours * 2 + 2 + ins;

		n = 1 + ttUSHORT(endPtsOfContours + numberOfContours * 2 - 2);

		m = n + 2 * numberOfContours;
		vertices = (FontVertex*)Memory::Allocate(m * sizeof(FontVertex), MEMORY_TAG_RESOURCE);
		if (!vertices) { return 0; }

		nextMove = 0;
		flagcount = 0;

		off = m - n;

		for (i = 0; i < n; ++i)
		{
			if (flagcount == 0)
			{
				flags = *points++;
				if (flags & 8) { flagcount = *points++; }
			}
			else { --flagcount; }
			vertices[off + i].type = flags;
		}

		x = 0;
		for (i = 0; i < n; ++i)
		{
			flags = vertices[off + i].type;
			if (flags & 2)
			{
				I16 dx = *points++;
				x += (flags & 16) ? dx : -dx;
			}
			else
			{
				if (!(flags & 16))
				{
					x = x + (I16)(points[0] * 256 + points[1]);
					points += 2;
				}
			}
			vertices[off + i].x = (I16)x;
		}

		y = 0;
		for (i = 0; i < n; ++i)
		{
			flags = vertices[off + i].type;
			if (flags & 4)
			{
				I16 dy = *points++;
				y += (flags & 32) ? dy : -dy;
			}
			else
			{
				if (!(flags & 32))
				{
					y = y + (I16)(points[0] * 256 + points[1]);
					points += 2;
				}
			}
			vertices[off + i].y = (I16)y;
		}

		numVertices = 0;
		sx = sy = cx = cy = scx = scy = 0;
		for (i = 0; i < n; ++i)
		{
			flags = vertices[off + i].type;
			x = (I16)vertices[off + i].x;
			y = (I16)vertices[off + i].y;

			if (nextMove == i)
			{
				if (i != 0) { numVertices = CloseShape(vertices, numVertices, wasOff, startOff, sx, sy, scx, scy, cx, cy); }

				startOff = !(flags & 1);
				if (startOff)
				{
					scx = x;
					scy = y;
					if (!(vertices[off + i + 1].type & 1))
					{
						sx = (x + (I32)vertices[off + i + 1].x) >> 1;
						sy = (y + (I32)vertices[off + i + 1].y) >> 1;
					}
					else
					{
						sx = (I32)vertices[off + i + 1].x;
						sy = (I32)vertices[off + i + 1].y;
						++i;
					}
				}
				else
				{
					sx = x;
					sy = y;
				}

				SetVertex(vertices[numVertices++], STBTT_vmove, sx, sy, 0, 0);
				wasOff = 0;
				nextMove = 1 + ttUSHORT(endPtsOfContours + j * 2);
				++j;
			}
			else
			{
				if (!(flags & 1))
				{
					if (wasOff) { SetVertex(vertices[numVertices++], STBTT_vcurve, (cx + x) >> 1, (cy + y) >> 1, cx, cy); }
					cx = x;
					cy = y;
					wasOff = 1;
				}
				else
				{
					if (wasOff) { SetVertex(vertices[numVertices++], STBTT_vcurve, x, y, cx, cy); }
					else { SetVertex(vertices[numVertices++], STBTT_vline, x, y, 0, 0); }
					wasOff = 0;
				}
			}
		}

		numVertices = CloseShape(vertices, numVertices, wasOff, startOff, sx, sy, scx, scy, cx, cy);
	}
	else if (numberOfContours < 0)
	{
		I32 more = 1;
		U8* comp = data + g + 10;
		numVertices = 0;
		vertices = nullptr;

		while (more)
		{
			U16 flags, gidx;
			I32 compNumVerts = 0, i;
			Vector<FontVertex> compVerts;
			FontVertex* tmp = nullptr;
			F32 mtx[6] = { 1,0,0,1,0,0 }, m, n;

			flags = ttSHORT(comp); comp += 2;
			gidx = ttSHORT(comp); comp += 2;

			if (flags & 2)
			{
				if (flags & 1)
				{
					mtx[4] = ttSHORT(comp); comp += 2;
					mtx[5] = ttSHORT(comp); comp += 2;
				}
				else
				{
					mtx[4] = ttCHAR(comp); comp += 1;
					mtx[5] = ttCHAR(comp); comp += 1;
				}
			}
			else
			{
				// @TODO handle matching point
				ASSERT(false);
			}
			if (flags & (1 << 3))
			{
				mtx[0] = mtx[3] = ttSHORT(comp) / 16384.0f; comp += 2;
				mtx[1] = mtx[2] = 0;
			}
			else if (flags & (1 << 6))
			{
				mtx[0] = ttSHORT(comp) / 16384.0f; comp += 2;
				mtx[1] = mtx[2] = 0;
				mtx[3] = ttSHORT(comp) / 16384.0f; comp += 2;
			}
			else if (flags & (1 << 7))
			{
				mtx[0] = ttSHORT(comp) / 16384.0f; comp += 2;
				mtx[1] = ttSHORT(comp) / 16384.0f; comp += 2;
				mtx[2] = ttSHORT(comp) / 16384.0f; comp += 2;
				mtx[3] = ttSHORT(comp) / 16384.0f; comp += 2;
			}

			m = (float)Math::Sqrt(mtx[0] * mtx[0] + mtx[1] * mtx[1]);
			n = (float)Math::Sqrt(mtx[2] * mtx[2] + mtx[3] * mtx[3]);

			compNumVerts = GetGlyphShape(info, gidx, compVerts);
			if (compNumVerts > 0)
			{
				for (i = 0; i < compNumVerts; ++i)
				{
					FontVertex* v = &compVerts[i];
					I16 x, y;
					x = v->x; y = v->y;
					v->x = (I16)(m * (mtx[0] * x + mtx[2] * y + mtx[4]));
					v->y = (I16)(n * (mtx[1] * x + mtx[3] * y + mtx[5]));
					x = v->cx; y = v->cy;
					v->cx = (I16)(m * (mtx[0] * x + mtx[2] * y + mtx[4]));
					v->cy = (I16)(n * (mtx[1] * x + mtx[3] * y + mtx[5]));
				}

				tmp = (FontVertex*)Memory::Allocate((numVertices + compNumVerts) * sizeof(FontVertex), MEMORY_TAG_RESOURCE);
				if (!tmp)
				{
					if (vertices) { Memory::Free(vertices, (numVertices + compNumVerts) * sizeof(FontVertex), MEMORY_TAG_RESOURCE); }
					return 0;
				}

				if (numVertices > 0 && vertices) { Memory::Copy(tmp, vertices, numVertices * sizeof(FontVertex)); }
				Memory::Copy(tmp + numVertices, compVerts.Data(), compNumVerts * sizeof(FontVertex));
				if (vertices) { Memory::Free(vertices, (numVertices + compNumVerts) * sizeof(FontVertex), MEMORY_TAG_RESOURCE); }

				vertices = tmp;
				numVertices += compNumVerts;
			}

			more = flags & (1 << 5);
		}
	}

	//TODO: Free vertices
	pvertices.CopyArray(vertices, numVertices);

	return numVertices;
}

I32 Resources::GetGlyphShapeT2(TTFInfo* info, I32 glyphIndex, Vector<FontVertex>& vertices)
{
	FontCsctx countCtx(1);
	FontCsctx outputCtx(0);
	if (RunCharstring(info, glyphIndex, countCtx))
	{
		vertices.Resize(countCtx.num_vertices);
		outputCtx.vertices = vertices;
		if (RunCharstring(info, glyphIndex, outputCtx))
		{
			ASSERT(outputCtx.num_vertices == countCtx.num_vertices);
			return outputCtx.num_vertices;
		}
	}

	vertices.Clear();
	return 0;
}

I32 Resources::GetGlyphBox(TTFInfo* info, I32 glyphIndex, I32& x0, I32& y0, I32& x1, I32& y1)
{
	if (info->cff.size) { GetGlyphInfoT2(info, glyphIndex, x0, y0, x1, y1); }
	else
	{
		I32 g = GetGlyfOffset(info, glyphIndex);
		if (g < 0) { return 0; }

		x0 = ttSHORT(info->data.Data() + g + 2);
		y0 = ttSHORT(info->data.Data() + g + 4);
		x1 = ttSHORT(info->data.Data() + g + 6);
		y1 = ttSHORT(info->data.Data() + g + 8);
	}

	return 1;
}

I32 Resources::GetGlyphInfoT2(TTFInfo* info, I32 glyphIndex, I32& x0, I32& y0, I32& x1, I32& y1)
{
	FontCsctx c(1);
	I32 r = RunCharstring(info, glyphIndex, c);

	x0 = r ? c.min_x : 0;
	y0 = r ? c.min_y : 0;
	x1 = r ? c.max_x : 0;
	y1 = r ? c.max_y : 0;

	return r ? c.num_vertices : 0;
}

FontBuffer Resources::GetGlyphSubrs(TTFInfo* info, I32 glyphIndex)
{
	FontBuffer fdselect = info->fdselect;
	I32 nranges, start, end, v, fmt, fdselector = -1, i;

	fdselect.Seek(0);
	fmt = fdselect.Get8();
	if (fmt == 0)
	{
		fdselect.Skip(glyphIndex);
		fdselector = fdselect.Get8();
	}
	else if (fmt == 3)
	{
		nranges = fdselect.Get16();
		start = fdselect.Get16();
		for (i = 0; i < nranges; i++)
		{
			v = fdselect.Get8();
			end = fdselect.Get16();
			if (glyphIndex >= start && glyphIndex < end)
			{
				fdselector = v;
				break;
			}
			start = end;
		}
	}

	if (fdselector == -1) return FontBuffer(nullptr, 0);

	FontBuffer dict = info->fontdicts.IndexGet(fdselector);

	return info->cff.GetSubrs(dict);
}

I32 Resources::RunCharstring(TTFInfo* info, I32 glyphIndex, FontCsctx& c)
{
	I32 inHeader = 1, maskbits = 0, subrStackHeight = 0, sp = 0, v, i, b0;
	I32 hasSubrs = 0, clearStack;
	F32 s[48];
	FontBuffer subrStack[10], subrs = info->subrs, b;
	F32 f;

#define STBTT__CSERR(s) (0)

	b = info->charstrings.IndexGet(glyphIndex);
	while (b.cursor < b.size)
	{
		i = 0;
		clearStack = 1;
		b0 = b.Get8();
		switch (b0)
		{ // @TODO implement hinting
		case 0x13:
		case 0x14:
			if (inHeader) { maskbits += (sp / 2); }
			inHeader = 0;
			b.Skip((maskbits + 7) / 8);
			break;
		case 0x01:
		case 0x03:
		case 0x12:
		case 0x17:
			maskbits += (sp / 2);
			break;
		case 0x15:
			inHeader = 0;
			if (sp < 2) { return STBTT__CSERR("rmoveto stack"); }
			CsctxRmoveTo(c, s[sp - 2], s[sp - 1]);
			break;
		case 0x04:
			inHeader = 0;
			if (sp < 1) { return STBTT__CSERR("vmoveto stack"); }
			CsctxRmoveTo(c, 0, s[sp - 1]);
			break;
		case 0x16:
			inHeader = 0;
			if (sp < 1) { return STBTT__CSERR("hmoveto stack"); }
			CsctxRmoveTo(c, s[sp - 1], 0);
			break;
		case 0x05:
			if (sp < 2) { return STBTT__CSERR("rlineto stack"); }
			for (; i + 1 < sp; i += 2) { CsctxRlineTo(c, s[i], s[i + 1]); }
			break;
		case 0x07:
			if (sp < 1) { return STBTT__CSERR("vlineto stack"); }
			goto vlineto;
		case 0x06:
			if (sp < 1) return STBTT__CSERR("hlineto stack");
			for (;;)
			{
				if (i >= sp) { break; }
				CsctxRlineTo(c, s[i], 0);
				i++;
			vlineto:
				if (i >= sp) { break; }
				CsctxRlineTo(c, 0, s[i]);
				i++;
			}
			break;
		case 0x1F:
			if (sp < 4) { return STBTT__CSERR("hvcurveto stack"); }
			goto hvcurveto;
		case 0x1E:
			if (sp < 4) { return STBTT__CSERR("vhcurveto stack"); }
			for (;;)
			{
				if (i + 3 >= sp) { break; }
				CsctxRccurveTo(c, 0, s[i], s[i + 1], s[i + 2], s[i + 3], (sp - i == 5) ? s[i + 4] : 0.0f);
				i += 4;
			hvcurveto:
				if (i + 3 >= sp) { break; }
				CsctxRccurveTo(c, s[i], 0, s[i + 1], s[i + 2], (sp - i == 5) ? s[i + 4] : 0.0f, s[i + 3]);
				i += 4;
			}
			break;
		case 0x08:
			if (sp < 6) { return STBTT__CSERR("rcurveline stack"); }
			for (; i + 5 < sp; i += 6) { CsctxRccurveTo(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5]); }
			break;
		case 0x18:
			if (sp < 8) { return STBTT__CSERR("rcurveline stack"); }
			for (; i + 5 < sp - 2; i += 6) { CsctxRccurveTo(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5]); }
			if (i + 1 >= sp) { return STBTT__CSERR("rcurveline stack"); }
			CsctxRlineTo(c, s[i], s[i + 1]);
			break;
		case 0x19:
			if (sp < 8) { return STBTT__CSERR("rlinecurve stack"); }
			for (; i + 1 < sp - 6; i += 2) { CsctxRlineTo(c, s[i], s[i + 1]); }
			if (i + 5 >= sp) { return STBTT__CSERR("rlinecurve stack"); }
			CsctxRccurveTo(c, s[i], s[i + 1], s[i + 2], s[i + 3], s[i + 4], s[i + 5]);
			break;
		case 0x1A:
		case 0x1B:
			if (sp < 4) { return STBTT__CSERR("(vv|hh)curveto stack"); }
			f = 0.0;
			if (sp & 1) { f = s[i]; i++; }
			for (; i + 3 < sp; i += 4)
			{
				if (b0 == 0x1B) { CsctxRccurveTo(c, s[i], f, s[i + 1], s[i + 2], s[i + 3], 0.0); }
				else { CsctxRccurveTo(c, f, s[i], s[i + 1], s[i + 2], 0.0, s[i + 3]); }
				f = 0.0;
			}
			break;
		case 0x0A:
			if (!hasSubrs)
			{
				if (info->fdselect.size) { subrs = GetGlyphSubrs(info, glyphIndex); }
				hasSubrs = 1;
			}
		case 0x1D:
			if (sp < 1) { return STBTT__CSERR("call(g|)subr stack"); }
			v = (int)s[--sp];
			if (subrStackHeight >= 10) { return STBTT__CSERR("recursion limit"); }
			subrStack[subrStackHeight++] = b;
			b = b0 == 0x0A ? subrs.GetSubr(v) : info->gsubrs.GetSubr(v);
			if (b.size == 0) { return STBTT__CSERR("subr not found"); }
			b.cursor = 0;
			clearStack = 0;
			break;
		case 0x0B:
			if (subrStackHeight <= 0) { return STBTT__CSERR("return outside subr"); }
			b = subrStack[--subrStackHeight];
			clearStack = 0;
			break;
		case 0x0E:
			CsctxCloseShape(c);
			return 1;
		case 0x0C: {
			F32 dx1, dx2, dx3, dx4, dx5, dx6, dy1, dy2, dy3, dy4, dy5, dy6;
			F32 dx, dy;
			I32 b1 = b.Get8();
			switch (b1)
			{
				// @TODO These "flex" implementations ignore the flex-depth and resolution,
				// and always draw beziers.
			case 0x22:
				if (sp < 7) { return STBTT__CSERR("hflex stack"); }
				dx1 = s[0];
				dx2 = s[1];
				dy2 = s[2];
				dx3 = s[3];
				dx4 = s[4];
				dx5 = s[5];
				dx6 = s[6];
				CsctxRccurveTo(c, dx1, 0, dx2, dy2, dx3, 0);
				CsctxRccurveTo(c, dx4, 0, dx5, -dy2, dx6, 0);
				break;
			case 0x23:
				if (sp < 13) { return STBTT__CSERR("flex stack"); }
				dx1 = s[0];
				dy1 = s[1];
				dx2 = s[2];
				dy2 = s[3];
				dx3 = s[4];
				dy3 = s[5];
				dx4 = s[6];
				dy4 = s[7];
				dx5 = s[8];
				dy5 = s[9];
				dx6 = s[10];
				dy6 = s[11];
				CsctxRccurveTo(c, dx1, dy1, dx2, dy2, dx3, dy3);
				CsctxRccurveTo(c, dx4, dy4, dx5, dy5, dx6, dy6);
				break;
			case 0x24:
				if (sp < 9) { return STBTT__CSERR("hflex1 stack"); }
				dx1 = s[0];
				dy1 = s[1];
				dx2 = s[2];
				dy2 = s[3];
				dx3 = s[4];
				dx4 = s[5];
				dx5 = s[6];
				dy5 = s[7];
				dx6 = s[8];
				CsctxRccurveTo(c, dx1, dy1, dx2, dy2, dx3, 0);
				CsctxRccurveTo(c, dx4, 0, dx5, dy5, dx6, -(dy1 + dy2 + dy5));
				break;
			case 0x25:
				if (sp < 11) { return STBTT__CSERR("flex1 stack"); }
				dx1 = s[0];
				dy1 = s[1];
				dx2 = s[2];
				dy2 = s[3];
				dx3 = s[4];
				dy3 = s[5];
				dx4 = s[6];
				dy4 = s[7];
				dx5 = s[8];
				dy5 = s[9];
				dx6 = dy6 = s[10];
				dx = dx1 + dx2 + dx3 + dx4 + dx5;
				dy = dy1 + dy2 + dy3 + dy4 + dy5;
				if (Math::Abs(dx) > Math::Abs(dy)) { dy6 = -dy; }
				else { dx6 = -dx; }
				CsctxRccurveTo(c, dx1, dy1, dx2, dy2, dx3, dy3);
				CsctxRccurveTo(c, dx4, dy4, dx5, dy5, dx6, dy6);
				break;
			default: return STBTT__CSERR("unimplemented");
			}
		} break;
		default:
			if (b0 != 255 && b0 != 28 && b0 < 32) { return STBTT__CSERR("reserved operator"); }
			if (b0 == 255) { f = (F32)(I32)b.Get32() / 0x10000; }
			else
			{
				b.Skip(-1);
				f = (float)(I16)b.CffInt();
			}
			if (sp >= 48) { return STBTT__CSERR("push stack overflow"); }
			s[sp++] = f;
			clearStack = 0;
			break;
		}
		if (clearStack) { sp = 0; }
	}
	return STBTT__CSERR("no endchar");

#undef STBTT__CSERR
}

void Resources::CsctxRmoveTo(FontCsctx& ctx, F32 dx, F32 dy)
{
	CsctxCloseShape(ctx);
	ctx.first_x = ctx.x = ctx.x + dx;
	ctx.first_y = ctx.y = ctx.y + dy;
	CsctxVertex(ctx, STBTT_vmove, (I32)ctx.x, (I32)ctx.y, 0, 0, 0, 0);
}

void Resources::CsctxRlineTo(FontCsctx& ctx, F32 dx, F32 dy)
{
	ctx.x += dx;
	ctx.y += dy;
	CsctxVertex(ctx, STBTT_vline, (I32)ctx.x, (I32)ctx.y, 0, 0, 0, 0);
}

void Resources::CsctxRccurveTo(FontCsctx& ctx, F32 dx1, F32 dy1, F32 dx2, F32 dy2, F32 dx3, F32 dy3)
{
	F32 cx1 = ctx.x + dx1;
	F32 cy1 = ctx.y + dy1;
	F32 cx2 = cx1 + dx2;
	F32 cy2 = cy1 + dy2;
	ctx.x = cx2 + dx3;
	ctx.y = cy2 + dy3;
	CsctxVertex(ctx, STBTT_vcubic, (I32)ctx.x, (I32)ctx.y, (I32)cx1, (I32)cy1, (I32)cx2, (I32)cy2);
}

void Resources::CsctxCloseShape(FontCsctx& ctx)
{
	if (ctx.first_x != ctx.x || ctx.first_y != ctx.y) { CsctxVertex(ctx, STBTT_vline, (I32)ctx.first_x, (I32)ctx.first_y, 0, 0, 0, 0); }
}

void Resources::CsctxVertex(FontCsctx& ctx, U8 type, I32 x, I32 y, I32 cx, I32 cy, I32 cx1, I32 cy1)
{
	if (ctx.bounds)
	{
		TrackVertex(ctx, x, y);
		if (type == STBTT_vcubic)
		{
			TrackVertex(ctx, cx, cy);
			TrackVertex(ctx, cx1, cy1);
		}
	}
	else
	{
		SetVertex(ctx.vertices[ctx.num_vertices], type, x, y, cx, cy);
		ctx.vertices[ctx.num_vertices].cx1 = (I16)cx1;
		ctx.vertices[ctx.num_vertices].cy1 = (I16)cy1;
	}
	++ctx.num_vertices;
}

void Resources::TrackVertex(FontCsctx& ctx, I32 x, I32 y)
{
	if (x > ctx.max_x || !ctx.started) { ctx.max_x = x; }
	if (y > ctx.max_y || !ctx.started) { ctx.max_y = y; }
	if (x < ctx.min_x || !ctx.started) { ctx.min_x = x; }
	if (y < ctx.min_y || !ctx.started) { ctx.min_y = y; }

	ctx.started = 1;
}

void Resources::SetVertex(FontVertex& v, U8 type, I32 x, I32 y, I32 cx, I32 cy)
{
	v.type = type;
	v.x = (I16)x;
	v.y = (I16)y;
	v.cx = (I16)cx;
	v.cy = (I16)cy;
}

I32 Resources::CloseShape(FontVertex* vertices, I32 numVertices, I32 wasOff, I32 startOff, I32 sx, I32 sy, I32 scx, I32 scy, I32 cx, I32 cy)
{
	if (startOff)
	{
		if (wasOff) { SetVertex(vertices[numVertices++], STBTT_vcurve, (cx + scx) >> 1, (cy + scy) >> 1, cx, cy); }
		SetVertex(vertices[numVertices++], STBTT_vcurve, sx, sy, scx, scy);
	}
	else
	{
		if (wasOff) { SetVertex(vertices[numVertices++], STBTT_vcurve, sx, sy, cx, cy); }
		else { SetVertex(vertices[numVertices++], STBTT_vline, sx, sy, 0, 0); }
	}
	return numVertices;
}

Vector<Vector2> Resources::FlattenCurves(Vector<FontVertex>& vertices, F32 objspaceFlatness, Vector<I32>& contourLengths)
{
	Vector<Vector2> points;
	I32 numPoints = 0;

	F32 objspace_flatness_squared = objspaceFlatness * objspaceFlatness;
	I32 i, n = 0, start = 0, pass;

	for (i = 0; i < vertices.Size(); ++i) { n += vertices[i].type == STBTT_vmove; }

	if (n == 0) { return points; }

	contourLengths.Resize(n);

	for (pass = 0; pass < 2; ++pass)
	{
		F32 x = 0, y = 0;

		if (pass == 1) { points.Resize(numPoints); }

		numPoints = 0;
		n = -1;

		for (i = 0; i < vertices.Size(); ++i)
		{
			switch (vertices[i].type)
			{
			case STBTT_vmove:
				if (n >= 0) { contourLengths[n] = numPoints - start; }
				++n;
				start = numPoints;

				x = vertices[i].x, y = vertices[i].y;
				AddPoint(points, numPoints++, x, y);
				break;
			case STBTT_vline:
				x = vertices[i].x, y = vertices[i].y;
				AddPoint(points, numPoints++, x, y);
				break;
			case STBTT_vcurve:
				TesselateCurve(points, &numPoints, x, y,
					vertices[i].cx, vertices[i].cy,
					vertices[i].x, vertices[i].y,
					objspace_flatness_squared, 0);
				x = vertices[i].x, y = vertices[i].y;
				break;
			case STBTT_vcubic:
				TesselateCubic(points, &numPoints, x, y,
					vertices[i].cx, vertices[i].cy,
					vertices[i].cx1, vertices[i].cy1,
					vertices[i].x, vertices[i].y,
					objspace_flatness_squared, 0);
				x = vertices[i].x, y = vertices[i].y;
				break;
			}
		}

		contourLengths[n] = numPoints - start;
	}

	return points;
}

void Resources::SortEdges(struct FontEdge* p, I32 n)
{
	SortEdgesQuicksort(p, n);
	SortEdgesInsSort(p, n);
}

void Resources::AddPoint(Vector<Vector2>& points, I32 n, F32 x, F32 y)
{
	if (points.Size())
	{
		points[n].x = x;
		points[n].y = y;
	}
}

I32 Resources::TesselateCurve(Vector<Vector2>& points, I32* numPoints, F32 x0, F32 y0, F32 x1, F32 y1, F32 x2, F32 y2, F32 objspaceFlatnessSquared, I32 n)
{
	F32 mx = (x0 + 2.0f * x1 + x2) * 0.25f;
	F32 my = (y0 + 2.0f * y1 + y2) * 0.25f;

	F32 dx = (x0 + x2) * 0.5f - mx;
	F32 dy = (y0 + y2) * 0.5f - my;

	if (n > 16) { return 1; }

	if (dx * dx + dy * dy > objspaceFlatnessSquared)
	{
		TesselateCurve(points, numPoints, x0, y0, (x0 + x1) * 0.5f, (y0 + y1) * 0.5f, mx, my, objspaceFlatnessSquared, n + 1);
		TesselateCurve(points, numPoints, mx, my, (x1 + x2) * 0.5f, (y1 + y2) * 0.5f, x2, y2, objspaceFlatnessSquared, n + 1);
	}
	else
	{
		AddPoint(points, *numPoints, x2, y2);
		*numPoints = *numPoints + 1;
	}

	return 1;
}

void Resources::TesselateCubic(Vector<Vector2>& points, I32* numPoints, F32 x0, F32 y0, F32 x1, F32 y1, F32 x2, F32 y2, F32 x3, F32 y3, F32 objspaceFlatnessSquared, I32 n)
{
	// @TODO this "flatness" calculation is just made-up nonsense that seems to work well enough
	F32 dx0 = x1 - x0;
	F32 dy0 = y1 - y0;
	F32 dx1 = x2 - x1;
	F32 dy1 = y2 - y1;
	F32 dx2 = x3 - x2;
	F32 dy2 = y3 - y2;
	F32 dx = x3 - x0;
	F32 dy = y3 - y0;
	F32 longlen = (Math::Sqrt(dx0 * dx0 + dy0 * dy0) + Math::Sqrt(dx1 * dx1 + dy1 * dy1) + Math::Sqrt(dx2 * dx2 + dy2 * dy2));
	F32 shortlen = Math::Sqrt(dx * dx + dy * dy);
	F32 flatness_squared = longlen * longlen - shortlen * shortlen;

	if (n > 16) { return; }

	if (flatness_squared > objspaceFlatnessSquared)
	{
		F32 x01 = (x0 + x1) * 0.5f;
		F32 y01 = (y0 + y1) * 0.5f;
		F32 x12 = (x1 + x2) * 0.5f;
		F32 y12 = (y1 + y2) * 0.5f;
		F32 x23 = (x2 + x3) * 0.5f;
		F32 y23 = (y2 + y3) * 0.5f;

		F32 xa = (x01 + x12) * 0.5f;
		F32 ya = (y01 + y12) * 0.5f;
		F32 xb = (x12 + x23) * 0.5f;
		F32 yb = (y12 + y23) * 0.5f;

		F32 mx = (xa + xb) * 0.5f;
		F32 my = (ya + yb) * 0.5f;

		TesselateCubic(points, numPoints, x0, y0, x01, y01, xa, ya, mx, my, objspaceFlatnessSquared, n + 1);
		TesselateCubic(points, numPoints, mx, my, xb, yb, x23, y23, x3, y3, objspaceFlatnessSquared, n + 1);
	}
	else
	{
		AddPoint(points, *numPoints, x3, y3);
		*numPoints = *numPoints + 1;
	}
}

void Resources::SortEdgesInsSort(FontEdge* p, I32 n)
{
	I32 i, j;
	for (i = 1; i < n; ++i)
	{
		FontEdge t = p[i];
		FontEdge* a = &t;
		j = i;

		while (j > 0)
		{
			FontEdge* b = &p[j - 1];
			I32 c = (a)->y0 < (b)->y0;
			if (!c) { break; }
			p[j] = p[j - 1];
			--j;
		}

		if (i != j) { p[j] = t; }
	}
}

void Resources::SortEdgesQuicksort(FontEdge* p, I32 n)
{
	while (n > 12)
	{
		FontEdge t;
		I32 c01, c12, c, m, i, j;

		m = n >> 1;
		c01 = p[0].y0 < p[m].y0;
		c12 = p[m].y0 < p[n - 1].y0;

		if (c01 != c12)
		{
			I32 z;
			c = p[0].y0 < p[n - 1].y0;

			z = (c == c12) ? 0 : n - 1;
			t = p[z];
			p[z] = p[m];
			p[m] = t;
		}

		t = p[0];
		p[0] = p[m];
		p[m] = t;

		i = 1;
		j = n - 1;

		for (;;)
		{
			for (;; ++i) { if (!(p[i].y0 < p[0].y0)) { break; } }
			for (;; --j) { if (!(p[0].y0 < p[j].y0)) { break; } }

			if (i >= j) { break; }
			t = p[i];
			p[i] = p[j];
			p[j] = t;

			++i;
			--j;
		}

		if (j < (n - i))
		{
			SortEdgesQuicksort(p, j);
			p = p + i;
			n = n - i;
		}
		else
		{
			SortEdgesQuicksort(p + i, n - i);
			n = j;
		}
	}
}

FontActiveEdge* Resources::NewActive(struct Heap& hh, FontEdge* e, I32 xOff, F32 startPoint)
{
	FontActiveEdge* z = (FontActiveEdge*)hh.Alloc(sizeof(FontActiveEdge));
	F32 dxdy = (e->x1 - e->x0) / (e->y1 - e->y0);
	ASSERT(z != nullptr);
	if (!z) { return z; }

	if (dxdy < 0) { z->dx = -Math::Floor(STBTT_FIX * -dxdy); }
	else { z->dx = Math::Floor(STBTT_FIX * dxdy); }

	z->x = Math::Floor(STBTT_FIX * e->x0 + z->dx * (startPoint - e->y0));
	z->x -= xOff * STBTT_FIX;

	z->ey = e->y1;
	z->next = 0;
	z->direction = e->invert ? 1 : -1;
	return z;
}

void Resources::FillActiveEdges(U8* scanline, I32 len, FontActiveEdge* e, I32 maxWeight)
{
	I32 x0 = 0, w = 0;

	while (e)
	{
		if (w == 0) { x0 = e->x; w += e->direction; }
		else
		{
			I32 x1 = e->x; w += e->direction;

			if (w == 0)
			{
				I32 i = x0 >> STBTT_FIXSHIFT;
				I32 j = x1 >> STBTT_FIXSHIFT;

				if (i < len && j >= 0)
				{
					if (i == j) { scanline[i] = scanline[i] + (U8)((x1 - x0) * maxWeight >> STBTT_FIXSHIFT); }
					else
					{
						if (i >= 0) { scanline[i] = scanline[i] + (U8)(((STBTT_FIX - (x0 & STBTT_FIXMASK)) * maxWeight) >> STBTT_FIXSHIFT); }
						else { i = -1; }

						if (j < len) { scanline[j] = scanline[j] + (U8)(((x1 & STBTT_FIXMASK) * maxWeight) >> STBTT_FIXSHIFT); }
						else { j = len; }

						for (++i; i < j; ++i) { scanline[i] = scanline[i] + (U8)maxWeight; }
					}
				}
			}
		}

		e = e->next;
	}
}

void Resources::RasterizeFont(FontBitmap& result, F32 flatnessInPixels, Vector<FontVertex>& vertices, I32 numVerts, F32 scaleX, F32 scaleY, F32 shiftX, F32 shiftY, I32 xOff, I32 yOff, I32 invert)
{
	F32 scale = scaleX > scaleY ? scaleY : scaleX;
	Vector<I32> windingLengths;
	Vector<Vector2> windings = FlattenCurves(vertices, flatnessInPixels / scale, windingLengths);

	if (windings.Size())
	{
		F32 y_scale_inv = invert ? -scaleY : scaleY;
		FontEdge* e;
		I32 n, i, j, k, m;
#if STBTT_RASTERIZER_VERSION == 1
		I32 vsubsample = result.h < 8 ? 15 : 5;
#elif STBTT_RASTERIZER_VERSION == 2
		I32 vsubsample = 1;
#else
#error "Unrecognized value of STBTT_RASTERIZER_VERSION"
#endif

		I32 size = 0;
		for (i = 0; i < windingLengths.Size(); ++i) { size += windingLengths[i]; }

		e = (FontEdge*)Memory::Allocate(sizeof(FontEdge) * (size + 1), MEMORY_TAG_RESOURCE);
		if (e == nullptr) { return; }
		n = 0;

		m = 0;
		for (i = 0; i < windingLengths.Size(); ++i)
		{
			Vector2* p = windings.Data() + m;
			m += windingLengths[i];
			j = windingLengths[i] - 1;
			for (k = 0; k < windingLengths[i]; j = k++)
			{
				I32 a = k, b = j;
				if (p[j].y == p[k].y) { continue; }
				e[n].invert = 0;
				if (invert ? p[j].y > p[k].y : p[j].y < p[k].y)
				{
					e[n].invert = 1;
					a = j, b = k;
				}
				e[n].x0 = p[a].x * scaleX + shiftX;
				e[n].y0 = (p[a].y * y_scale_inv + shiftY) * vsubsample;
				e[n].x1 = p[b].x * scaleX + shiftX;
				e[n].y1 = (p[b].y * y_scale_inv + shiftY) * vsubsample;
				++n;
			}
	}

		SortEdges(e, n);

		RasterizeSortedEdges(result, e, n, vsubsample, xOff, yOff);

		Memory::Free(e, sizeof(FontEdge) * (size + 1), MEMORY_TAG_RESOURCE);
}
}

void Resources::RasterizeSortedEdges(FontBitmap& result, FontEdge* edge, I32 n, I32 vSubSample, I32 xOff, I32 yOff)
{
	Heap hh = { 0, 0, 0 };
	FontActiveEdge* active = nullptr;
	I32 y, j = 0;
	I32 max_weight = (255 / vSubSample);
	I32 s;
	U8 scanlineData[512];
	U8* scanline;

	if (result.w > 512) { scanline = (U8*)Memory::Allocate(result.w, MEMORY_TAG_RESOURCE); }
	else { scanline = scanlineData; }

	y = yOff * vSubSample;
	edge[n].y0 = (yOff + result.h) * (F32)vSubSample + 1;

	while (j < result.h)
	{
		Memory::Zero(scanline, result.w);

		for (s = 0; s < vSubSample; ++s)
		{
			F32 scanY = y + 0.5f;
			FontActiveEdge** step = &active;

			while (*step)
			{
				FontActiveEdge* z = *step;
				if (z->ey <= scanY)
				{
					*step = z->next;
					ASSERT(z->direction);
					z->direction = 0;
					hh.Free(z);
				}
				else
				{
					z->x += z->dx;
					step = &((*step)->next);
				}
			}

			for (;;)
			{
				I32 changed = 0;
				step = &active;
				while (*step && (*step)->next)
				{
					if ((*step)->x > (*step)->next->x)
					{
						FontActiveEdge* t = *step;
						FontActiveEdge* q = t->next;

						t->next = q->next;
						q->next = t;
						*step = q;
						changed = 1;
					}
					step = &(*step)->next;
				}
				if (!changed) { break; }
			}

			while (edge->y0 <= scanY)
			{
				if (edge->y1 > scanY)
				{
					FontActiveEdge* z = NewActive(hh, edge, xOff, scanY);
					if (z)
					{
						if (active == nullptr) { active = z; }
						else if (z->x < active->x)
						{
							z->next = active;
							active = z;
						}
						else
						{
							FontActiveEdge* p = active;
							while (p->next && p->next->x < z->x) { p = p->next; }
							z->next = p->next;
							p->next = z;
						}
					}
				}
				++edge;
			}

			if (active) { FillActiveEdges(scanline, result.w, active, max_weight); }

			++y;
		}

		Memory::Copy(result.pixels.Data() + j * result.stride, scanline, result.w);
		++j;
	}

	hh.Cleanup();

	if (scanline != scanlineData) { Memory::Free(scanline, result.w, MEMORY_TAG_RESOURCE); }
}

#pragma endregion