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

		Vector4 area = Renderer::RenderArea();

		for (U32 i = 0; i < renderTargetCount; ++i)
		{
			Resources::RecreateTexture(renderTargets[i], Settings::WindowWidth(), Settings::WindowHeight(), 1);
		}

		if (depthStencilTarget)
		{
			Resources::RecreateTexture(depthStencilTarget, Settings::WindowWidth(), Settings::WindowHeight(), 1);
		}

		vkDestroyFramebuffer(Renderer::device, frameBuffer, Renderer::allocationCallbacks);
		vkDestroyRenderPass(Renderer::device, renderpass, Renderer::allocationCallbacks);

		Renderer::CreateRenderpass(this);
	}
}

// RENDER PASS CREATION

RenderpassInfo& RenderpassInfo::Reset()
{
	renderTargetCount = 0;

	depthStencilTarget = {};

	colorLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;

	name.Clear();

	return *this;
}

RenderpassInfo& RenderpassInfo::AddRenderTarget(Texture* texture)
{
	renderTargets[renderTargetCount++] = texture;

	return *this;
}

RenderpassInfo& RenderpassInfo::SetDepthStencilTarget(Texture* texture)
{
	depthStencilTarget = texture;

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