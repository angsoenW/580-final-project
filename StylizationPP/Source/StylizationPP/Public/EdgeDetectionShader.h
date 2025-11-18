#pragma once

#include "CoreMinimal.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "ShaderParameterMacros.h"
#include "RHIStaticStates.h"
#include "RenderGraphUtils.h"
#include "ScreenPass.h"


class STYLIZATIONPP_API FEdgeDetectionShader : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FEdgeDetectionShader)

	SHADER_USE_PARAMETER_STRUCT(FEdgeDetectionShader, FGlobalShader)

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_STRUCT(FScreenPassTextureViewportParameters, SceneColorViewport)
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, OriginalSceneColor)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Output)
	END_SHADER_PARAMETER_STRUCT()

	// Basic shader initialization
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	// Define environment variables used by compute shader
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("THREADS_X"), 8);
		OutEnvironment.SetDefine(TEXT("THREADS_Y"), 8);
		OutEnvironment.SetDefine(TEXT("THREADS_Z"), 1);
	}
};