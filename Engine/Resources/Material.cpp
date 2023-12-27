#include "Material.hpp"

#include "Resources\Resources.hpp"
#include "Rendering\CommandBuffer.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\UI.hpp"

void RendergraphInfo::AddPipeline(const RenderStage& stage)
{
	if (stage.type >= RENDER_STAGE_COUNT)
	{
		Logger::Error("Invalid Render Stage Supplied To RendergraphInfo!");
		return;
	}

	if (stages[stage.type][stage.index].type != RENDER_STAGE_INVALID)
	{
		Logger::Warn("Overriding Render Stage!"); //TODO: Better warning message
	}

	stages[stage.type][stage.index] = stage;
}

U32 Rendergraph::InstanceSize(RenderStageType type, U32 index) const { if (pipelines[type][index].shader) { return pipelines[type][index].shader->instanceStride; } return 0; }

U32 Rendergraph::VertexSize(RenderStageType type, U32 index) const { if (pipelines[type][index].shader) { return pipelines[type][index].shader->vertexSize; } return 0; }

Pipeline* Rendergraph::GetPipeline(RenderStageType type, U32 index) { return &(pipelines[type][index]); }

bool Rendergraph::Create(RendergraphInfo& info)
{
	name = info.name;

	bool useVertices = false;
	bool useInstances = false;
	bool useIndices = false;

	U32 renderpass = 0;
	U32 subpass = 0;
	bool first = true;

	RenderpassInfo renderpassInfos[RENDER_STAGE_COUNT];

	for (U32 i = 0; i < RENDER_STAGE_COUNT; ++i)
	{
		for (U32 j = 0; j < MAX_PIPELINES_PER_STAGE; ++j)
		{
			RenderStage& stage = info.stages[i][j];

			if (stage.type == RENDER_STAGE_INVALID) { break; }

			useVertices |= stage.info.shader->useVertices;
			useInstances |= stage.info.shader->useInstancing;
			useIndices |= stage.info.shader->useIndexing;

			if (stage.info.shader->clearTypes || first)
			{
				if(!first){ ++renderpass; }

				subpass = 0;

				renderpassInfos[renderpass].Reset();
				renderpassInfos[renderpass].name = info.name + renderpass;
				renderpassInfos[renderpass].AddRenderTarget(Renderer::defaultRenderTarget);

				if (stage.info.shader->depthEnable) { renderpassInfos[renderpass].SetDepthStencilTarget(Renderer::defaultDepthTarget); }

				renderpassInfos[renderpass].colorLoadOp = stage.info.shader->clearTypes & CLEAR_TYPE_COLOR ? ATTACHMENT_LOAD_OP_CLEAR : ATTACHMENT_LOAD_OP_LOAD;
				renderpassInfos[renderpass].depthLoadOp = stage.info.shader->clearTypes & CLEAR_TYPE_DEPTH ? ATTACHMENT_LOAD_OP_CLEAR : ATTACHMENT_LOAD_OP_LOAD;

				renderpassInfos[renderpass].subpassCount = 1;

				if (stage.info.shader->subpass.inputAttachmentCount) { Logger::Error("First Shader In Renderpass Cannot Have Input Attachments!"); return false; }
			}

			stage.info.renderpass = renderpass;

			if (stage.info.shader->subpass.inputAttachmentCount)
			{
				++subpass;

				renderpassInfos[renderpass].AddSubpass(stage.info.shader->subpass);
			}

			stage.info.subpass = subpass;

			first = false;
		}
	}

	for (U32 i = 0; i <= renderpass; ++i)
	{
		Renderer::CreateRenderpass(&renderpasses[i], renderpassInfos[i]);
	}

	for (U32 i = 0; i < RENDER_STAGE_COUNT; ++i)
	{
		for (U32 j = 0; j < MAX_PIPELINES_PER_STAGE; ++j)
		{
			RenderStage& stage = info.stages[i][j];

			if (stage.type == RENDER_STAGE_INVALID) { break; }

			pipelines[i][j].Create(stage.info, &renderpasses[stage.info.renderpass]);
		}
	}

	return true;
}

void Rendergraph::Run(CommandBuffer* commandBuffer, const Buffer& vertexBuffer, const Buffer& instanceBuffers, const Buffer& indexBuffer, const Buffer& drawBuffer, const Buffer& countsBuffer)
{
	U32 renderpass = U32_MAX;
	U32 subpass = 0;

	for (U32 i = 0; i < RENDER_STAGE_COUNT; ++i)
	{
		for (U32 j = 0; j < MAX_PIPELINES_PER_STAGE; ++j)
		{
			Pipeline& pipeline = pipelines[i][j];

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

			pipeline.Run(commandBuffer, vertexBuffer, instanceBuffers, indexBuffer, drawBuffer, countsBuffer);
		}
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