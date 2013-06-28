#include "lighthelper.fx"

cbuffer cbPerFrame {
	Light g_Light;
	float3 g_EyePosW;
};

cbuffer cbPerObject {
	float4x4 g_World;
	float4x4 g_WVP; 
	
	float4 difColor;
	bool hasTexture;
};

// Nonnumeric values cannot be added to a cbuffer.
//Texture2D g_DiffuseMap;
Texture2D ObjTexture;
SamplerState ObjSamplerState;
Texture2D g_SpecMap;

SamplerState g_TriLinearSam {
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

struct VS_IN {
	float3 posL		: POSITION;
	float3 normalL	: NORMAL;
	float2 texC		: TEXCOORD;
};

struct VS_OUT {
	float4 posH		: SV_POSITION;
	float3 posW		: POSITION;
	float3 normalW	: NORMAL;
	float2 texC		: TEXCOORD;
};

VS_OUT VS(VS_IN vIn) {
	VS_OUT vOut;
	
	// Transform to homogeneous clip space
	vOut.posH = mul(float4(vIn.posL, 1.0f), g_WVP);

	// Transform to world space
	vOut.posW    = mul(float4(vIn.posL, 1.0f), g_World);
	vOut.normalW = mul(float4(vIn.normalL, 0.0f), g_World);	
	
	// Output vertex attributes for interpolation across triangle
	//vOut.texC  = mul(float4(vIn.texC, 0.0f, 1.0f), g_TexMtx);
	vOut.texC  = vIn.texC;

	return vOut;
}

float4 PS(VS_OUT pIn) : SV_Target {
	// Get materials from texture maps.
	//float4 diffuse = g_DiffuseMap.Sample( g_TriLinearSam, pIn.texC );
	float4 diffuse = ObjTexture.Sample( ObjSamplerState, pIn.texC );
	float4 spec    = g_SpecMap.Sample( g_TriLinearSam, pIn.texC );
	
	// Map [0,1] --> [0,256]
	spec.a *= 256.0f;
	
	// Interpolating normal can make it not be of unit length so normalize it.
    float3 normalW = normalize(pIn.normalW);
    
	// Compute the lit color for this pixel.
    SurfaceInfo v = {pIn.posW, normalW, diffuse, spec};
	float3 litColor = ParallelLight(v, g_Light, g_EyePosW);
    
    return float4(litColor, diffuse.a);
}

technique10 TexTech {
    pass P0 {
        SetVertexShader( CompileShader( vs_4_0, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0, PS() ) );
    }
}