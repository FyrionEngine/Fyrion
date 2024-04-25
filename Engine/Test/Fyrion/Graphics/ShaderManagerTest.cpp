#include <doctest.h>

#include "Fyrion/Engine.hpp"
#include "Fyrion/Core/Array.hpp"
#include "Fyrion/Graphics/ShaderManager.hpp"

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
}
