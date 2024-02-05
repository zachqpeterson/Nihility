#include "Material.hpp"

#include "Resources\Resources.hpp"
#include "Rendering\CommandBuffer.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\UI.hpp"

void RendergraphInfo::AddPipeline(const PipelineInfo& pipeline)
{
	if (pipeline.type & PIPELINE_TYPE_DEFAULT)
	{
		if (pipeline.type & PIPELINE_TYPE_MESH_OPAQUE)
		{
			if (hasDefaultOpaque) { Logger::Warn("Rendergraphs Cannot Have Multiple Default Pipelines Of The Same Type, Ignoring Duplicates..."); }
			hasDefaultOpaque = true;
		}
		if (pipeline.type & PIPELINE_TYPE_MESH_TRANSPARENT)
		{
			if (hasDefaultTransparent) { Logger::Warn("Rendergraphs Cannot Have Multiple Default Pipelines Of The Same Type, Ignoring Duplicates..."); }
			hasDefaultTransparent = true;
		}
	}

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

void Rendergraph::AddPreprocessing(Pipeline* pipeline)
{
	for (Pipeline& p : pipelines)
	{
		if (((pipeline->type & PIPELINE_TYPE_MESH_OPAQUE) && (p.type & PIPELINE_TYPE_PRE_PROCESSING_OPAQUE)) ||
			((pipeline->type & PIPELINE_TYPE_MESH_TRANSPARENT) && (p.type & PIPELINE_TYPE_PRE_PROCESSING_TRANSPARENT)))
		{
			p.drawSets.Push(pipeline->drawSets.Front());
		}
	}
}

Pipeline* Rendergraph::GetPipeline(U8 id) { return &pipelines[id]; }
Pipeline* Rendergraph::DefaultOpaqueMeshPipeline() { if (defaultOpaque != U8_MAX) { return &(pipelines[defaultOpaque]); } return nullptr; }
Pipeline* Rendergraph::DefaultTransparentMeshPipeline() { if (defaultTransparent != U8_MAX) { return &(pipelines[defaultTransparent]); } return nullptr; }
U8 Rendergraph::DefaultOpaqueMeshPipelineID() const { return defaultOpaque; }
U8 Rendergraph::DefaultTransparentMeshPipelineID() const { return defaultTransparent; }

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
			renderpassInfo.resize = pipeline.resize;

			if (pipeline.renderTargetCount || pipeline.depthStencilTarget)
			{
				for (U32 i = 0; i < pipeline.renderTargetCount; ++i)
				{
					renderpassInfo.AddRenderTarget(pipeline.renderTargets[i]);
					renderpassInfo.renderArea = { { 0, 0 }, { pipeline.renderTargets[i]->width, pipeline.renderTargets[i]->height } };
				}

				if (pipeline.depthStencilTarget)
				{
					renderpassInfo.SetDepthStencilTarget(pipeline.depthStencilTarget);
					renderpassInfo.renderArea = { { 0, 0 }, { pipeline.depthStencilTarget->width, pipeline.depthStencilTarget->height } };
				}
			}
			else
			{
				renderpassInfo.AddRenderTarget(Renderer::defaultRenderTarget);

				if (pipeline.shader->depthEnable) { renderpassInfo.SetDepthStencilTarget(Renderer::defaultDepthTarget); }

				renderpassInfo.renderArea = { { 0, 0 }, { Renderer::defaultRenderTarget->width, Renderer::defaultRenderTarget->height } };
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
		Renderpass* renderpass = &renderpasses.Push({});
		Renderer::CreateRenderpass(renderpass, renderpassInfos[i], prevRenderpass);
		prevRenderpass = renderpass;
	}

	U8 i = 0;
	for (PipelineInfo& pipelineInfo : info.pipelines)
	{
		Pipeline& pipeline = pipelines.Push({});
		pipeline.Create(pipelineInfo, &renderpasses[pipelineInfo.renderpass]);

		if (pipeline.type & PIPELINE_TYPE_DEFAULT)
		{
			if (pipeline.type & PIPELINE_TYPE_MESH_OPAQUE) { defaultOpaque = i; }
			if (pipeline.type & PIPELINE_TYPE_MESH_TRANSPARENT) { defaultTransparent = i; }
		}

		++i;
	}

	return true;
}

void Rendergraph::Run(CommandBuffer* commandBuffer)
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

		pipeline.Run(commandBuffer);
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