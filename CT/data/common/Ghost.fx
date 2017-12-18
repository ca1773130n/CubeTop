//ghost shader based on nvidia's cgFX sample

// un-tweakables

float4x4 worldIT:	WorldIT;
float4x4 wvp : 		WorldViewProj;
float4x4 world : 	World;
float4x4 viewIT : 	ViewIT;

// tweakables


int lodBias<
	string UIName = "LOD Bias";
	string UIType = "IntSpinner";
	float UIMin = 0;
	float UIMax = 10;	
>  = 0;


float fade <
	string UIName = "Faderizer";
	string UIType = "FloatSpinner";
	float UIMin = 0.0f;
	float UIMax = 20.0f;	
>  = 1.0f;

texture diffuseTexture : DiffuseMap
<
	string UIName = "Diffuse Map";
	string name = "none";
	string TextureType = "2D";
>;	

//////// techniques ////////////////////////////

technique Ghostly
{
	pass p0
	{
		VertexShaderConstant[0] = <wvp>;
		VertexShaderConstant[4] = <worldIT>;
		VertexShaderConstant[8] = <world>;

		VertexShaderConstant[11] = {1.1,1.1,0.6,0.0};
		VertexShaderConstant[12] = <fade>;

		VertexShaderConstant[16] = <viewIT>;

		VertexShader = 
	        asm
	        {
			vs_1_1
	
			dcl_position  v0		
			dcl_normal    v3		
			dcl_texcoord0  v7		
			dcl_texcoord1  v8			
		
	
			// Transform pos to screen space.
			m4x4 oPos, v0, c0

			// Normal to world space:
			dp3 r0.x, v3, c4
			dp3 r0.y, v3, c5
			dp3 r0.z, v3, c6

			// normalize normal
			dp3 r0.w, r0, r0
			rsq r0.w, r0.w
			mul r0, r0, r0.w	// r0 has normalized normal.

			// vpos to world space.
			dp4 r1.x, v0, c8
			dp4 r1.y, v0, c9
			dp4 r1.z, v0, c10
			dp4 r1.w, v0, c11	// r1 has position in world space.

			// eye vector, normalize.
			add r2, c19, -r1

			dp3 r2.w, r2, r2
			rsq r2.w, r2.w
			mul r2, r2, r2.w	// r2 has normalized eye vector.

			// E dot N
			dp3 r0, r2, r0
			mul r0, r0, r0
			mul r0, r0, c12.xxxx
			mul oD0, r0, r0

			mov oT0, v7
		};

		ZEnable = true;
		ZWriteEnable = true;
		AlphaBlendEnable = true;

		SrcBlend = One;
		DestBlend = InvSrcColor;

		CullMode = CCW;
		Lighting = false;

		Texture[0] = <diffuseTexture>;
		MinFilter[0] = Linear;
		MagFilter[0] = Linear;
		MipFilter[0] = Linear;
		MipMapLodBias[0] = <lodBias>;

		MinFilter[1] = Linear;
		MagFilter[1] = Linear;
		MipFilter[1] = None;
		MipMapLodBias[1] = <lodBias>;
		AddressU[1] = Clamp;
		AddressV[1] = Clamp;
        

		ColorOp[0] = Modulate;
		ColorArg1[0] = Texture;
		ColorArg2[0] = Diffuse | Complement;

		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Texture;
		AlphaArg2[0] = Diffuse;

	}

	pass p1
	{
		VertexShaderConstant[0] = <wvp>;
		VertexShaderConstant[4] = <worldIT>;
		VertexShaderConstant[8] = <world>;

		VertexShaderConstant[11] = {1.1,1.1,0.6,0.0};
		VertexShaderConstant[12] = {1.0,1.1,0.6,0.0};

		VertexShaderConstant[16] = <viewIT>;

		VertexShader = 
		asm
		{
	        
			vs_1_1

			dcl_position  v0		
			dcl_normal    v3		
			dcl_texcoord0  v7		
			dcl_texcoord1  v8			

			// Transform pos to screen space.
			m4x4 oPos, v0, c0

			// Normal to world space:
			dp3 r0.x, v3, c4
			dp3 r0.y, v3, c5
			dp3 r0.z, v3, c6

			// normalize normal
			dp3 r0.w, r0, r0
			rsq r0.w, r0.w
			mul r0, r0, r0.w			// r0 has normalized normal.

			// vpos to world space.
			dp4 r1.x, v0, c8
			dp4 r1.y, v0, c9
			dp4 r1.z, v0, c10
			dp4 r1.w, v0, c11			// r1 has position in world space.

			// eye vector, normalize.
			add r2, c19, -r1

			dp3 r2.w, r2, r2
			rsq r2.w, r2.w
			mul r2, r2, r2.w			// r2 has normalized eye vector.

			// E dot N
			dp3 r0, r2, r0
			mul r0, r0, r0
			mul r0, r0, c12.xxxx
			mul oD0, r0, r0

			mov oT0, v7
		};

		ZEnable = true;
		ZWriteEnable = true;
		AlphaBlendEnable = true;
		CullMode = CW;
		Lighting = false;

		SrcBlend = One;
		DestBlend = InvSrcColor;
		Texture[0] = <diffuseTexture>;
		
		MinFilter[0] = Linear;
		MagFilter[0] = Linear;
		MipFilter[0] = Linear;

		MinFilter[1] = Linear;
		MagFilter[1] = Linear;
		MipFilter[1] = None;
		AddressU[1] = Clamp;
		AddressV[1] = Clamp;

		ColorOp[0] = Modulate;
		ColorArg1[0] = Texture;
		ColorArg2[0] = Diffuse | Complement;

		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Texture;
		AlphaArg2[0] = Diffuse;
	}
}
