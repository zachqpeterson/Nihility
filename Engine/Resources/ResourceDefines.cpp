#include "ResourceDefines.hpp"

#include "Rendering\CommandBuffer.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\Pipeline.hpp"
#include "Resources\Settings.hpp"
#include "Resources\Resources.hpp"

// SAMPLER CREATION

SamplerInfo& SamplerInfo::SetMinMagMip(VkFilter min, VkFilter mag, VkSamplerMipmapMode mip)
{
	minFilter = min;
	magFilter = mag;
	mipFilter = mip;

	return *this;
}

SamplerInfo& SamplerInfo::SetAddressModeU(VkSamplerAddressMode u)
{
	addressModeU = u;

	return *this;
}

SamplerInfo& SamplerInfo::SetAddressModeUV(VkSamplerAddressMode u, VkSamplerAddressMode v)
{
	addressModeU = u;
	addressModeV = v;

	return *this;
}

SamplerInfo& SamplerInfo::SetAddressModeUVW(VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w)
{
	addressModeU = u;
	addressModeV = v;
	addressModeW = w;

	return *this;
}

SamplerInfo& SamplerInfo::SetName(const String& name)
{
	this->name = name;

	return *this;
}

// TEXTURE CREATION

TextureInfo& TextureInfo::SetSize(U16 width, U16 height, U16 depth)
{
	this->width = width;
	this->height = height;
	this->depth = depth;

	return *this;
}

TextureInfo& TextureInfo::SetFormatType(VkFormat format, VkImageType type)
{
	this->format = format;
	this->type = type;

	return *this;
}

TextureInfo& TextureInfo::SetName(const String& name)
{
	this->name = name;

	return *this;
}

TextureInfo& TextureInfo::SetData(void* data)
{
	initialData = data;

	return *this;
}

// RENDER PASS

void Renderpass::Resize()
{
	if (lastResize < Renderer::absoluteFrame)
	{
		lastResize = Renderer::absoluteFrame;

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

		Renderer::CreateRenderpass(this);
	}
}

// RENDER PASS CREATION

RenderpassInfo& RenderpassInfo::Reset()
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

RenderpassInfo& RenderpassInfo::AddRenderTarget(Texture* texture)
{
	outputTextures[renderTargetCount++] = texture;

	return *this;
}

RenderpassInfo& RenderpassInfo::SetDepthStencilTexture(Texture* texture)
{
	depthStencilTexture = texture;

	return *this;
}

RenderpassInfo& RenderpassInfo::SetName(const String& name)
{
	this->name = name;

	return *this;
}

RenderpassInfo& RenderpassInfo::SetOperations(VkAttachmentLoadOp color, VkAttachmentLoadOp depth, VkAttachmentLoadOp stencil)
{
	colorOperation = color;
	depthOperation = depth;
	stencilOperation = stencil;

	return *this;
}

RenderpassInfo& RenderpassInfo::AddClearColor(const Vector4& color)
{
	clears[clearCount++].color = { color.x, color.y, color.z, color.w };

	return *this;
}

RenderpassInfo& RenderpassInfo::AddClearDepth(F32 depth)
{
	clears[clearCount++].depthStencil = { depth, 0 };

	return *this;
}