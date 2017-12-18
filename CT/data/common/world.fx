struct PixelInput {
	float4 norm		: TEXCOORD0;
	float4 worldpos	: TEXCOORD1;
	float4 eyevector	: TEXCOORD2;
	float4 tangent		: TEXCOORD3;
	float4 binormal	: TEXCOORD4;
	float4 objpos		: TEXCOORD5;
	float4 tex0		: TEXCOORD6;
	float4 tex1		: TEXCOORD7;
};


struct PixelOutput {
	float4 c 		: COLOR;
};


uniform sampler2D ReflectTex;
uniform float4 Diffuse = float4(0.482353,0.913726,0.878431,1.000000);
uniform float4 Specular = float4(0.900000,0.900000,0.900000,1.000000);
uniform float4 Emissive = float4(0.000000,0.000000,0.000000,1.0f);
uniform float SpecularLevel = 0.370000;
uniform float GlossLevel = 100.000000;
uniform float3 LightCol0;
uniform float3 LightDir0;
uniform float LightFallOff0;
uniform float LightHotSpot0;
uniform float Opacity = 1.000000;

PixelOutput fragmentEntry(PixelInput pi)
{
	PixelOutput PO;
	float3 N;
	N = normalize(pi.norm.xyz);
	float3 Eye;
	Eye.xyz = normalize(pi.eyevector.xyz);
	float4 nColor = float4(0.0,0.0,0.0,1.0);
	float3 Refl;
	float4 Reflection;
	float reflu;
	float reflv;
	float2 tr;
	Refl = reflect(pi.eyevector.xyz, N);
	Refl = normalize(Refl);
	tr = Refl;
	tr = normalize(tr) * 0.95;
	reflu = atan2(tr.y,tr.x);
	reflu = reflu /(2.0* PI);
	reflv = acos(-Refl.z);
	reflv = reflv /(1.0* PI);
	Reflection =  tex2D(ReflectTex, float2(reflu, reflv));
	float4 mDiffuse = Diffuse;
	float4 mSpecular = Specular;
	float4 SelfIllum = Emissive;
	nColor = nColor + Reflection;
	nColor = nColor + SelfIllum * mDiffuse;
	float3 Light;
	float f;
	float fAtt;
	float fDist;
/*  default light */
	fAtt = 1.0;
	nColor = nColor + float4(LightCol0,1.0) * mDiffuse * clamp(dot(N,LightDir0),0,1) * fAtt;
	f = pow(f, GlossLevel);
	f = f * SpecularLevel;
	nColor = nColor + float4(LightCol0,1.0) * mSpecular * f * fAtt;
/* end of default light */
	nColor.a = Opacity;
	PO.c = nColor;
	return PO;
}

