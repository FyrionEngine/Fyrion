struct VSOutput
{
	float4 pos  : SV_POSITION;
	float2 uv   : TEXCOORD0;
};

VSOutput MainVS(uint vertexIndex : SV_VertexID)
{
	VSOutput output = (VSOutput)0;
	output.uv       = float2((vertexIndex << 1) & 2, vertexIndex & 2);
	output.pos      = float4(output.uv * 2.0f - 1.0f, 0.0f, 1.0f);
	return output;
}

Texture2D texture;
SamplerState samplerState;

float4 MainPS(VSOutput input) : SV_TARGET
{
	return texture.Sample(samplerState, input.uv);
}