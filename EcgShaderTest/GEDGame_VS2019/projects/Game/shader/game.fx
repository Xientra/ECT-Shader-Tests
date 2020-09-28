//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Texture2D g_Diffuse; // Material albedo for diffuse lighting
Texture2D g_NormalMap; // float buffer for the height map of the terrain 
Buffer<float> g_HeightMap;

Texture2D g_CockpitSpecular;
Texture2D g_CockpitGlow;

float3 g_CameraForward;
float g_startupTime;
Texture2D sv_0 : SV_Target0;
//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
    float4 g_LightDir; // Object space
    int g_TerrainRes;
};

cbuffer cbChangesEveryFrame
{
    matrix g_World;
    matrix g_WorldViewProjection;
    float g_Time;
    matrix g_WorldNormals;

    float4 g_CameraPos;
};

cbuffer cbUserChanges
{
};


//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct PosNorTex
{
    float4 Pos : SV_POSITION;
    float4 Nor : NORMAL;
    float2 Tex : TEXCOORD;
};

struct PosTexLi
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
    float Li : LIGHT_INTENSITY;
    float3 normal : NORMAL;
};

struct PosTex
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
};

struct T3dVertexVSIn
{
    float3 Pos : POSITION; //Position in object space     
    float2 Tex : TEXCOORD; //Texture coordinate     
    float3 Nor : NORMAL; //Normal in object space     
    float3 Tan : TANGENT; //Tangent in object space (not used in Ass. 5) 
};

struct T3dVertexPSIn
{
    float4 Pos : SV_POSITION; //Position in clip space     
    float2 Tex : TEXCOORD; //Texture coordinate     
    float3 PosWorld : POSITION; //Position in world space     
    float3 NorWorld : NORMAL; //Normal in world space     
    float3 TanWorld : TANGENT; //Tangent in world space (not used in Ass. 5) 
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


//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------

PosTexLi SimpleVS(PosNorTex Input)
{
    PosTexLi output = (PosTexLi) 0;

    // Transform position from object space to homogenious clip space
    output.Pos = mul(Input.Pos, g_WorldViewProjection);

    // Pass trough normal and texture coordinates
    output.Tex = Input.Tex;

    // Calculate light intensity
    output.normal = normalize(mul(Input.Nor, g_World).xyz); // Assume orthogonal world matrix
    output.Li = saturate(dot(output.normal, g_LightDir.xyz));
        
    return output;
}
float4 SimplePS(PosTexLi Input) : SV_Target0
{
    // Perform lighting in object space, so that we can use the input normal "as it is"
    //float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
    float4 matDiffuse = g_Diffuse.Sample(samLinearClamp, Input.Tex);
    return float4(matDiffuse.rgb * Input.Li, 1);
	//return float4(Input.normal, 1);
}



PosTex TerrainVS(uint VertexID : SV_VertexID)
{

    PosTex output = (PosTex) 0;


    
    float z = (int) (VertexID / g_TerrainRes);
    float x = VertexID - (z * g_TerrainRes);

    x /= g_TerrainRes;
    z /= g_TerrainRes;

    float y = g_HeightMap[VertexID]; // + sin((x * 50) + g_startupTime) * sin((z * 50) + g_startupTime) * 0.05;
    
    output.Tex = float2(x, z);

    x -= 0.5f;
    z -= 0.5f;

    
    output.Pos = float4(x, y, z, 1);
    output.Pos = mul(output.Pos, g_WorldViewProjection);

    return output;
}
float4 TerrainPS(PosTex Input) : SV_Target0
{
    float3 n;
    
    float4 nImg = g_NormalMap.Sample(samAnisotropic, Input.Tex);
    
    n.xz = ((nImg * 2) - 1).rg;
    n.y = sqrt(1 - (n.x * n.x + n.z * n.z)); // maybe use sqrt lol   1 = x*x + y*y + z*z => 1 - x*x - z*z = y*y

    n = normalize(mul(n, g_World).xyz);


    float3 matDiffuse = g_Diffuse.Sample(samLinearClamp, Input.Tex).rgb;

    float i = saturate(dot(n, g_LightDir.xyz));
    
    float ambientLight = 0.03;
    
    if (i < ambientLight)
        i = ambientLight;

    return float4(matDiffuse * i, 1);
}



T3dVertexPSIn MeshVS(T3dVertexVSIn Input)
{

    T3dVertexPSIn output = (T3dVertexPSIn) 0;

    // Transform the vertex position into clip space (output.Pos), 
    output.Pos = mul(float4(Input.Pos, 1), g_WorldViewProjection);

    // copy the texture coordinate
    output.Tex = Input.Tex;

    // transform the vertex position & normal & tangent into world space 

    float4 posWorldClip = mul(float4(Input.Pos, 1), g_World);

    output.PosWorld = posWorldClip.xyz / posWorldClip.w;
    output.NorWorld = normalize(mul(float4(Input.Nor, 0), g_WorldNormals).xyz);
    output.TanWorld = normalize(mul(float4(Input.Tan, 0), g_World).xyz);


    // The vertex tangent is reserved for future assignments and should 
    // be treated as a directional vector (i.e. like the normal). 


    return output;
}
float4 MeshPS(T3dVertexPSIn Input) : SV_Target0
{

    //return g_Diffuse.Sample(samAnisotropic, Input.Tex);

    float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
    float4 matSpecular = g_CockpitSpecular.Sample(samAnisotropic, Input.Tex);
    float4 matGlow = g_CockpitGlow.Sample(samAnisotropic, Input.Tex);

    float4 colLight = float4(1, 1, 1, 1);
    float4 colLightAmbient = float4(1, 1, 1, 1);

    float3 n = normalize(Input.NorWorld);
    float3 l = g_LightDir.xyz;
    //float3 lWorld = (g_LightDir.xyz * g_LightDir.w).xyz;
    float3 r = reflect(-(l.xyz), n);
    float3 v = normalize(g_CameraPos.xyz - Input.PosWorld);

    float cd = 0.5f;
    float cs = 0.4f;
    float ca = 0.1f;
    float cg = 0.5f;
    
    int s = 32;

    return cd * matDiffuse * saturate(dot(n, l)) * colLight +
        cs * matSpecular * pow(saturate(dot(r, v)), s) * colLight +
        ca * matDiffuse * colLightAmbient +
        cg * matGlow;
}



T3dVertexPSIn DistanceVS(T3dVertexVSIn Input)
{

    T3dVertexPSIn output = (T3dVertexPSIn) 0;

    //Input.Pos += Input.Nor * 3; //Input.Nor * (sin(g_startupTime) + 0.5) * 1;
    
    
    // Transform the vertex position into clip space (output.Pos), 
    output.Pos = mul(float4(Input.Pos, 1), g_WorldViewProjection);

    // copy the texture coordinate
    output.Tex = Input.Tex;

    // transform the vertex position & normal & tangent into world space 

    float4 posWorldClip = mul(float4(Input.Pos, 1), g_World);

    output.PosWorld = posWorldClip.xyz / posWorldClip.w;
    output.NorWorld = normalize(mul(float4(Input.Nor, 0), g_WorldNormals).xyz);
    
    return output;
}
float4 DistancePS(T3dVertexPSIn Input) : SV_Target0
{

    float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
    float4 matSpecular = g_CockpitSpecular.Sample(samAnisotropic, Input.Tex);
    float4 matGlow = g_CockpitGlow.Sample(samAnisotropic, Input.Tex);

    float4 colLight = float4(1, 1, 1, 1);
    float4 colLightAmbient = float4(1, 1, 1, 1);

    float3 n = normalize(Input.NorWorld);
    float3 l = g_LightDir.xyz;
    float3 lWorld = (g_LightDir.xyz * g_LightDir.w).xyz;
    float3 r = reflect(-(l.xyz), n);
    float3 v = normalize(g_CameraPos.xyz - Input.PosWorld);
    
    float cd = 0.5f;
    float cs = 0.4f;
    float ca = 0.1f;
    float cg = 0.5f;
    
    int s = 6;
    
    /*
    return cd * matDiffuse * saturate(dot(n, l)) * colLight +
        cs * matSpecular * pow(saturate(dot(r, v)), s) * colLight +
        ca * matDiffuse * colLightAmbient +
        cg * matGlow;
    */
    
    float col = length(g_CameraPos.xyz - Input.PosWorld) / 5000;
    return float4(col, col, col, 1);

}



struct toonPSIn
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
    float3 PosWorld : POSITION; //Position in world space, i guess 
    float3 NorWorld : NORMAL;
};
toonPSIn ToonVS(T3dVertexVSIn Input)
{
    toonPSIn output = (toonPSIn) 0;
    
    output.Pos = mul(float4(Input.Pos, 1), g_WorldViewProjection);
    output.Tex = Input.Tex;
    

    output.NorWorld = normalize(mul(float4(Input.Nor, 0), g_WorldNormals).xyz);
    
    // -----------------------------------------
    float4 posWorldClip = mul(float4(Input.Pos, 1), g_World);
    output.PosWorld = posWorldClip.xyz / posWorldClip.w;
    // -----------------------------------------
    
    return output;
}
float4 ToonPS(toonPSIn Input) : SV_TARGET0
{
    float4 pixel = sv_0.Sample(samAnisotropic, Input.Tex);
    
    
    float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
    float3 l = g_LightDir.xyz;
    float3 v = normalize(g_CameraPos.xyz - Input.PosWorld); // vector to camera
    float3 r = reflect(-(g_LightDir.xyz), normalize(Input.NorWorld));
    float3 n = Input.NorWorld;
    
    
    float4 ambient = float4(0.25, 0.25, 0.25, 1);
    
    float lightIntensity = dot(n, l) > 0 ? 0.69 : 0;
    
    float specularIntensity = pow(saturate(dot(v, r)), 32 * 32);
    float4 specularColor = (specularIntensity > 0 ? 1 : 0) * float4(1, 1, 1, 1);
    
    
    float rimDot = 1 - dot(v, n);
    rimDot = rimDot > 0.75 ? 1 : 0;
    
    return (lightIntensity + ambient + 10 * specularColor - rimDot * 1000) * matDiffuse;
}



T3dVertexPSIn FlatShadingVS(T3dVertexVSIn Input)
{

    T3dVertexPSIn output = (T3dVertexPSIn) 0;

    // Transform the vertex position into clip space (output.Pos), 
    output.Pos = mul(float4(Input.Pos, 1), g_WorldViewProjection);

    // copy the texture coordinate
    output.Tex = Input.Tex;

    // transform the vertex position & normal & tangent into world space 

    float4 posWorldClip = mul(float4(Input.Pos, 1), g_World);

    output.PosWorld = posWorldClip.xyz / posWorldClip.w;
    output.NorWorld = normalize(mul(float4(Input.Nor, 0), g_WorldNormals).xyz);
    output.TanWorld = normalize(mul(float4(Input.Tan, 0), g_World).xyz);


    // The vertex tangent is reserved for future assignments and should 
    // be treated as a directional vector (i.e. like the normal). 


    return output;
}
float4 FlatShadingPS(T3dVertexPSIn Input) : SV_Target0
{

    //return g_Diffuse.Sample(samAnisotropic, Input.Tex);

    float4 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex);
    float4 matSpecular = g_CockpitSpecular.Sample(samAnisotropic, Input.Tex);
    float4 matGlow = g_CockpitGlow.Sample(samAnisotropic, Input.Tex);

    float4 colLight = float4(1, 1, 1, 1);
    float4 colLightAmbient = float4(1, 1, 1, 1);

    float3 n = normalize(Input.NorWorld);
    float3 l = g_LightDir.xyz;
    //float3 lWorld = (g_LightDir.xyz * g_LightDir.w).xyz;
    float3 r = reflect(-(l.xyz), n);
    float3 v = normalize(g_CameraPos.xyz - Input.PosWorld);

    float cd = 0.5f;
    float cs = 0.4f;
    float ca = 0.1f;
    float cg = 0.5f;
    
    int s = 32;

    return cd * matDiffuse * saturate(dot(n, l)) * colLight +
        cs * matSpecular * pow(saturate(dot(r, v)), s) * colLight +
        ca * matDiffuse * colLightAmbient +
        cg * matGlow;
}



cbuffer cbPerMeshVS
{
    matrix worldNormalsMatrix;
    
    matrix worldViewMatrix;
    matrix projectionMatrix;

    float3 lightPosition; //position of a pointlight in view space
    
    float3 ambientLight;
    float3 lightColor; //color of the pointlight
    float4 shaderParams; // these are just some hardcodede values
};
struct PS_INPUT
{
    float4 pos : SV_POSITION; //projected position must be provided to the rasterizer stage in register SV_POSITION
    float3 color : COLOR;
    float3 viewSpacePos : VIEW_SPACE_POSITION;
    float3 viewSpaceNor : VIEW_SPACE_NORMAL;
};
PS_INPUT phongShadingVS(float3 v3fPosition : POSITION, float3 v3fNormal : NORMAL, float3 v3fColor : COLOR)
{
    PS_INPUT output = (PS_INPUT) 0;
    
    output.color = v3fColor;
    
    output.viewSpaceNor = mul(v3fNormal.xyz, (float3x3) worldNormalsMatrix); // vertex normal in view space
    
    output.viewSpacePos = mul(float4(v3fPosition.xyz, 1), worldViewMatrix).xyz; // vertex position in view space
    output.pos = mul(float4(output.viewSpacePos.xyz, 1), projectionMatrix); // project vertex position
    
    return output;
}
float4 phongShadingPS(PS_INPUT input) : SV_Target
{
    // calculate diffuse color
    float3 lightDirection = normalize(lightPosition - input.viewSpacePos);
    float3 normal = normalize(input.viewSpaceNor);
    float diffuse = saturate(dot(lightDirection, normal));
    
    float3 reflection = reflect(lightDirection, normal);
    float3 viewVector = normalize(-input.viewSpacePos.xyz);
    float3 specular = pow(dot(reflection, viewVector), shaderParams.w);
    
    return float4(saturate(shaderParams.x * ambientLight + shaderParams.y * diffuse * lightColor * input.color + shaderParams.z * specular * lightColor), 1);
}



//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 Render
{
    pass P0 //Terrain Shader
    {
        SetVertexShader(CompileShader(vs_4_0, TerrainVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, TerrainPS()));
        
        SetRasterizerState(rsCullNone);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
    pass P1_Mesh
    {
        SetVertexShader(CompileShader(vs_4_0, MeshVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, MeshPS()));

        SetRasterizerState(rsCullBack);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }

    pass P1_Mesh_Toon
    {
        SetVertexShader(CompileShader(vs_4_0, ToonVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, ToonPS()));

        SetRasterizerState(rsCullBack);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }

    pass P1_Mesh_Distance
    {
        SetVertexShader(CompileShader(vs_4_0, DistanceVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, DistancePS()));

        SetRasterizerState(rsCullBack);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }

    pass P1_Mesh_Flat
    {
        SetVertexShader(CompileShader(vs_4_0, FlatShadingVS()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, FlatShadingPS()));

        SetRasterizerState(rsCullBack);
        SetDepthStencilState(EnableDepth, 0);
        SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
    }
}