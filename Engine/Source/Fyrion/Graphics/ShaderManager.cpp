#include "ShaderManager.hpp"
#include "Fyrion/Platform/Platform.hpp"

#if defined(FY_WIN)
#include "Windows.h"
#else
#include "dxc/WinAdapter.h"
#endif

#include <algorithm>
#include <iostream>

#include "spirv_reflect.h"
#include "Assets/ShaderAsset.hpp"
#include "Fyrion/Core/Logger.hpp"
#include "dxc/dxcapi.h"
#include "Fyrion/Asset/AssetManager.hpp"
#include "Fyrion/Asset/AssetTypes.hpp"
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
        Logger& logger = Logger::GetLogger("Fyrion::ShaderManager", LogLevel::Debug);

        IDxcUtils*            utils{};
        IDxcCompiler3*        compiler{};
        DxcCreateInstanceProc dxcCreateInstance;
    }

    struct IncludeHandler : public IDxcIncludeHandler
    {
        ShaderAsset*  shader{};
        IDxcBlobEncoding* blobEncoding{};

        IncludeHandler(ShaderAsset*  asset) : shader(asset) {}

        static String FormatFilePath(LPCWSTR pFilename)
        {
            std::wstring widePath(pFilename);
            std::string  fileName{};
            std::transform(widePath.begin(), widePath.end(), std::back_inserter(fileName), [](wchar_t c)
            {
                return (char)c;
            });

            if (const auto c = fileName.find(".\\"); c != std::string::npos)
            {
                fileName.replace(c, sizeof(".\\") - 1, "");
            }

            auto c = fileName.find('\\');
            while (c != std::string::npos)
            {
                fileName.replace(c, sizeof("\\") - 1, "/");
                c = fileName.find('\\');
            }

            if (c = fileName.find(":/"); c != std::string::npos)
            {
                fileName.replace(c, sizeof(":/") - 1, "://");
            }

            return {fileName.c_str(), fileName.size()};
        }


        HRESULT STDMETHODCALLTYPE LoadSource(LPCWSTR pFilename, IDxcBlob** ppIncludeSource) override
        {
            String includePath = FormatFilePath(pFilename);
            //check if that's a path
            if (StringView(includePath).FindFirstOf("://") == nPos)
            {
                if (shader && shader->GetParent() != nullptr)
                {
                    includePath = String(shader->GetParent()->GetPath()).Append("/").Append(includePath);
                }
            }

            ShaderAsset* shaderInclude = AssetManager::FindByPath<ShaderAsset>(includePath);
            if (!shaderInclude)
            {
                return S_FALSE;
            }

            String source = shaderInclude->GetShaderSource();

            utils->CreateBlob(source.CStr(), source.Size(), CP_UTF8, &blobEncoding);
            *ppIncludeSource = blobEncoding;

            shaderInclude->AddShaderDependency(shader);

            return S_OK;
        }

        ULONG AddRef() override
        {
            return 0;
        }

        ULONG Release() override
        {
            if (blobEncoding)
            {
                blobEncoding->Release();
            }
            return 0;
        }

        HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** ppvObject) override
        {
            return 0;
        }
    };

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
            args.EmplaceBack(L"-fvk-use-dx-layout");
            args.EmplaceBack(L"-fvk-use-dx-position-w");
        }

        IDxcResult*    pResults{};

        IncludeHandler includeHandler{shaderCreation.asset};

        compiler->Compile(&source, args.Data(), args.Size(), &includeHandler, IID_PPV_ARGS(&pResults));

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
        includeHandler.Release();

        return true;
    }

    namespace SpirvUtils
    {
        Format CastFormat(const SpvReflectFormat& format)
        {
            switch (format)
            {
                case SPV_REFLECT_FORMAT_UNDEFINED: break;
                case SPV_REFLECT_FORMAT_R16_UINT: break;
                case SPV_REFLECT_FORMAT_R16_SINT: break;
                case SPV_REFLECT_FORMAT_R16_SFLOAT: return Format::R16F;
                case SPV_REFLECT_FORMAT_R16G16_UINT: break;
                case SPV_REFLECT_FORMAT_R16G16_SINT: break;
                case SPV_REFLECT_FORMAT_R16G16_SFLOAT: return Format::RG16F;
                case SPV_REFLECT_FORMAT_R16G16B16_UINT: break;
                case SPV_REFLECT_FORMAT_R16G16B16_SINT: break;
                case SPV_REFLECT_FORMAT_R16G16B16_SFLOAT: return Format::RGB16F;
                case SPV_REFLECT_FORMAT_R16G16B16A16_UINT: break;
                case SPV_REFLECT_FORMAT_R16G16B16A16_SINT: break;
                case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT: return Format::RGBA16F;
                case SPV_REFLECT_FORMAT_R32_UINT: break;
                case SPV_REFLECT_FORMAT_R32_SINT: break;
                case SPV_REFLECT_FORMAT_R32_SFLOAT: return Format::R32F;
                case SPV_REFLECT_FORMAT_R32G32_UINT: break;
                case SPV_REFLECT_FORMAT_R32G32_SINT: break;
                case SPV_REFLECT_FORMAT_R32G32_SFLOAT: return Format::RG32F;
                case SPV_REFLECT_FORMAT_R32G32B32_UINT: break;
                case SPV_REFLECT_FORMAT_R32G32B32_SINT: break;
                case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT: return Format::RGB32F;
                case SPV_REFLECT_FORMAT_R32G32B32A32_UINT: break;
                case SPV_REFLECT_FORMAT_R32G32B32A32_SINT: break;
                case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return Format::RGBA32F;
                case SPV_REFLECT_FORMAT_R64_UINT: break;
                case SPV_REFLECT_FORMAT_R64_SINT: break;
                case SPV_REFLECT_FORMAT_R64_SFLOAT: break;
                case SPV_REFLECT_FORMAT_R64G64_UINT: break;
                case SPV_REFLECT_FORMAT_R64G64_SINT: break;
                case SPV_REFLECT_FORMAT_R64G64_SFLOAT: break;
                case SPV_REFLECT_FORMAT_R64G64B64_UINT: break;
                case SPV_REFLECT_FORMAT_R64G64B64_SINT: break;
                case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT: break;
                case SPV_REFLECT_FORMAT_R64G64B64A64_UINT: break;
                case SPV_REFLECT_FORMAT_R64G64B64A64_SINT: break;
                case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT: break;
            }
            logger.FatalError("CastFormat format not found");
            return {};
        }

        RenderType CastRenderType(const SpvOp& spvOp)
        {
            switch (spvOp)
            {
                case SpvOpTypeVoid: return RenderType::Void;
                case SpvOpTypeBool: return RenderType::Bool;
                case SpvOpTypeInt: return RenderType::Int;
                case SpvOpTypeFloat: return RenderType::Float;
                case SpvOpTypeVector: return RenderType::Vector;
                case SpvOpTypeMatrix: return RenderType::Matrix;
                case SpvOpTypeImage: return RenderType::Image;
                case SpvOpTypeSampler: return RenderType::Sampler;
                case SpvOpTypeSampledImage: return RenderType::SampledImage;
                case SpvOpTypeArray: return RenderType::Array;
                case SpvOpTypeRuntimeArray: return RenderType::RuntimeArray;
                case SpvOpTypeStruct: return RenderType::Struct;
                default: return RenderType::None;
            }
        }

        u32 GetAttributeSize(SpvReflectFormat reflectFormat)
        {
            switch (reflectFormat)
            {
                case SPV_REFLECT_FORMAT_UNDEFINED: return 0;
                case SPV_REFLECT_FORMAT_R32_UINT: return sizeof(u32);
                case SPV_REFLECT_FORMAT_R32_SINT: return sizeof(i32);
                case SPV_REFLECT_FORMAT_R32_SFLOAT: return sizeof(f32);
                case SPV_REFLECT_FORMAT_R32G32_UINT: return sizeof(u32) * 2;
                case SPV_REFLECT_FORMAT_R32G32_SINT: return sizeof(i32) * 2;
                case SPV_REFLECT_FORMAT_R32G32_SFLOAT: return sizeof(f32) * 2;
                case SPV_REFLECT_FORMAT_R32G32B32_UINT: return sizeof(u32) * 3;
                case SPV_REFLECT_FORMAT_R32G32B32_SINT: return sizeof(i32) * 3;
                case SPV_REFLECT_FORMAT_R32G32B32_SFLOAT: return sizeof(f32) * 3;
                case SPV_REFLECT_FORMAT_R32G32B32A32_UINT: return sizeof(u32) * 4;
                case SPV_REFLECT_FORMAT_R32G32B32A32_SINT: return sizeof(i32) * 4;
                case SPV_REFLECT_FORMAT_R32G32B32A32_SFLOAT: return sizeof(f32) * 4;
                case SPV_REFLECT_FORMAT_R16_UINT: break;
                case SPV_REFLECT_FORMAT_R16_SINT: break;
                case SPV_REFLECT_FORMAT_R16_SFLOAT: break;
                case SPV_REFLECT_FORMAT_R16G16_UINT: break;
                case SPV_REFLECT_FORMAT_R16G16_SINT: break;
                case SPV_REFLECT_FORMAT_R16G16_SFLOAT: break;
                case SPV_REFLECT_FORMAT_R16G16B16_UINT: break;
                case SPV_REFLECT_FORMAT_R16G16B16_SINT: break;
                case SPV_REFLECT_FORMAT_R16G16B16_SFLOAT: break;
                case SPV_REFLECT_FORMAT_R16G16B16A16_UINT: break;
                case SPV_REFLECT_FORMAT_R16G16B16A16_SINT: break;
                case SPV_REFLECT_FORMAT_R16G16B16A16_SFLOAT: break;
                case SPV_REFLECT_FORMAT_R64_UINT: break;
                case SPV_REFLECT_FORMAT_R64_SINT: break;
                case SPV_REFLECT_FORMAT_R64_SFLOAT: break;
                case SPV_REFLECT_FORMAT_R64G64_UINT: break;
                case SPV_REFLECT_FORMAT_R64G64_SINT: break;
                case SPV_REFLECT_FORMAT_R64G64_SFLOAT: break;
                case SPV_REFLECT_FORMAT_R64G64B64_UINT: break;
                case SPV_REFLECT_FORMAT_R64G64B64_SINT: break;
                case SPV_REFLECT_FORMAT_R64G64B64_SFLOAT: break;
                case SPV_REFLECT_FORMAT_R64G64B64A64_UINT: break;
                case SPV_REFLECT_FORMAT_R64G64B64A64_SINT: break;
                case SPV_REFLECT_FORMAT_R64G64B64A64_SFLOAT: break;
            }
            logger.FatalError("GetAttributeSize format not found");
            return 0;
        }

        DescriptorType GetDescriptorType(SpvReflectDescriptorType descriptorType)
        {
            switch (descriptorType)
            {
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER: return DescriptorType::Sampler;
                case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: return DescriptorType::SampledImage;
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: return DescriptorType::StorageImage;
                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: return DescriptorType::UniformBuffer;
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER: return DescriptorType::StorageBuffer;
                case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: return DescriptorType::AccelerationStructure;
                case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: break;
                case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: break;
            }
            logger.FatalError("GetDescriptorType DescriptorType not found");
            return {};
        }

        ViewType DimToViewType(SpvDim dim, u32 arrayed)
        {
            if (arrayed)
            {
                switch (dim)
                {
                    case SpvDim1D: return ViewType::Type1DArray;
                    case SpvDim2D: return ViewType::Type2DArray;
                    case SpvDim3D: return ViewType::Type3D;
                    case SpvDimCube: return ViewType::TypeCubeArray;
                    case SpvDimRect: break;
                    case SpvDimBuffer: break;
                    case SpvDimSubpassData: break;
                    case SpvDimMax: break;
                    case SpvDimTileImageDataEXT: break;
                }
            }
            else
            {
                switch (dim)
                {
                    case SpvDim1D: return ViewType::Type1D;
                    case SpvDim2D: return ViewType::Type2D;
                    case SpvDim3D: return ViewType::Type3D;
                    case SpvDimCube: return ViewType::TypeCube;
                    case SpvDimRect: break;
                    case SpvDimBuffer: break;
                    case SpvDimSubpassData: break;
                    case SpvDimMax: break;
                    case SpvDimTileImageDataEXT: break;
                }
            }

            logger.FatalError("DimToViewType SpvDim not found");

            return ViewType::Undefined;
        }

        u32 CalcTypeSize(SpvReflectTypeDescription* reflectTypeDescription)
        {
            switch (reflectTypeDescription->op)
            {
                case SpvOpTypeInt: return sizeof(i32);
                case SpvOpTypeFloat: return sizeof(f32);
                case SpvOpTypeVector: return reflectTypeDescription->traits.numeric.vector.component_count * sizeof(f32);
                case SpvOpTypeMatrix: return reflectTypeDescription->traits.numeric.matrix.row_count * reflectTypeDescription->traits.numeric.matrix.stride;
                case SpvOpTypeArray:
                {
                    u32 size{};
                    for (int d = 0; d < reflectTypeDescription->traits.array.dims_count; ++d)
                    {
                        size += reflectTypeDescription->traits.array.dims[d] * reflectTypeDescription->traits.numeric.scalar.width;
                    }
                    return size;
                }
                case SpvOpTypeRuntimeArray: return 0; //runtime arrays has size?
                case SpvOpTypeStruct: return 0;       //ignore, size will be calculated using the fields
            }

            logger.FatalError("CalcTypeSize type not found");

            return 0;
        }

        void GetTypeDescription(DescriptorBinding& descriptorBinding, Array<TypeDescription>& parentMembers, SpvReflectTypeDescription* reflectTypeDescription)
        {
            u32 typeSize = CalcTypeSize(reflectTypeDescription);

            TypeDescription typeDescription = TypeDescription{
                .name = reflectTypeDescription->struct_member_name != nullptr ? reflectTypeDescription->struct_member_name : "",
                .type = CastRenderType(reflectTypeDescription->op),
                .size = typeSize,
                .offset = descriptorBinding.size
            };

            descriptorBinding.size += typeSize;

            for (int m = 0; m < reflectTypeDescription->member_count; ++m)
            {
                GetTypeDescription(descriptorBinding, typeDescription.members, &reflectTypeDescription->members[m]);
            }

            parentMembers.EmplaceBack(typeDescription);
        }
    }


    void SortAndAddDescriptors(ShaderInfo& shaderInfo, const HashMap<u32, HashMap<u32, DescriptorBinding>>& descriptors)
    {
        //sorting descriptors
        Array<u32> sortDescriptors{};
        sortDescriptors.Reserve(descriptors.Size());
        for (auto& descriptorIt : descriptors)
        {
            sortDescriptors.EmplaceBack(descriptorIt.first);
        }
        std::sort(sortDescriptors.begin(), sortDescriptors.end());

        for (auto& set: sortDescriptors)
        {
            const HashMap<u32, DescriptorBinding>& bindings = descriptors.Find(set)->second;
            DescriptorLayout descriptorLayout = DescriptorLayout{set};

            Array<u32> sortBindings{};
            sortBindings.Reserve(bindings.Size());
            for (auto& bindingIt : bindings)
            {
                sortBindings.EmplaceBack(bindingIt.first);
            }
            std::ranges::sort(sortBindings);

            for (auto& binding : sortBindings)
            {
                descriptorLayout.bindings.EmplaceBack(bindings.Find(binding)->second);
            }
            shaderInfo.descriptors.EmplaceBack(descriptorLayout);
        }
    }

    ShaderInfo ShaderManager::ExtractShaderInfo(const Span<u8>& bytes, const Span<ShaderStageInfo>& stages, RenderApiType renderApi)
    {
        ShaderInfo shaderInfo;

        if (renderApi != RenderApiType::D3D12)
        {
            u32                                           varCount = 0;
            HashMap<u32, HashMap<u32, DescriptorBinding>> descriptors{};

            for (const ShaderStageInfo& stageInfo : stages)
            {
                Array<u32> data;
                data.Resize(stageInfo.size);
                MemCopy(data.Data(), bytes.Data() + stageInfo.offset, stageInfo.size);

                SpvReflectShaderModule module{};
                spvReflectCreateShaderModule(data.Size(), data.Data(), &module);

                if (stageInfo.stage == ShaderStage::Vertex)
                {
                    spvReflectEnumerateInputVariables(&module, &varCount, nullptr);
                    Array<SpvReflectInterfaceVariable*> inputVariables;
                    inputVariables.Resize(varCount);
                    spvReflectEnumerateInputVariables(&module, &varCount, inputVariables.Data());

                    u32 offset = 0;
                    for (SpvReflectInterfaceVariable* variable : inputVariables)
                    {
                        if (variable->location < U32_MAX)
                        {
                            InterfaceVariable interfaceVariable{
                                .location = variable->location,
                                .offset = offset,
                                .name = variable->name,
                                .format = SpirvUtils::CastFormat(variable->format),
                                .size = SpirvUtils::GetAttributeSize(variable->format),
                            };

                            offset += interfaceVariable.size;
                            shaderInfo.inputVariables.EmplaceBack(interfaceVariable);
                        }
                    }
                    shaderInfo.stride = offset;
                }
                else if (stageInfo.stage == ShaderStage::Pixel)
                {
                    Array<SpvReflectInterfaceVariable*> outputVariables;
                    spvReflectEnumerateOutputVariables(&module, &varCount, nullptr);
                    outputVariables.Resize(varCount);
                    spvReflectEnumerateOutputVariables(&module, &varCount, outputVariables.Data());

                    for (SpvReflectInterfaceVariable* variable : outputVariables)
                    {
                        InterfaceVariable interfaceVariable{
                            .location = variable->location,
                            .format = SpirvUtils::CastFormat(variable->format),
                            .size = SpirvUtils::GetAttributeSize(variable->format),
                        };
                        shaderInfo.outputVariables.EmplaceBack(interfaceVariable);
                    }
                } else
                {
                    int a = 0;
                }

                spvReflectEnumeratePushConstantBlocks(&module, &varCount, nullptr);
                Array<SpvReflectBlockVariable*> blockVariables;
                blockVariables.Resize(varCount);
                spvReflectEnumeratePushConstantBlocks(&module, &varCount, blockVariables.Data());

                for (auto block : blockVariables)
                {
                    shaderInfo.pushConstants.EmplaceBack(ShaderPushConstant{
                        .name = block->name,
                        .offset = block->offset,
                        .size = block->size,
                        .stage = stageInfo.stage
                    });
                }

                u32 varCount{0};
                spvReflectEnumerateDescriptorSets(&module, &varCount, nullptr);
                Array<SpvReflectDescriptorSet*> descriptorSets{};
                descriptorSets.Resize(varCount);
                spvReflectEnumerateDescriptorSets(&module, &varCount, descriptorSets.Data());

                spvReflectEnumerateDescriptorBindings(&module, &varCount, nullptr);
                Array<SpvReflectDescriptorBinding*> descriptorBinds{};
                descriptorBinds.Resize(varCount);
                spvReflectEnumerateDescriptorBindings(&module, &varCount, descriptorBinds.Data());

                for (const auto descriptorSet : descriptorSets)
                {
                    auto setIt = descriptors.Find(descriptorSet->set);
                    if (setIt == descriptors.end())
                    {
                        setIt = descriptors.Insert({descriptorSet->set, HashMap<u32, DescriptorBinding>{}}).first;
                    }
                    HashMap<u32, DescriptorBinding>& bindings = setIt->second;

                    for (const auto descriptorBind : descriptorBinds)
                    {
                        if (descriptorBind->set == descriptorSet->set)
                        {
                            if (bindings.Find(descriptorBind->binding) == bindings.end())
                            {
                                DescriptorBinding descriptorBinding = DescriptorBinding{
                                    .binding = descriptorBind->binding,
                                    .count = descriptorBind->count,
                                    .name = descriptorBind->name,
                                    .descriptorType = SpirvUtils::GetDescriptorType(descriptorBind->descriptor_type),
                                    .renderType = SpirvUtils::CastRenderType(descriptorBind->type_description->op),
                                    .viewType = SpirvUtils::DimToViewType(descriptorBind->image.dim, descriptorBind->image.arrayed),
                                };

                                for (int m = 0; m < descriptorBind->type_description->member_count; ++m)
                                {
                                    SpirvUtils::GetTypeDescription(descriptorBinding, descriptorBinding.members, &descriptorBind->type_description->members[m]);
                                }

                                bindings.Emplace(descriptorBind->binding, Traits::Move(descriptorBinding));
                            }
                        }
                    }
                }

                spvReflectDestroyShaderModule(&module);
            }

            SortAndAddDescriptors(shaderInfo, descriptors);
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
