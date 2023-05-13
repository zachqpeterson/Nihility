#include "Engine.hpp"

#include "Rendering\Renderer.hpp"
#include "Math\Math.hpp"
#include "Resources\Resources.hpp"

BufferHandle                    cubeVertexBuffer;
BufferHandle                    cubeIndexBuffer;
PipelineHandle                  cubePipeline;
BufferHandle                    cubeConstantBuffer;
DescriptorSetLayoutHandle       cubeDescriptorSetLayout;

struct UniformData 
{
	Matrix4 m;
	Matrix4 vp;
	Vector4 eye;
	Vector4 light;
};

bool Init()
{
	// Create pipeline state
	PipelineCreation pipelineCreation;

	// Vertex input
	// TODO(marco): component format should be based on buffer view type
	pipelineCreation.vertexInput.AddVertexAttribute({ 0, 0, 0, VERTEX_COMPONENT_FLOAT3 }); // position
	pipelineCreation.vertexInput.AddVertexStream({ 0, 12, VERTEX_INPUT_RATE_VERTEX });

	pipelineCreation.vertexInput.AddVertexAttribute({ 1, 1, 0, VERTEX_COMPONENT_FLOAT4 }); // tangent
	pipelineCreation.vertexInput.AddVertexStream({ 1, 16, VERTEX_INPUT_RATE_VERTEX });

	pipelineCreation.vertexInput.AddVertexAttribute({ 2, 2, 0, VERTEX_COMPONENT_FLOAT3 }); // normal
	pipelineCreation.vertexInput.AddVertexStream({ 2, 12, VERTEX_INPUT_RATE_VERTEX });

	pipelineCreation.vertexInput.AddVertexAttribute({ 3, 3, 0, VERTEX_COMPONENT_FLOAT2 }); // texcoord
	pipelineCreation.vertexInput.AddVertexStream({ 3, 8, VERTEX_INPUT_RATE_VERTEX });

	// Render pass
	pipelineCreation.renderPass = Renderer::GetSwapchainOutput();
	// Depth
	pipelineCreation.depthStencil.SetDepth(true, VK_COMPARE_OP_LESS_OR_EQUAL);

	pipelineCreation.shaders.
		SetName("Cube").
		AddStage("Cube.vert", VK_SHADER_STAGE_VERTEX_BIT).
		AddStage("Cube.frag", VK_SHADER_STAGE_FRAGMENT_BIT);

	// Descriptor set layout
	DescriptorSetLayoutCreation cubeRllCreation{};
	cubeRllCreation.AddBinding({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, "LocalConstants" });
	cubeRllCreation.AddBinding({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, 1, "MaterialConstant" });
	cubeRllCreation.AddBinding({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, 1, "diffuseTexture" });
	cubeRllCreation.AddBinding({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3, 1, "roughnessMetalnessTexture" });
	cubeRllCreation.AddBinding({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, 1, "roughnessMetalnessTexture" });
	cubeRllCreation.AddBinding({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5, 1, "emissiveTexture" });
	cubeRllCreation.AddBinding({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6, 1, "occlusionTexture" });
	// Setting it into pipeline
	cubeDescriptorSetLayout = Renderer::CreateDescriptorSetLayout(cubeRllCreation);
	pipelineCreation.AddDescriptorSetLayout(cubeDescriptorSetLayout);

	// Constant buffer
	BufferCreation bufferCreation;
	bufferCreation.Reset().Set(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, RESOURCE_USAGE_DYNAMIC, sizeof(UniformData)).SetName("cube_cb");
	cubeConstantBuffer = Renderer::CreateBuffer(bufferCreation);

	cubePipeline = Renderer::CreatePipeline(pipelineCreation);

	return true;
}

void Update()
{

}

void Shutdown()
{

}

int main(int argc, char** argv)
{
	Engine::Initialize("Nihility Demo", MakeVersionNumber(0, 1, 0), Init, Update, Shutdown);

	return 0;
}