#include "ResourceDefines.hpp"

#include "Rendering\CommandBuffer.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Resources.hpp"

// SAMPLER CREATION

SamplerCreation& SamplerCreation::SetMinMagMip(VkFilter min, VkFilter mag, VkSamplerMipmapMode mip)
{
	minFilter = min;
	magFilter = mag;
	mipFilter = mip;

	return *this;
}

SamplerCreation& SamplerCreation::SetAddressModeU(VkSamplerAddressMode u)
{
	addressModeU = u;

	return *this;
}

SamplerCreation& SamplerCreation::SetAddressModeUV(VkSamplerAddressMode u, VkSamplerAddressMode v)
{
	addressModeU = u;
	addressModeV = v;

	return *this;
}

SamplerCreation& SamplerCreation::SetAddressModeUVW(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w)
{
	addressModeU = u;
	addressModeV = v;
	addressModeW = w;

	return *this;
}

SamplerCreation& SamplerCreation::SetName(const String& name)
{
	this->name = name;

	return *this;
}

// TEXTURE CREATION

TextureCreation& TextureCreation::SetSize(U16 width, U16 height, U16 depth)
{
	this->width = width;
	this->height = height;
	this->depth = depth;

	return *this;
}

TextureCreation& TextureCreation::SetFormatType(VkFormat format, VkImageType type)
{
	this->format = format;
	this->type = type;

	return *this;
}

TextureCreation& TextureCreation::SetName(const String& name)
{
	this->name = name;

	return *this;
}

TextureCreation& TextureCreation::SetData(void* data)
{
	initialData = data;

	return *this;
}

// RENDER PASS OUTPUT

RenderpassOutput& RenderpassOutput::Reset()
{
	colorFormatCount = 0;

	for (U32 i = 0; i < MAX_IMAGE_OUTPUTS; ++i)
	{
		colorFormats[i] = VK_FORMAT_UNDEFINED;
	}

	depthStencilFormat = VK_FORMAT_UNDEFINED;
	colorOperation = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthOperation = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	stencilOperation = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

	return *this;
}

RenderpassOutput& RenderpassOutput::Color(VkFormat format)
{
	colorFormats[colorFormatCount++] = format;
	return *this;
}

RenderpassOutput& RenderpassOutput::Depth(VkFormat format)
{
	depthStencilFormat = format;
	return *this;
}

RenderpassOutput& RenderpassOutput::SetOperations(VkAttachmentLoadOp color, VkAttachmentLoadOp depth, VkAttachmentLoadOp stencil)
{
	colorOperation = color;
	depthOperation = depth;
	stencilOperation = stencil;

	return *this;
}

// RENDER PASS

void Renderpass::Resize()
{
	if (lastResize < Renderer::CurrentFrame())
	{
		lastResize = Renderer::CurrentFrame();

		width = Settings::WindowWidth();
		height = Settings::WindowHeight();

		for (U32 i = 0; i < renderTargetCount; ++i)
		{
			Resources::RecreateTexture(outputTextures[i], width, height, 1);
			vkDestroyFramebuffer(Renderer::device, frameBuffers[i], Renderer::allocationCallbacks);
		}

		if (outputDepth)
		{
			Resources::RecreateTexture(outputDepth, width, height, 1);
		}

		vkDestroyRenderPass(Renderer::device, renderpass, Renderer::allocationCallbacks);

		Renderer::CreateRenderPass(this);
	}
}

// RENDER PASS CREATION

RenderpassCreation& RenderpassCreation::Reset()
{
	width = 0;
	height = 0;
	renderTargetCount = 0;

	depthStencilTexture = {};

	colorOperation = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthOperation = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	stencilOperation = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

	name.Clear();

	return *this;
}

RenderpassCreation& RenderpassCreation::AddRenderTarget(Texture* texture)
{
	outputTextures[renderTargetCount++] = texture;

	return *this;
}

RenderpassCreation& RenderpassCreation::SetDepthStencilTexture(Texture* texture)
{
	depthStencilTexture = texture;

	return *this;
}

RenderpassCreation& RenderpassCreation::SetName(const String& name)
{
	this->name = name;

	return *this;
}

RenderpassCreation& RenderpassCreation::SetOperations(VkAttachmentLoadOp color, VkAttachmentLoadOp depth, VkAttachmentLoadOp stencil)
{
	colorOperation = color;
	depthOperation = depth;
	stencilOperation = stencil;

	return *this;
}

RenderpassCreation& RenderpassCreation::AddClearColor(const Vector4& color)
{
	clears[clearCount++].color = { color.x, color.y, color.z, color.w };

	return *this;
}

RenderpassCreation& RenderpassCreation::AddClearDepth(F32 depth)
{
	clears[clearCount++].depthStencil = { depth, 0 };

	return *this;
}