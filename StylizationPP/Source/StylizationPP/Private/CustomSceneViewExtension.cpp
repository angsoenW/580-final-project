#include "CustomSceneViewExtension.h"


namespace
{
	TAutoConsoleVariable<int32> CVarShaderOn(
		TEXT("r.StylizationPP"),
		0,
		TEXT("Enable Custom SceneViewExtension \n")
		TEXT(" 0: OFF;")
		TEXT(" 1: ON."),
		ECVF_RenderThreadSafe);
}


FCustomSceneViewExtension::FCustomSceneViewExtension(const FAutoRegister& AutoRegister) : FSceneViewExtensionBase(AutoRegister)
{
	UE_LOG(LogTemp, Log, TEXT("StylizationPP: Custom SceneViewExtension registered"));
}

void FCustomSceneViewExtension::SubscribeToPostProcessingPass(EPostProcessingPass PassId, const FSceneView& View, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)
{
	// Define to what Post Processing stage to hook the SceneViewExtension into. See SceneViewExtension.h and PostProcessing.cpp for more info
	if (PassId == EPostProcessingPass::MotionBlur)
	{
		InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FCustomSceneViewExtension::CustomPostProcessing));
	}
}


FScreenPassTexture FCustomSceneViewExtension::CustomPostProcessing(FRDGBuilder& GraphBuilder, const FSceneView& SceneView, const FPostProcessMaterialInputs& Inputs)
{
	// SceneViewExtension gives SceneView, not ViewInfo so we need to setup some basics
	const FSceneViewFamily& ViewFamily = *SceneView.Family;

	const FScreenPassTexture& SceneColor = FScreenPassTexture::CopyFromSlice(GraphBuilder, Inputs.GetInput(EPostProcessMaterialInput::SceneColor));

	if (!SceneColor.IsValid() || CVarShaderOn.GetValueOnRenderThread() == 0)
	{
		return SceneColor;
	}
	
	const FScreenPassTextureViewport SceneColorViewport(SceneColor);
	
	// Here starts the RDG stuff
	RDG_EVENT_SCOPE(GraphBuilder, "Custom Postprocess Effect");
	{
		// Accesspoint to our Shaders
		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(ViewFamily.GetFeatureLevel());

		// Setup all the descriptors to create a target texture
		FRDGTextureDesc OutputDesc;
		{
			OutputDesc = SceneColor.Texture->Desc;

			OutputDesc.Reset();
			OutputDesc.Flags |= TexCreate_UAV;
			OutputDesc.Flags &= ~(TexCreate_RenderTargetable | TexCreate_FastVRAM);

			FLinearColor ClearColor(0., 0., 0., 0.);
			OutputDesc.ClearValue = FClearValueBinding(ClearColor);
		}

		// Create target texture
		FRDGTextureRef OutputTexture = GraphBuilder.CreateTexture(OutputDesc, TEXT("Custom Effect Output Texture"));

		// Set the shader parameters
		//FCustomShader::FParameters* PassParameters = GraphBuilder.AllocParameters<FCustomShader::FParameters>();
		FEdgeDetectionShader::FParameters* PassParameters = GraphBuilder.AllocParameters<FEdgeDetectionShader::FParameters>();

		// Input is the SceneColor from PostProcess Material Inputs
		PassParameters->OriginalSceneColor = SceneColor.Texture;

		// Use ScreenPassTextureViewportParameters so we don't need to calculate these ourselves
		PassParameters->SceneColorViewport = GetScreenPassTextureViewportParameters(SceneColorViewport);

		FIntPoint PassViewSize = SceneColor.ViewRect.Size();
		
		// Create UAV from Target Texture
		PassParameters->Output = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutputTexture));

		// Set Compute Shader and execute
		FIntVector GroupCount = FComputeShaderUtils::GetGroupCount(PassViewSize, FComputeShaderUtils::kGolden2DGroupSize);

		//TShaderMapRef<FCustomShader> ComputeShader(GlobalShaderMap);
		TShaderMapRef<FEdgeDetectionShader> ComputeShader(GlobalShaderMap);

		FComputeShaderUtils::AddPass(
			GraphBuilder,
			RDG_EVENT_NAME("Custom SceneViewExtension Post Processing CS Shader %dx%d", PassViewSize.X, PassViewSize.Y),
			ComputeShader,
			PassParameters,
			GroupCount);

		// Copy the output texture back to SceneColor
		// Returning the new texture as ScreenPassTexture doesn't work, so this is pretty fast alternative
		// Also with f.ex 'PrePostProcessPass_RenderThread' you get only input and something similar needs to be implemented then
		AddCopyTexturePass(GraphBuilder, OutputTexture, SceneColor.Texture);
	}

	// The call expects ScreenPassTexture as a return, we return with the same texture as we started with, see AddCopyTexturePass above 
	return SceneColor;
}


//void ShaderExampleSceneViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder,
//	const FViewInfo& InView, const FPostProcessingInputs& Inputs)
//{
//	/// 1.从预定义参数结构体中获取场景纹理与视口信息
//	const FIntRect Viewport = InView.ViewRect;
//	const FSceneTextureShaderParameters SceneTextures = CreateSceneTextureShaderParameters(GraphBuilder, InView, ESceneTextureSetupMode::SceneColor | ESceneTextureSetupMode::GBuffers);
//
//	// 包装场景颜色纹理及其视口，确保后续渲染仅影响该区域
//	const FScreenPassTexture SceneColorTexture((*Inputs.SceneTextures)->SceneColorTexture, Viewport);
//
//	/// 2.为全局着色器配置输入与输出
//	FSampleGlobalShader::FParameters* Parameters = GraphBuilder.AllocParameters<FSampleGlobalShader::FParameters>();
//	//将对应的输入参数赋值给Shader声明的输入参数
//	Parameters->SceneColorTexture = SceneColorTexture.Texture;
//	Parameters->SceneTextures = SceneTextures;
//	Parameters->TargetColour = FVector3f(1.0f, 0.9f, 0.7f);
//
//	//绑定输出渲染目标（此处直接复用输入的场景颜色纹理）
//	Parameters->RenderTargets[0] = FRenderTargetBinding((*Inputs.SceneTextures)->SceneColorTexture, ERenderTargetLoadAction::ELoad);
//
//	/// 3.执行全屏幕渲染通道
//	//从全局shader映射中获取我们定义的Global Shader实例
//	const FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
//	TShaderMapRef<FSampleGlobalShader> PixelShader(GlobalShaderMap);
//
//	// 在全屏幕渲染通道将我们的执行命令加到RDG中
//	FPixelShaderUtils::AddFullscreenPass(GraphBuilder, GlobalShaderMap, FRDGEventName(TEXT("Color Calibration Pass")), PixelShader, Parameters, Viewport);
//}
