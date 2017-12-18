float4x4 mWorld_0_0 : WORLDT;
float4x4 mIWorld_0_0 : WORLDI;
float4x4 mWorldViewProj_0_0 : WORLDVIEWPROJECTION;
float4 mCamPos_0_0 : WORLD_CAMERA_POSITION = {0.000000f, 0.000000f, 0.000000f, 0.000000f};
float Opacity_1_0 : OPACITY
<
string UIName = "Opacity";
float UIMin = 0.0;
float UIMax = 1.0;
float UIStep = 0.01;
> = 1.000000f;
float SpecularLevel_1_0 : SPECULARLEVEL
<
string UIName = "SpecularLevel";
float UIMin = 0.0;
float UIMax = 1.0;
float UIStep = 0.01;
> = 0.370000f;
float GlossLevel_1_0 : GLOSSLEVEL
<
string UIName = "GlossLevel";
float UIMin = 0.0;
float UIMax = 100.0;
float UIStep = 1.0;
> = 100.000000f;
float4 LightDir0_1_0 : DIRECTION
<
string Object = "TargetLight";
string UIName = "LightDir0";
int refID = 0;
> = {0.000000f, 0.000000f, 0.000000f, 0.000000f};
float4 LightCol0_1_0 : LIGHTCOLOR
<
int LightRef = 0;
> = {0.000000f, 0.000000f, 0.000000f, 0.000000f};
float4 Emissive_1_0 : EMISSIVE
<
string UIName = "Emissive";
> = {0.000000f, 0.000000f, 0.000000f, 1.000000f};
float4 Specular_1_0 : SPECULAR
<
string UIName = "Specular";
> = {0.900000f, 0.900000f, 0.900000f, 1.000000f};
float4 Diffuse_1_0 : DIFFUSE
<
string UIName = "Diffuse";
> = {0.482353f, 0.913726f, 0.878431f, 1.000000f};


texture ReflectTex
<
int Texcoord = 0;
int MapChannel = 0;
string UIName = "ReflectTexture";
string name = "02-Default_ReflectTex.dds";
>;

sampler ReflectTexSampler = sampler_state
{
Texture = (ReflectTex);
};

VertexShader VS_pass0 = asm
{

vs_3_0

// Generated by ATI's Ashli Compiler on 2009-01-01 03:34:12.
// Ashli Version: 1.7.1


// Instructions (Alu):            18
// Registers    (Temp):           3
// Registers    (Constant):       14
// Registers    (Output):         1
// Registers    (OutputTexCoord): 2

dcl_position0 v0
dcl_normal0 v2

def c13, 0, 0, 0, 0

dcl_position o0

dcl_texcoord0 o3
dcl_texcoord2 o5

mul r0, c12, v0.r
mad r0, c11, v0.g, r0
mad r0, c10, v0.b, r0
mad r0, c9, v0.a, r0
mov r1, c13.r
dp4 r1.r, c8, v2
dp4 r1.g, c7, v2
dp4 r1.b, c6, v2
dp4 r1.a, c5, v2
mov r2, c13.r
dp4 r2.r, c4, v0
dp4 r2.g, c3, v0
dp4 r2.b, c2, v0
dp4 r2.a, c1, v0
add r2, c0, -r2
mov o0, r0
mov o3, r1
mov o5, r2

};


PixelShader PS_pass0 = asm
{

ps_3_0

// Generated by ATI's Ashli Compiler on 2009-01-01 03:34:12.
// Ashli Version: 1.7.1


// Instructions (Alu):            74
// Instructions (Tex):            1
// Registers    (Temp):           8
// Registers    (Constant):       14
// Registers    (TexCoord):       2
// Registers    (Texture):        1
// Registers    (OutputColor):    1

dcl_texcoord0 v2.rgba
dcl_texcoord2 v4.rgba

dcl_2d s0

def c0, -0.0114658, 0.31831, 0.0173528, 0.0223722
def c9, 0.0303819, 0.0446429, 0.075, 0.166667
def c10, 1.5708, 0.159155, 3.14159, 0.999974
def c11, -0.332568, 0.193235, -0.11573, 0.0519506
def c12, 1, 0, 0.95, 2
def c13, 0, 0, 0, 1

mov r0, c12.g
mov r0.rgb, c5
mov r0.a, c12.r
mov r2, c12
pow r1.r, r2.g, c3.r
mul r1.r, r1.r, c2.r
mul r1, c7, r1.r
dp3 r2.r, v2.rgb, v2.rgb
rsq r2.r, r2.r
mul r2, r2.r, v2.rgb
dp3 r3.r, r2, c4
max r3.r, r3.r, c12.g
min r3.r, r3.r, c12.r
mul r3, c8, r3.r
dp3 r4.r, r2, v4.rgb
mul r2, r4.r, r2
mad r2, -c12.a, r2, v4.rgb
dp3 r4.r, r2, r2
rsq r4.r, r4.r
mul r2, r4.r, r2
mul r4.r, -r2.b, -r2.b
mad r4.g, c0.b, r4.r, c0.a
mad r4.g, r4.r, r4.g, c9.r
mad r4.g, r4.r, r4.g, c9.g
mad r4.g, r4.r, r4.g, c9.b
mad r4.g, r4.r, r4.g, c9.a
mad r4.r, r4.r, r4.g, c12.r
mad r4.r, -r4.r, -r2.b, c10.r
dp3 r4.g, r2, r2
rsq r4.g, r4.g
mul r2, r4.g, r2
mul r2, r2, c12.b
abs r4.g, r2.r
abs r4.b, r2.g
cmp r4.a, -r2.g, r2.g, c12.r
cmp r4.a, r2.g, r4.a, -c12.r
mul r5.r, r4.a, c10.r
cmp r4.b, -r4.b, c12.g, r5.r
rcp r5.r, r2.r
mul r5.r, r2.g, r5.r
abs r5.g, r5.r
cmp r5.r, -r2.r, r5.g, r5.r
abs r5.g, r5.r
add r5.g, c12.r, -r5.g
rcp r5.b, r5.r
cmp r5.b, r5.g, r5.r, r5.b
mul r5.a, r5.b, r5.b
mov r7, c0
mad r6.r, r5.a, r7.r, c11.a
mad r6.r, r5.a, r6.r, c11.b
mad r6.r, r5.a, r6.r, c11.g
mad r6.r, r5.a, r6.r, c11.r
mad r5.a, r5.a, r6.r, c10.a
mul r5.b, r5.a, r5.b
cmp r5.a, -r5.r, r5.r, c12.r
cmp r5.r, r5.r, r5.a, -c12.r
mad r5.r, r5.r, c10.r, -r5.b
cmp r5.r, r5.g, r5.b, r5.r
add r5.g, c10.b, -r5.r
mul r4.a, r4.a, r5.g
cmp r2.r, -r2.r, r4.a, r5.r
cmp r2.r, -r4.g, r4.b, r2.r
mov r5, c12.g
mul r5.r, r2.r, c10.g
mul r5.g, r4.r, c0.g
texld r2, r5, s0
add r2, c13, r2
mov r4, c6
mad r2, r4, c8, r2
mad r2, r0, r3, r2
mad r0, r0, r1, r2
mov r0.a, c1.r
mov oC0, r0

};


technique technique0
{

pass
{

VertexShaderConstant[0] = <mCamPos_0_0>;
VertexShaderConstant[1] = <mWorld_0_0[3]>;
VertexShaderConstant[2] = <mWorld_0_0[2]>;
VertexShaderConstant[3] = <mWorld_0_0[1]>;
VertexShaderConstant[4] = <mWorld_0_0[0]>;
VertexShaderConstant[5] = <mIWorld_0_0[3]>;
VertexShaderConstant[6] = <mIWorld_0_0[2]>;
VertexShaderConstant[7] = <mIWorld_0_0[1]>;
VertexShaderConstant[8] = <mIWorld_0_0[0]>;
VertexShaderConstant[9] = <mWorldViewProj_0_0[3]>;
VertexShaderConstant[10] = <mWorldViewProj_0_0[2]>;
VertexShaderConstant[11] = <mWorldViewProj_0_0[1]>;
VertexShaderConstant[12] = <mWorldViewProj_0_0[0]>;
PixelShaderConstant[1] = <Opacity_1_0>;
PixelShaderConstant[2] = <SpecularLevel_1_0>;
PixelShaderConstant[3] = <GlossLevel_1_0>;
PixelShaderConstant[4] = <LightDir0_1_0>;
PixelShaderConstant[5] = <LightCol0_1_0>;
PixelShaderConstant[6] = <Emissive_1_0>;
PixelShaderConstant[7] = <Specular_1_0>;
PixelShaderConstant[8] = <Diffuse_1_0>;
Sampler[0] = (ReflectTexSampler);
VertexShader  = <VS_pass0>;
PixelShader  = <PS_pass0>;

}

}


bool UseLPRT = false;