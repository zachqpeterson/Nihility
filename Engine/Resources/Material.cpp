#include "Material.hpp"

#include "Resources\Resources.hpp"
#include "Rendering\CommandBuffer.hpp"
#include "Rendering\Renderer.hpp"
#include "Rendering\UI.hpp"

//bool Rendergraph::Create(RendergraphInfo& info)
//{
//	name = info.name;
//
//	bool useVertices = false;
//	bool useInstances = false;
//	bool useIndices = false;
//
//	U32 renderpass = 0;
//	U32 subpass = 0;
//	bool first = true;
//
//	RenderpassInfo renderpassInfos[8];
//
//	for (PipelineInfo& pipeline : info.pipelines)
//	{
//		useVertices |= pipeline.shader->useVertices;
//		useInstances |= pipeline.shader->useInstancing;
//		useIndices |= pipeline.shader->useIndexing;
//
//		if (pipeline.shader->clearTypes || first) //TODO: Check for different render/depth targets
//		{
//			if (!first) { ++renderpass; }
//
//			subpass = 0;
//
//			RenderpassInfo& renderpassInfo = renderpassInfos[renderpass];
//
//			renderpassInfo.Reset();
//			renderpassInfo.name = info.name + renderpass;
//			renderpassInfo.resize = pipeline.resize;
//
//			if (pipeline.renderTargetCount || pipeline.depthStencilTarget)
//			{
//				for (U32 i = 0; i < pipeline.renderTargetCount; ++i)
//				{
//					renderpassInfo.AddRenderTarget(pipeline.renderTargets[i]);
//					renderpassInfo.renderArea = { { 0, 0 }, { pipeline.renderTargets[i]->width, pipeline.renderTargets[i]->height } };
//				}
//
//				if (pipeline.depthStencilTarget)
//				{
//					renderpassInfo.SetDepthStencilTarget(pipeline.depthStencilTarget);
//					renderpassInfo.renderArea = { { 0, 0 }, { pipeline.depthStencilTarget->width, pipeline.depthStencilTarget->height } };
//				}
//			}
//			else
//			{
//				renderpassInfo.AddRenderTarget(Renderer::defaultRenderTarget);
//
//				if (pipeline.shader->depthEnable) { renderpassInfo.SetDepthStencilTarget(Renderer::defaultDepthTarget); }
//
//				renderpassInfo.renderArea = { { 0, 0 }, { Renderer::defaultRenderTarget->width, Renderer::defaultRenderTarget->height } };
//			}
//
//			renderpassInfo.colorLoadOp = pipeline.shader->clearTypes & CLEAR_TYPE_COLOR ? ATTACHMENT_LOAD_OP_CLEAR : ATTACHMENT_LOAD_OP_LOAD;
//			renderpassInfo.depthLoadOp = pipeline.shader->clearTypes & CLEAR_TYPE_DEPTH ? ATTACHMENT_LOAD_OP_CLEAR : ATTACHMENT_LOAD_OP_LOAD;
//
//			renderpassInfo.subpassCount = 1;
//
//			if (pipeline.shader->subpass.inputAttachmentCount) { Logger::Error("First Shader In Renderpass Cannot Have Input Attachments!"); return false; }
//		}
//
//		pipeline.renderpass = renderpass;
//
//		if (pipeline.shader->subpass.inputAttachmentCount)
//		{
//			++subpass;
//
//			renderpassInfos[renderpass].AddSubpass(pipeline.shader->subpass);
//		}
//
//		pipeline.subpass = subpass;
//
//		first = false;
//	}
//
//	Renderpass* prevRenderpass = nullptr;
//
//	for (U32 i = 0; i <= renderpass; ++i)
//	{
//		Renderpass* renderpass = &renderpasses.Push({});
//		Renderer::CreateRenderpass(renderpass, renderpassInfos[i], prevRenderpass);
//		prevRenderpass = renderpass;
//	}
//
//	U8 i = 0;
//	for (PipelineInfo& pipelineInfo : info.pipelines)
//	{
//		Pipeline& pipeline = pipelines.Push({});
//		pipeline.Create(pipelineInfo, &renderpasses[pipelineInfo.renderpass]);
//
//		++i;
//	}
//
//	return true;
//}
//
//void Rendergraph::Run(CommandBuffer* commandBuffer)
//{
//	U32 renderpass = U32_MAX;
//	U32 subpass = 0;
//
//	for (Pipeline& pipeline : pipelines)
//	{
//		if (!pipeline.pipeline) { break; }
//
//		if (pipeline.renderpass != renderpass)
//		{
//			if (renderpass != U32_MAX) { commandBuffer->EndRenderpass(); }
//
//			commandBuffer->BeginRenderpass(&renderpasses[pipeline.renderpass]);
//			renderpass = pipeline.renderpass;
//			subpass = 0;
//		}
//		else if (pipeline.subpass != subpass)
//		{
//			commandBuffer->NextSubpass();
//			subpass = pipeline.subpass;
//		}
//
//		pipeline.Run(commandBuffer);
//	}
//
//	commandBuffer->EndRenderpass();
//}