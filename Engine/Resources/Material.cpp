#include "Material.hpp"

#include "Resources\Resources.hpp"
#include "Rendering\CommandBuffer.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\UI.hpp"

void RendergraphInfo::AddPipeline(const PipelineInfo& pipeline)
{
	bool found = false;
	for (U32 i = 0; i < pipelines.Size(); ++i)
	{
		if (pipeline.renderOrder < pipelines[i].renderOrder)
		{
			pipelines.Insert(i, pipeline);
			found = true;
			break;
		}
	}

	if (!found) { pipelines.Push(pipeline); }
}

void Rendergraph::SetBuffers(const BufferData& buffers)
{
	for (Pipeline& pipeline : pipelines) { pipeline.SetBuffers(buffers); }
}

bool Rendergraph::Create(RendergraphInfo& info)
{
	name = info.name;

	bool useVertices = false;
	bool useInstances = false;
	bool useIndices = false;

	U32 renderpass = 0;
	U32 subpass = 0;
	bool first = true;

	RenderpassInfo renderpassInfos[8];

	for (PipelineInfo& pipeline : info.pipelines)
	{
		useVertices |= pipeline.shader->useVertices;
		useInstances |= pipeline.shader->useInstancing;
		useIndices |= pipeline.shader->useIndexing;

		if (pipeline.shader->clearTypes || first) //TODO: Check for different render/depth targets
		{
			if (!first) { ++renderpass; }

			subpass = 0;

			RenderpassInfo& renderpassInfo = renderpassInfos[renderpass];

			renderpassInfo.Reset();
			renderpassInfo.name = info.name + renderpass;

			if (pipeline.renderTargetCount || pipeline.depthStencilTarget)
			{
				for (U32 i = 0; i < pipeline.renderTargetCount; ++i)
				{
					renderpassInfo.AddRenderTarget(pipeline.renderTargets[i]);
				}

				renderpassInfo.SetDepthStencilTarget(pipeline.depthStencilTarget);
			}
			else
			{
				renderpassInfo.AddRenderTarget(Renderer::defaultRenderTarget);

				if (pipeline.shader->depthEnable) { renderpassInfo.SetDepthStencilTarget(Renderer::defaultDepthTarget); }
			}

			renderpassInfo.colorLoadOp = pipeline.shader->clearTypes & CLEAR_TYPE_COLOR ? ATTACHMENT_LOAD_OP_CLEAR : ATTACHMENT_LOAD_OP_LOAD;
			renderpassInfo.depthLoadOp = pipeline.shader->clearTypes & CLEAR_TYPE_DEPTH ? ATTACHMENT_LOAD_OP_CLEAR : ATTACHMENT_LOAD_OP_LOAD;

			renderpassInfo.subpassCount = 1;

			if (pipeline.shader->subpass.inputAttachmentCount) { Logger::Error("First Shader In Renderpass Cannot Have Input Attachments!"); return false; }
		}

		pipeline.renderpass = renderpass;

		if (pipeline.shader->subpass.inputAttachmentCount)
		{
			++subpass;

			renderpassInfos[renderpass].AddSubpass(pipeline.shader->subpass);
		}

		pipeline.subpass = subpass;

		first = false;
	}

	Renderpass* prevRenderpass = nullptr;

	for (U32 i = 0; i <= renderpass; ++i)
	{
		renderpasses.Push({});
		Renderer::CreateRenderpass(&renderpasses.Back(), renderpassInfos[i], prevRenderpass);
		prevRenderpass = &renderpasses.Back();
	}

	for (PipelineInfo& pipeline : info.pipelines)
	{
		pipelines.Push({});
		pipelines.Back().Create(pipeline, &renderpasses[pipeline.renderpass]);
	}

	return true;
}

void Rendergraph::Run(CommandBuffer* commandBuffer, PipelineGroup* groups)
{
	U32 renderpass = U32_MAX;
	U32 subpass = 0;

	for (Pipeline& pipeline : pipelines)
	{
		if (!pipeline.pipeline) { break; }

		if (pipeline.renderpass != renderpass)
		{
			if (renderpass != U32_MAX) { commandBuffer->EndRenderpass(); }

			commandBuffer->BeginRenderpass(&renderpasses[pipeline.renderpass]);
			renderpass = pipeline.renderpass;
			subpass = 0;
		}
		else if (pipeline.subpass != subpass)
		{
			commandBuffer->NextSubpass();
			subpass = pipeline.subpass;
		}

		if (pipeline.type == MATERIAL_TYPE_INVALID) { pipeline.Run(commandBuffer, nullptr); }
		else { pipeline.Run(commandBuffer, groups + pipeline.type); }
	}

	commandBuffer->EndRenderpass();
}

void Rendergraph::Resize()
{
	for (Renderpass& renderpass : renderpasses)
	{
		Renderer::RecreateRenderpass(&renderpass);
	}
}