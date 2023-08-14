//
// Copyright (C) 2019 Google, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include <cstdio>
#include <cstdint>
#include <memory>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "../../SPIRV/GlslangToSpv.h"
#include "../../Public/ShaderLang.h"

#ifndef __EMSCRIPTEN__
#define EMSCRIPTEN_KEEPALIVE
#endif

const TBuiltInResource DefaultTBuiltInResource = {
    .maxLights = 32,
    .maxClipPlanes = 6,
    .maxTextureUnits = 32,
    .maxTextureCoords = 32,
    .maxVertexAttribs = 64,
    .maxVertexUniformComponents = 4096,
    .maxVaryingFloats = 64,
    .maxVertexTextureImageUnits = 32,
    .maxCombinedTextureImageUnits = 80,
    .maxTextureImageUnits = 32,
    .maxFragmentUniformComponents = 4096,
    .maxDrawBuffers = 32,
    .maxVertexUniformVectors = 128,
    .maxVaryingVectors = 8,
    .maxFragmentUniformVectors = 16,
    .maxVertexOutputVectors = 16,
    .maxFragmentInputVectors = 15,
    .minProgramTexelOffset = -8,
    .maxProgramTexelOffset = 7,
    .maxClipDistances = 8,
    .maxComputeWorkGroupCountX = 65535,
    .maxComputeWorkGroupCountY = 65535,
    .maxComputeWorkGroupCountZ = 65535,
    .maxComputeWorkGroupSizeX = 1024,
    .maxComputeWorkGroupSizeY = 1024,
    .maxComputeWorkGroupSizeZ = 64,
    .maxComputeUniformComponents = 1024,
    .maxComputeTextureImageUnits = 16,
    .maxComputeImageUniforms = 8,
    .maxComputeAtomicCounters = 8,
    .maxComputeAtomicCounterBuffers = 1,
    .maxVaryingComponents = 60,
    .maxVertexOutputComponents = 64,
    .maxGeometryInputComponents = 64,
    .maxGeometryOutputComponents = 128,
    .maxFragmentInputComponents = 128,
    .maxImageUnits = 8,
    .maxCombinedImageUnitsAndFragmentOutputs = 8,
    .maxCombinedShaderOutputResources = 8,
    .maxImageSamples = 0,
    .maxVertexImageUniforms = 0,
    .maxTessControlImageUniforms = 0,
    .maxTessEvaluationImageUniforms = 0,
    .maxGeometryImageUniforms = 0,
    .maxFragmentImageUniforms = 8,
    .maxCombinedImageUniforms = 8,
    .maxGeometryTextureImageUnits = 16,
    .maxGeometryOutputVertices = 256,
    .maxGeometryTotalOutputComponents = 1024,
    .maxGeometryUniformComponents = 1024,
    .maxGeometryVaryingComponents = 64,
    .maxTessControlInputComponents = 128,
    .maxTessControlOutputComponents = 128,
    .maxTessControlTextureImageUnits = 16,
    .maxTessControlUniformComponents = 1024,
    .maxTessControlTotalOutputComponents = 4096,
    .maxTessEvaluationInputComponents = 128,
    .maxTessEvaluationOutputComponents = 128,
    .maxTessEvaluationTextureImageUnits = 16,
    .maxTessEvaluationUniformComponents = 1024,
    .maxTessPatchComponents = 120,
    .maxPatchVertices = 32,
    .maxTessGenLevel = 64,
    .maxViewports = 16,
    .maxVertexAtomicCounters = 0,
    .maxTessControlAtomicCounters = 0,
    .maxTessEvaluationAtomicCounters = 0,
    .maxGeometryAtomicCounters = 0,
    .maxFragmentAtomicCounters = 8,
    .maxCombinedAtomicCounters = 8,
    .maxAtomicCounterBindings = 1,
    .maxVertexAtomicCounterBuffers = 0,
    .maxTessControlAtomicCounterBuffers = 0,
    .maxTessEvaluationAtomicCounterBuffers = 0,
    .maxGeometryAtomicCounterBuffers = 0,
    .maxFragmentAtomicCounterBuffers = 1,
    .maxCombinedAtomicCounterBuffers = 1,
    .maxAtomicCounterBufferSize = 16384,
    .maxTransformFeedbackBuffers = 4,
    .maxTransformFeedbackInterleavedComponents = 64,
    .maxCullDistances = 8,
    .maxCombinedClipAndCullDistances = 8,
    .maxSamples = 4,
    .maxMeshOutputVerticesNV = 256,
    .maxMeshOutputPrimitivesNV = 512,
    .maxMeshWorkGroupSizeX_NV = 32,
    .maxMeshWorkGroupSizeY_NV = 1,
    .maxMeshWorkGroupSizeZ_NV = 1,
    .maxTaskWorkGroupSizeX_NV = 32,
    .maxTaskWorkGroupSizeY_NV = 1,
    .maxTaskWorkGroupSizeZ_NV = 1,
    .maxMeshViewCountNV = 4,
    .maxDualSourceDrawBuffersEXT = 1,

    .limits = {
        .nonInductiveForLoops = 1,
        .whileLoops = 1,
        .doWhileLoops = 1,
        .generalUniformIndexing = 1,
        .generalAttributeMatrixVectorIndexing = 1,
        .generalVaryingIndexing = 1,
        .generalSamplerIndexing = 1,
        .generalVariableIndexing = 1,
        .generalConstantMatrixVectorIndexing = 1,
}};

static bool initialized = false;

extern "C" {

/*
 * Takes in a GLSL shader as a string and converts it to SPIR-V in binary form.
 *
 * |glsl|          Null-terminated string containing the shader to be converted.
 * |stage_int|     Magic number indicating the type of shader being processed.
*                  Legal values are as follows:
 *                   Vertex = 0
 *                   Fragment = 4
 *                   Compute = 5
 * |gen_debug|     Flag to indicate if debug information should be generated.
 * |spirv|         Output parameter for a pointer to the resulting SPIR-V data.
 * |spirv_len|     Output parameter for the length of the output binary buffer.
 *
 * Returns a void* pointer which, if not null, must be destroyed by
 * destroy_output_buffer.o. (This is not the same pointer returned in |spirv|.)
 * If null, the compilation failed.
 */
EMSCRIPTEN_KEEPALIVE
void* convert_glsl_to_spirv(const char* glsl,
                            int stage_int,
                            bool gen_debug,
                            glslang::EShTargetLanguageVersion spirv_version,
                            uint32_t** spirv,
                            size_t* spirv_len)
{
    if (glsl == nullptr) {
        fprintf(stderr, "Input pointer null\n");
        return nullptr;
    }
    if (spirv == nullptr || spirv_len == nullptr) {
        fprintf(stderr, "Output pointer null\n");
        return nullptr;
    }
    *spirv = nullptr;
    *spirv_len = 0;

    if (stage_int != 0 && stage_int != 4 && stage_int != 5) {
        fprintf(stderr, "Invalid shader stage\n");
        return nullptr;
    }
    EShLanguage stage = static_cast<EShLanguage>(stage_int);
    switch (spirv_version) {
        case glslang::EShTargetSpv_1_0:
        case glslang::EShTargetSpv_1_1:
        case glslang::EShTargetSpv_1_2:
        case glslang::EShTargetSpv_1_3:
        case glslang::EShTargetSpv_1_4:
        case glslang::EShTargetSpv_1_5:
            break;
        default:
            fprintf(stderr, "Invalid SPIR-V version number\n");
            return nullptr;
    }

    if (!initialized) {
        glslang::InitializeProcess();
        initialized = true;
    }

    glslang::TShader shader(stage);
    shader.setStrings(&glsl, 1);
    shader.setEnvInput(glslang::EShSourceGlsl, stage, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
    shader.setEnvTarget(glslang::EShTargetSpv, spirv_version);
    if (!shader.parse(&DefaultTBuiltInResource, 100, true, EShMsgDefault)) {
        fprintf(stderr, "Parse failed\n");
        fprintf(stderr, "%s\n", shader.getInfoLog());
        return nullptr;
    }

    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(EShMsgDefault)) {
        fprintf(stderr, "Link failed\n");
        fprintf(stderr, "%s\n", program.getInfoLog());
        return nullptr;
    }

    glslang::SpvOptions spvOptions;
    spvOptions.generateDebugInfo = gen_debug;
    spvOptions.optimizeSize = false;
    spvOptions.disassemble = false;
    spvOptions.validate = false;

    std::vector<uint32_t>* output = new std::vector<uint32_t>;
    glslang::GlslangToSpv(*program.getIntermediate(stage), *output, nullptr, &spvOptions);

    *spirv_len = output->size();
    *spirv = output->data();
    return output;
}

/*
 * Destroys a buffer created by convert_glsl_to_spirv
 */
EMSCRIPTEN_KEEPALIVE
void destroy_output_buffer(void* p)
{
    delete static_cast<std::vector<uint32_t>*>(p);
}

}  // extern "C"

/*
 * For non-Emscripten builds we supply a generic main, so that the glslang.js
 * build target can generate an executable with a trivial use case instead of
 * generating a WASM binary. This is done so that there is a target that can be
 * built and output analyzed using desktop tools, since WASM binaries are
 * specific to the Emscripten toolchain.
 */
#ifndef __EMSCRIPTEN__
int main() {
    const char* input = R"(#version 310 es

void main() { })";

    uint32_t* output;
    size_t output_len;

    void* id = convert_glsl_to_spirv(input, 4, false, glslang::EShTargetSpv_1_0, &output, &output_len);
    assert(output != nullptr);
    assert(output_len != 0);
    destroy_output_buffer(id);
    return 0;
}
#endif  // ifndef __EMSCRIPTEN__
