#include "ShaderManager.hpp"
#include "Fyrion/Platform/Platform.hpp"

#if defined(FY_WIN)
#include "Windows.h"
#else
#include "dxc/WinAdapter.h"
#endif

#include "Fyrion/Core/Logger.hpp"
#include "dxc/dxcapi.h"

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


    void ShaderCompilerInit()
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

    void ShaderCompilerShutdown()
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
