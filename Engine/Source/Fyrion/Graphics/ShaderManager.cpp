#include "ShaderManager.hpp"
#include "Fyrion/Platform/Platform.hpp"

#if defined(FY_WIN)
#include "Windows.h"
#else
#include "dxc/WinAdapter.h"
#endif

#include <iostream>

#include "spirv_cross.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "dxc/dxcapi.h"
#include "Fyrion/Core/HashMap.hpp"

#define SHADER_MODEL "6_5"

#define COMPUTE_SHADER_MODEL    L"cs_" SHADER_MODEL
#define DOMAIN_SHADER_MODEL     L"ds_" SHADER_MODEL
#define GEOMETRY_SHADER_MODEL   L"gs_" SHADER_MODEL
#define HULL_SHADER_MODEL       L"hs_" SHADER_MODEL
#define PIXEL_SHADER_MODELS     L"ps_" SHADER_MODEL
#define VERTEX_SHADER_MODEL     L"vs_" SHADER_MODEL
#define MESH_SHADER_MODEL              L"ms_" SHADER_MODEL
#define AMPLIFICATION_SHADER_MODEL     L"as_" SHADER_MODEL
#define LIB_SHADER_MODEL        L"lib_" SHADER_MODEL

namespace Fyrion
{
    namespace
    {
        Logger&               logger = Logger::GetLogger("Fyrion::ShaderCompiler");

        IDxcUtils*            utils{};
        IDxcCompiler3*        compiler{};
        DxcCreateInstanceProc dxcCreateInstance;
    }

    constexpr auto GetShaderStage(ShaderStage shader)
    {
        switch (shader)
        {
        case ShaderStage::Vertex: return VERTEX_SHADER_MODEL;
        case ShaderStage::Hull: return HULL_SHADER_MODEL;
        case ShaderStage::Domain: return DOMAIN_SHADER_MODEL;
        case ShaderStage::Geometry: return GEOMETRY_SHADER_MODEL;
        case ShaderStage::Pixel: return PIXEL_SHADER_MODELS;
        case ShaderStage::Compute: return COMPUTE_SHADER_MODEL;
        case ShaderStage::Amplification: return AMPLIFICATION_SHADER_MODEL;
        case ShaderStage::Mesh: return MESH_SHADER_MODEL;
        case ShaderStage::RayGen:
        case ShaderStage::RayIntersection:
        case ShaderStage::RayAnyHit:
        case ShaderStage::RayClosestHit:
        case ShaderStage::RayMiss:
        case ShaderStage::Callable:
        case ShaderStage::All:
            return LIB_SHADER_MODEL;
        default:
            break;
        }
        FY_ASSERT(false, "[ShaderCompiler] shader stage not found");
        return L"";
    }


    void ShaderManagerInit()
    {
        VoidPtr instance = Platform::LoadDynamicLib("dxcompiler");
        if (instance)
        {
            dxcCreateInstance = reinterpret_cast<DxcCreateInstanceProc>(Platform::GetFunctionAddress(instance, "DxcCreateInstance"));
            dxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));
            dxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));
        }
    }

    bool ShaderManager::CompileShader(const ShaderCreation& shaderCreation, Array<u8>& bytes)
    {
        bool shaderCompilerValid = utils && compiler;
        if (!shaderCompilerValid)
        {
            logger.Warn("DxShaderCompiler not loaded");
            return false;
        }

        IDxcBlobEncoding* pSource = {};
        utils->CreateBlob(shaderCreation.source.CStr(), shaderCreation.source.Size(), CP_UTF8, &pSource);

        DxcBuffer source{};
        source.Ptr = pSource->GetBufferPointer();
        source.Size = pSource->GetBufferSize();
        source.Encoding = DXC_CP_ACP;

        std::wstring entryPoint = std::wstring{shaderCreation.entryPoint.begin(), shaderCreation.entryPoint.end()};

        Array<LPCWSTR> args;
        args.EmplaceBack(L"-E");
        args.EmplaceBack(entryPoint.c_str());
        args.EmplaceBack(L"-Wno-ignored-attributes");

        args.EmplaceBack(L"-T");
        args.EmplaceBack(GetShaderStage(shaderCreation.shaderStage));

        if (shaderCreation.renderApi != RenderApiType::D3D12)
        {
            args.EmplaceBack(L"-spirv");
            args.EmplaceBack(L"-fspv-target-env=vulkan1.2");
        }

        IDxcResult* pResults{};
        //IncludeHandler includeHandler{rid};
        compiler->Compile(&source, args.Data(), args.Size(), nullptr, IID_PPV_ARGS(&pResults));

        IDxcBlobUtf8* pErrors = {};
        pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
        if (pErrors != nullptr && pErrors->GetStringLength() != 0)
        {
            logger.Error("Error on compile shader {} ", pErrors->GetStringPointer());
            return false;
        }

        HRESULT hrStatus;
        pResults->GetStatus(&hrStatus);
        if (FAILED(hrStatus))
        {
            logger.Error("Compilation Failed ");
            return false;
        }

        IDxcBlob*     pShader = nullptr;
        IDxcBlobWide* pShaderName = nullptr;
        pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderName);

        usize offset = bytes.Size();
        bytes.Resize(bytes.Size() + pShader->GetBufferSize());
        auto buffer = static_cast<u8*>(pShader->GetBufferPointer());

        for (usize i = 0; i < pShader->GetBufferSize(); ++i)
        {
            bytes[offset + i] = buffer[i];
        }

        pResults->Release();
        pShader->Release();
        if (pShaderName)
        {
            pShaderName->Release();
        }
        pSource->Release();

        return true;
    }

    Format CastFormat(spirv_cross::SPIRType type)
    {
        switch (type.basetype)
        {
        case spirv_cross::SPIRType::SByte: break;
        case spirv_cross::SPIRType::UByte:
        case spirv_cross::SPIRType::Boolean:
            switch (type.vecsize)
            {
            case 1: return Format::R;
            case 2: return Format::RG;
            case 3: return Format::RGB;
            case 4: return Format::RGBA;
            }
            break;
        case spirv_cross::SPIRType::Short: break;
        case spirv_cross::SPIRType::UShort: break;
        case spirv_cross::SPIRType::Int: break;
        case spirv_cross::SPIRType::UInt: break;
        case spirv_cross::SPIRType::Int64: break;
        case spirv_cross::SPIRType::UInt64: break;
        case spirv_cross::SPIRType::Half: break;
        case spirv_cross::SPIRType::Float:
            switch (type.vecsize)
            {
            case 1: return Format::R32F;
            case 2: return Format::RG32F;
            case 3: return Format::RGB32F;
            case 4: return Format::RGBA32F;
            }
            break;
        case spirv_cross::SPIRType::Double:
            break;
        case spirv_cross::SPIRType::Char:
            break;
        case spirv_cross::SPIRType::Unknown:
        case spirv_cross::SPIRType::Void:
        case spirv_cross::SPIRType::Image:
        case spirv_cross::SPIRType::SampledImage:
        case spirv_cross::SPIRType::Sampler:
        case spirv_cross::SPIRType::AccelerationStructure:
        case spirv_cross::SPIRType::RayQuery:
        case spirv_cross::SPIRType::ControlPointArray:
        case spirv_cross::SPIRType::Interpolant:
        case spirv_cross::SPIRType::Struct:
        case spirv_cross::SPIRType::AtomicCounter:
            break;
        }
        return Format::Undefined;
    }

    void ReadResources(const spirv_cross::Compiler&                           comp,
                       const spirv_cross::SmallVector<spirv_cross::Resource>& resources,
                       HashMap<u32, HashMap<u32, DescriptorBinding>>&         descriptors)
    {
        for (auto& resource : resources)
        {
            u32 set = comp.get_decoration(resource.id, spv::DecorationDescriptorSet);
            u32 binding = comp.get_decoration(resource.id, spv::DecorationBinding);

            auto setIt = descriptors.Find(set);
            if (setIt == descriptors.end())
            {
                setIt = descriptors.Insert({set, HashMap<u32, DescriptorBinding>{}}).first;
            }
            auto& descriptorMap = setIt->second;
            if (auto it = descriptorMap.Find(binding); it == descriptorMap.end())
            {
                DescriptorBinding& descriptorBinding = descriptorMap.Emplace(binding, DescriptorBinding{
                    .binding = binding,
                }).first->second;
            }
            // std::string name = comp.get_name(resource.id);
            // std::cout << name << std::endl;
        }
    }

    ShaderInfo ShaderManager::ExtractShaderInfo(const Span<u8>& bytes, const Span<ShaderStageInfo>& stages, RenderApiType renderApi)
    {
        ShaderInfo shaderInfo;

        if (renderApi != RenderApiType::D3D12)
        {
            HashMap<u32, HashMap<u32, DescriptorBinding>> descriptors{};

            for (const ShaderStageInfo& stageInfo : stages)
            {
                std::vector<u32> data;
                data.resize(stageInfo.size / sizeof(u32));
                MemCopy(data.data(), bytes.Data() + stageInfo.offset, stageInfo.size);
                spirv_cross::Compiler compiler(data);
                const spirv_cross::ShaderResources& resources = compiler.get_shader_resources();


                if (stageInfo.stage == ShaderStage::Vertex)
                {
                    for (const auto& input : resources.stage_inputs)
                    {
                        Format format = CastFormat(compiler.get_type(input.base_type_id));

                        shaderInfo.inputVariables.EmplaceBack(InterfaceVariable{
                            .location = compiler.get_decoration(input.id, spv::DecorationLocation),
                            .offset = compiler.get_decoration(input.id, spv::DecorationOffset),
                            .name = compiler.get_decoration(input.id, spv::DecorationHlslSemanticGOOGLE),
                            .format = format,
                            .size = GetFormatSize(format),
                        });
                    }
                }

                if (stageInfo.stage == ShaderStage::Pixel)
                {
                    for (const auto& output : resources.stage_outputs)
                    {
                        Format format = CastFormat(compiler.get_type(output.base_type_id));

                        shaderInfo.outputVariables.EmplaceBack(InterfaceVariable{
                            .location = compiler.get_decoration(output.id, spv::DecorationLocation),
                            .offset = compiler.get_decoration(output.id, spv::DecorationOffset),
                            .name = compiler.get_decoration(output.id, spv::DecorationHlslSemanticGOOGLE),
                            .format = format,
                            .size = GetFormatSize(format),
                        });
                    }
                }

                for (const auto& pushContant : resources.push_constant_buffers)
                {
                    spirv_cross::SPIRType type = compiler.get_type(pushContant.type_id);

                    shaderInfo.pushConstants.EmplaceBack(ShaderPushConstant{
                        .name = String{pushContant.name.c_str()},
                        .offset = compiler.get_decoration(pushContant.id, spv::DecorationOffset),
                        .size = static_cast<u32>(compiler.get_declared_struct_size(type)),
                        .stage = stageInfo.stage
                    });
                }

                ReadResources(compiler, resources.separate_images, descriptors);
                ReadResources(compiler, resources.separate_samplers, descriptors);
                ReadResources(compiler, resources.storage_images, descriptors);
                ReadResources(compiler, resources.uniform_buffers, descriptors);
                ReadResources(compiler, resources.storage_buffers, descriptors);
                ReadResources(compiler, resources.acceleration_structures, descriptors);
            }

        }
        return shaderInfo;
    }

    void ShaderManagerShutdown()
    {
        if (utils)
        {
            utils->Release();
            utils = nullptr;
        }

        if (compiler)
        {
            compiler->Release();
            compiler = nullptr;
        }
    }
}
