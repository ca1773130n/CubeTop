
/*** Generated through Lumonix shaderFX  by: Admin in 3dsmax at: 2008-07-31 ¿ÀÈÄ 10:20:38  ***/ 

// This FX shader was built to support FX Composer. 


float4 UIColor_6274
<
	string UIName = "Color";
	string UIWidget = "Color";
> = {0.582733f, 0.841726f, 0.798561f, 1.0f };
 
texture NormalMap_2976
<
	string ResourceName = "N_blank.dds";
	string UIName = "Normal Map";
	string ResourceType = "2D";
>;
 
sampler2D NormalMap_2976Sampler = sampler_state
{
	Texture = <NormalMap_2976>;
	MinFilter = ANISOTROPIC;
	MagFilter = ANISOTROPIC;
	MipFilter = ANISOTROPIC;
};
 
float4x4 viewInv : ViewInverse < string UIWidget = "None"; >;  
float4x4 world : World < string UIWidget = "None"; >;  
 
texture ReflectCube : environment
<
	string ResourceName = "sunol_cubemap.dds";
	string UIName = "Reflect Cube";
	string ResourceType = "Cube";
>;
 
samplerCUBE ReflectCubeSampler = sampler_state
{
	Texture = <ReflectCube>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
};
 
float4x4 worldI : WorldInverse < string UIWidget = "None"; >;  
texture UICubeMap_5987 : environment
<
	string ResourceName = "sunol_cubemap.dds";
	string UIName = "Red Cube Map";
	string ResourceType = "Cube";
>;
 
samplerCUBE UICubeMap_5987Sampler = sampler_state
{
	Texture = <UICubeMap_5987>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
};
 
texture UICubeMap_2500 : environment
<
	string ResourceName = "sunol_cubemap.dds";
	string UIName = "Green Cube Map";
	string ResourceType = "Cube";
>;
 
samplerCUBE UICubeMap_2500Sampler = sampler_state
{
	Texture = <UICubeMap_2500>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
};
 
texture UICubeMap_3600 : environment
<
	string ResourceName = "sunol_cubemap.dds";
	string UIName = "Blue Cube Map";
	string ResourceType = "Cube";
>;
 
samplerCUBE UICubeMap_3600Sampler = sampler_state
{
	Texture = <UICubeMap_3600>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
};
 
float FresnelPower
<
	string UIWidget = "slider";
	float UIMin = 0.0;
	float UIMax = 100.0;
	float UIStep = 0.01;
	string UIName = "Fresnel Power";
> = 1.0;
 
float FresnelScale
<
	string UIWidget = "slider";
	float UIMin = 0.0;
	float UIMax = 100.0;
	float UIStep = 0.01;
	string UIName = "Fresnel Scale";
> = 1.0;
 
float FresnelBias
<
	string UIWidget = "slider";
	float UIMin = -100.0;
	float UIMax = 100.0;
	float UIStep = 0.01;
	string UIName = "Fresnel Bias";
> = 0.0;
 
float UIConst_6892
<
	string UIWidget = "slider";
	float UIMin = 0.0;
	float UIMax = 10.0;
	float UIStep = 0.1;
	string UIName = "Glossiness";
> = 64.0;
 
float4 SpecularColor : Specular
<
	string UIName = "Specular Color";
	string UIWidget = "Color";
> = {0.501961f, 0.501961f, 0.501961f, 1.0f };
 


/************** light info **************/ 

float3 light1Pos : POSITION 
< 
	string Object = "PointLight"; 
	string Space = "World"; 
> = {100.0f, 100.0f, 100.0f}; 

//---------------------------------- 

float4 light1Color  : SPECULAR 
< 
	string UIName =  "Light1 Color"; 
	string Object = "Pointlight"; 
	string UIWidget = "Color"; 
> = { 1.0f, 1.0f, 1.0f, 1.0f}; 

//---------------------------------- 

float4x4 wvp : WorldViewProjection < string UIWidget = "None"; >;  
// input from application 
	struct a2v { 
	float4 position		: POSITION; 

	float2 texCoord		: TEXCOORD0; 
	float3 tangent		: TANGENT; 
	float3 binormal		: BINORMAL; 
	float3 normal		: NORMAL; 

}; 

// output to fragment program 
struct v2f { 
        float4 position    		: POSITION; 

		float2 texCoord			: TEXCOORD0; 
        float3 eyeVec	    	: TEXCOORD1; 
        float3 lightVec    		: TEXCOORD2; 

}; 

v2f av(a2v In, uniform float3 lightPosition) 
{ 
	v2f Out = (v2f)0; 
    Out.position = mul(In.position, wvp);				//transform vert position to homogeneous clip space 

	//this code was added by the texture map Node 
    Out.texCoord = In.texCoord;						//pass through texture coordinates from channel 1 
	//this code was added by the Eye Vector Node 
    float3x3 objTangentXf;								//build object to tangent space transform matrix 
    	objTangentXf[0] = In.tangent; 
    	objTangentXf[1] = -In.binormal; 
    	objTangentXf[2] = In.normal; 
	//these three lines were added by the Eye Vector Node 
    float4 osIPos = mul(viewInv[3], worldI);			//put world space eye position in object space 
    float3 osIVec = osIPos.xyz - In.position.xyz;		//object space eye vector 
    Out.eyeVec = mul(objTangentXf, osIVec);				//tangent space eye vector passed out 

    return Out; 
} 

	//Single Pass Pixel Shader
float4 af(v2f In, uniform float4 lightColor) : COLOR 
{ 

	float4 NormalMap_2976 = tex2D(NormalMap_2976Sampler, In.texCoord.xy);
	NormalMap_2976.xyz = NormalMap_2976.xyz * 2 - 1;		//expand to -1 to 1 range 
	NormalMap_2976.y = -NormalMap_2976.y; 		// green channel flipped 
	NormalMap_2976.rgb = normalize(NormalMap_2976.rgb); 		//normalized the normal vector 
    viewInv[3].x = -viewInv[3].x;		//negate eye vector for proper reflections 
    viewInv[3].y = -viewInv[3].y;		//negate eye vector for proper reflections 
    viewInv[3].z = -viewInv[3].z;		//negate eye vector for proper reflections 
    	float3 viewDir = mul(viewInv[3], world) ;	// Adjust for world space position of object 
    float3 Reflect = reflect(viewDir, NormalMap_2976.rgb);		//Compute the reflection vector 
	float4 ReflectCube = texCUBE(ReflectCubeSampler, Reflect.xyz);
    float3 V = normalize(In.eyeVec.xyz);		//normalized eye vector 
	float RedIndex = 0.89; 
    float3 RefractRed = refract( V, -NormalMap_2976.rgb, RedIndex );		//Compute the refraction vector 
    RefractRed.xyz = -RefractRed.xyz;  //negate for right side up refraction in Y up 
    RefractRed.x = -RefractRed.x;		//negate 
    RefractRed.y = -RefractRed.y;		//negate 
    RefractRed.z = -RefractRed.z;		//negate 
	float4 UICubeMap_5987 = texCUBE(UICubeMap_5987Sampler, RefractRed.xyz);
	float GreenIndex = 0.9; 
    float3 RefractGreen = refract( V, -NormalMap_2976.rgb, GreenIndex );		//Compute the refraction vector 
    RefractGreen.xyz = -RefractGreen.xyz;  //negate for right side up refraction in Y up 
    RefractGreen.x = -RefractGreen.x;		//negate 
    RefractGreen.y = -RefractGreen.y;		//negate 
    RefractGreen.z = -RefractGreen.z;		//negate 
	float4 UICubeMap_2500 = texCUBE(UICubeMap_2500Sampler, RefractGreen.xyz);
	float BlueIndex = 0.91; 
    float3 RefractBlue = refract( V, -NormalMap_2976.rgb, BlueIndex );		//Compute the refraction vector 
    RefractBlue.xyz = -RefractBlue.xyz;  //negate for right side up refraction in Y up 
    RefractBlue.x = -RefractBlue.x;		//negate 
    RefractBlue.y = -RefractBlue.y;		//negate 
    RefractBlue.z = -RefractBlue.z;		//negate 
	float4 UICubeMap_3600 = texCUBE(UICubeMap_3600Sampler, RefractBlue.xyz);
	float3 FinalRefractionColor = float3(UICubeMap_5987.r, UICubeMap_2500.g, UICubeMap_3600.b);
	float NdotV_fresnel = saturate(dot(V,NormalMap_2976.rgb)); 		//clamped dot product 
	float Pow_NdotV = pow(NdotV_fresnel,FresnelPower);		//x to the power of y 
	float timesFresnelScale = Pow_NdotV * FresnelScale;
	float plusFresnelBias = timesFresnelScale + FresnelBias;
	float ClampFresnel = saturate(plusFresnelBias); 
	float3 LerpReflectionAndRefraction = lerp(ReflectCube.rgb, FinalRefractionColor, ClampFresnel); 
	float3 MathOperator_718 = UIColor_6274 * LerpReflectionAndRefraction;
	float3 input1 = MathOperator_718; 

	float4 ret =  float4(input1, 1); 
	return ret; 
} 

v2f v(a2v In, uniform float3 lightPosition) 
{ 
	v2f Out = (v2f)0; 
    Out.position = mul(In.position, wvp);				//transform vert position to homogeneous clip space 

	//this code was added by the texture map Node 
    Out.texCoord = In.texCoord;						//pass through texture coordinates from channel 1 
	//this code was added by the Light Vector Node 
    float3x3 objTangentXf;								//build object to tangent space transform matrix 
    	objTangentXf[0] = In.tangent; 
    	objTangentXf[1] = -In.binormal; 
    	objTangentXf[2] = In.normal; 
	//these three lines were added by the Light Vector Node 
    float3 osLPos = mul(lightPosition, worldI);			//put world space light position in object space 
    float3 osLVec = osLPos - In.position.xyz;		//object space light vector 
    Out.lightVec = mul(objTangentXf, osLVec);			//tangent space light vector passed out 
	//these three lines were added by the Eye Vector Node 
    float4 osIPos = mul(viewInv[3], worldI);			//put world space eye position in object space 
    float3 osIVec = osIPos.xyz - In.position.xyz;		//object space eye vector 
    Out.eyeVec = mul(objTangentXf, osIVec);				//tangent space eye vector passed out 

    return Out; 
} 

	//Multi-Pass Pixel Shader
float4 f(v2f In, uniform float4 lightColor) : COLOR 
{ 

	float4 NormalMap_2976 = tex2D(NormalMap_2976Sampler, In.texCoord.xy);
	NormalMap_2976.xyz = NormalMap_2976.xyz * 2 - 1;		//expand to -1 to 1 range 
	NormalMap_2976.y = -NormalMap_2976.y; 		// green channel flipped 
	NormalMap_2976.rgb = normalize(NormalMap_2976.rgb); 		//normalized the normal vector 
	float3 L = normalize(In.lightVec.xyz);	//normalized light vector 
    float3 V = normalize(In.eyeVec.xyz);		//normalized eye vector 
	float3 HalfAngle = L + V;
	float3 NormalizedHalfAngle = normalize(HalfAngle);		//Normalize 
	float NdotH_specular = saturate(dot(NormalizedHalfAngle,NormalMap_2976.rgb)); 		//clamped dot product 
	float NdotH_pow_Glossiness = pow(NdotH_specular,UIConst_6892);		//x to the power of y 
	float3 SpecularLight = SpecularColor * NdotH_pow_Glossiness;
	float3 input2 = SpecularLight; 

	float4 ret =  float4(input2, 1); 
	return ret; 
} 

technique Complete  
{  
	pass ambient  
    {		 
	VertexShader = compile vs_1_1 av(light1Pos); 
	ZEnable = true; 
	ZWriteEnable = true; 
	CullMode = ccw; 
	AlphaBlendEnable = false; 
	PixelShader = compile ps_3_0 af(light1Color); 
	}  

	pass light1  
    {		 
	VertexShader = compile vs_1_1 v(light1Pos); 
	ZEnable = true; 
	CullMode = ccw; 
	AlphaBlendEnable = true; 
	ZWriteEnable = true; 
	ZFunc = LessEqual; 
	SrcBlend = One; 
	DestBlend = One; 
	PixelShader = compile ps_3_0 f(light1Color); 
	}  

}  

