#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Graphics/ShaderManager.hpp"
#include "Fyrion/IO/Path.hpp"
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
#if 0
    	Engine::Init();
    	{
    		String path = Path::Join(FY_TEST_FILES, "ShaderTest");
            REQUIRE(FileSystem::GetFileStatus(path).exists);
    		ResourceAssets::LoadAssetsFromDirectory("Fyrion", path);

    		{
    			RID rid = Repository::GetByPath("Fyrion://TestRaster.raster");
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

    			CHECK(shaderInfo.inputVariables[0].location == 0);
    			CHECK(shaderInfo.inputVariables[0].format == Format::RGB32F);

    			//TODO - improve tests.
    		}

    		{
    			RID rid = Repository::GetByPath("Fyrion://TestComp.comp");
    			REQUIRE(rid);

    			ResourceObject shader = Repository::Read(rid);
    			REQUIRE(shader);

    			Span<u8> bytes = shader[ShaderAsset::Bytes].As<Span<u8>>();
    			CHECK(!bytes.Empty());

    			Span<ShaderStageInfo> stages = shader[ShaderAsset::Stages].As<Span<ShaderStageInfo>>();
    			ShaderInfo shaderInfo = shader[ShaderAsset::Info].As<ShaderInfo>();

    			REQUIRE(stages.Size() == 1);
    			CHECK(stages[0].stage == ShaderStage::Compute);


    			CHECK(shaderInfo.descriptors.Size() == 2);

    			{
    				DescriptorLayout& descriptorLayout = shaderInfo.descriptors[0];
    				CHECK(descriptorLayout.set == 0);
    				REQUIRE(descriptorLayout.bindings.Size() == 1);

    				DescriptorBinding& biding = descriptorLayout.bindings[0];

					CHECK(biding.binding == 0);
					CHECK(biding.count == 1);
					CHECK(biding.name == "texture");
    				CHECK(biding.descriptorType == DescriptorType::StorageImage);
					CHECK(biding.renderType == RenderType::Image);
    				CHECK(biding.viewType == ViewType::Type2D);
    			}

    			{
    				DescriptorLayout& descriptorLayout = shaderInfo.descriptors[1];
    				CHECK(descriptorLayout.set == 1);
    				REQUIRE(descriptorLayout.bindings.Size() == 2);

    				{
    					DescriptorBinding& biding = descriptorLayout.bindings[0];
    					CHECK(biding.binding == 0);
    					CHECK(biding.count == 5);
    					CHECK(biding.name == "fixedTextures");
    					CHECK(biding.descriptorType == DescriptorType::StorageImage);
    					CHECK(biding.renderType == RenderType::Array);
    					CHECK(biding.viewType == ViewType::Type2D);
    				}
    				{
    					DescriptorBinding& biding = descriptorLayout.bindings[1];
    					CHECK(biding.binding == 1);
    					CHECK(biding.count == 0);
    					CHECK(biding.name == "runtimeTextures");
    					CHECK(biding.descriptorType == DescriptorType::StorageImage);
    					CHECK(biding.renderType == RenderType::RuntimeArray);
    					CHECK(biding.viewType == ViewType::Type2D);
    				}
    			}
    		}
    	}
    	Engine::Destroy();
#endif
    }
}
