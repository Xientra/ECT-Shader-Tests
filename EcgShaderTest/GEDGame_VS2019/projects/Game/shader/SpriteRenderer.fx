//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Texture2D g_GatlingSprite;
Texture2D g_PlasmaSprite;

matrix g_ViewProjection;
float3 g_CameraUp;
float3 g_CameraRight;

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
    //float4 g_LightDir; // Object space
    //int g_TerrainRes;
};

cbuffer cbChangesEveryFrame
{
    //matrix g_World;

    //float g_Time;
    //matrix g_WorldNormals;

    //float4 g_CameraPos;
};

cbuffer cbUserChanges
{
};

//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct SpriteVertex
{
    float3 position : POSITION; // world-space position (sprite center)
    float radius : RADIUS; // world-space radius (= half side length of the sprite quad)
    int textureIndex : TEXTUREINDEX; // which texture to use (out of SpriteRenderer::m_spriteSRV)
};

struct PSVertex
{
    float4 Position : SV_Position;
    int textureIndex : TEXTUREINDEX;
    float2 textureCoords : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------

SamplerState samAnisotropic
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState samLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

//--------------------------------------------------------------------------------------
// Rasterizer states
//--------------------------------------------------------------------------------------

RasterizerState rsDefault
{
};

RasterizerState rsCullFront
{
    CullMode = Front;
};

RasterizerState rsCullBack
{
    CullMode = Back;
};

RasterizerState rsCullNone
{
    CullMode = None;
};

RasterizerState rsLineAA
{
    CullMode = None;
    AntialiasedLineEnable = true;
};

//--------------------------------------------------------------------------------------
// DepthStates
//--------------------------------------------------------------------------------------
DepthStencilState EnableDepth
{
    DepthEnable = TRUE;
    DepthWriteMask = ALL;
    DepthFunc = LESS_EQUAL;
};

BlendState NoBlending
{
    AlphaToCoverageEnable = FALSE;
    BlendEnable[0] = FALSE;
};

BlendState BSBlendOver
{
    BlendEnable[0] = TRUE;
    SrcBlend[0] = SRC_ALPHA;
    SrcBlendAlpha[0] = ONE;
    DestBlend[0] = INV_SRC_ALPHA;
    DestBlendAlpha[0] = INV_SRC_ALPHA;
};
    
//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------



SpriteVertex SpriteVS(SpriteVertex input)
{
    return input;
}

[maxvertexcount(4)]
void SpriteGS(point SpriteVertex vertex[1], inout TriangleStream<PSVertex> stream)
{
    
    SpriteVertex input = vertex[0];
    
    PSVertex v;
    
    /*
    input.position = float3(0, input.position.y, 0);
    input.radius = 5;
    */
    
    v.textureIndex = input.textureIndex;
    
    v.Position = float4((input.position - input.radius * g_CameraRight + input.radius * g_CameraUp).xyz, 1);
    v.Position = mul(v.Position, g_ViewProjection);
    v.textureCoords = float2(0, 0);
    stream.Append(v);
    
    v.Position = float4((input.position - input.radius * g_CameraRight - input.radius * g_CameraUp).xyz, 1);
    v.Position = mul(v.Position, g_ViewProjection);
    v.textureCoords = float2(0, 1);
    stream.Append(v);
    
    v.Position = float4((input.position + input.radius * g_CameraRight + input.radius * g_CameraUp).xyz, 1);
    v.Position = mul(v.Position, g_ViewProjection);
    v.textureCoords = float2(1, 0);
    stream.Append(v);
    
    v.Position = float4((input.position + input.radius * g_CameraRight - input.radius * g_CameraUp).xyz, 1);
    v.Position = mul(v.Position, g_ViewProjection);
    v.textureCoords = float2(1, 1);
    stream.Append(v);
    
}

float4 SpritePS(PSVertex input) : SV_Target0
{
    //input.textureIndex = 0;

    switch (input.textureIndex)
    {
        case 0:
            return g_GatlingSprite.Sample(samAnisotropic, input.textureCoords);
        case 1:
            return g_PlasmaSprite.Sample(samAnisotropic, input.textureCoords);
        default:
            return float4(1, 0, 1, 1);
    }
}




void DummyVS(SpriteVertex input, out float4 pos : SV_Position)
{
    //pos = float4(0, 0, 0.5, 1);
    pos = mul(float4(input.position, 1), g_ViewProjection);
}

float4 DummyPS(float4 pos : SV_Position) : SV_Target0
{
    return float4(1, 1, 0, 1);
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 Render
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, SpriteVS()));
        SetGeometryShader(CompileShader(gs_4_0, SpriteGS()));
        SetPixelShader(CompileShader(ps_4_0, SpritePS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(BSBlendOver, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }


    pass P1
    {
        SetVertexShader(CompileShader(vs_4_0, DummyVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, DummyPS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}
