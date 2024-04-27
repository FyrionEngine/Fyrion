#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Assets/AssetTypes.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Graphics/ShaderManager.hpp"
#include "Fyrion/IO/Path.hpp"
#include "Fyrion/Resource/Repository.hpp"
#include "Fyrion/Resource/ResourceAssets.hpp"
#include "Fyrion/IO/FileSystem.hpp"

#include <iostream>

using namespace Fyrion;

namespace
{
    auto simpleShader = R""""(

	    struct VSInput
	    {
	         float3 position  : POSITION0;
	         float3 color     : COLOR0;
	    };

	    struct VSOutput
	    {
			float4 pos          : SV_POSITION;
	        float3 color        : COLOR0;
	    };

	    struct CameraBuffer
	    {
		    float4x4 projectionMatrix;
		    float4x4 viewMatrix;
	    };

	    [[vk::push_constant]] CameraBuffer camera;

	    VSOutput MainVS(VSInput input)
	    {
		    VSOutput output   = (VSOutput)0;
		    output.pos        = mul(camera.projectionMatrix, mul(camera.viewMatrix, float4(input.position, 1.0)));
		    output.color      = input.color;
		    return output;
	    }

	    float4 MainPS(VSOutput input) : SV_TARGET
	    {
		    return float4(input.color, 1.0);
	    }

		)"""";


    TEST_CASE("Graphics::ShaderManager::CompileShader")
    {
        Engine::Init();
        {
        	//VS
        	{
		        ShaderCreation shaderCreation{
			        .source = simpleShader,
			        .entryPoint = "MainVS",
			        .shaderStage = ShaderStage::Vertex,
			        .renderApi = RenderApiType::Vulkan
		        };

        		Array<u8> bytes{};
        		bool res = ShaderManager::CompileShader(shaderCreation, bytes);
        		CHECK(res);
        		CHECK(bytes.Size() > 0);
        	}

        	//PS
        	{
        		ShaderCreation shaderCreation{
        			.source = simpleShader,
					.entryPoint = "MainPS",
					.shaderStage = ShaderStage::Pixel,
					.renderApi = RenderApiType::Vulkan
				};

        		Array<u8> bytes{};
        		bool res = ShaderManager::CompileShader(shaderCreation, bytes);
        		CHECK(res);
        		CHECK(bytes.Size() > 0);
        	}
        }
        Engine::Destroy();
    }

	TEST_CASE("Graphics::ShaderAsset")
    {
    	Engine::Init();
    	{
    		String path = Path::Join(FY_TEST_FILES, "ShaderTest");
            REQUIRE(FileSystem::GetFileStatus(path).exists);
    		ResourceAssets::LoadAssetsFromDirectory("Fyrion", path);

			RID rid = Repository::GetByPath("Fyrion://Test.raster");
    		CHECK(rid);

    		ResourceObject shader = Repository::Read(rid);
    		CHECK(shader);

    		Span<u8> bytes = shader[ShaderAsset::Bytes].As<Span<u8>>();
    		CHECK(!bytes.Empty());

    		Span<ShaderStageInfo> stages = shader[ShaderAsset::Stages].As<Span<ShaderStageInfo>>();
    		ShaderInfo shaderInfo = shader[ShaderAsset::Info].As<ShaderInfo>();

			CHECK(stages.Size() == 2);

    		CHECK(stages[0].stage == ShaderStage::Vertex);
    		CHECK(stages[1].stage == ShaderStage::Pixel);

    		CHECK(shaderInfo.inputVariables.Size() == 3);
    		CHECK(shaderInfo.outputVariables.Size() == 1);
    		CHECK(shaderInfo.pushConstants.Size() == 1);
    		CHECK(shaderInfo.descriptors.Size() == 2);


    	}
    	Engine::Destroy();

    }
}
