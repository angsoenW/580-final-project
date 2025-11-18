// Copyright Epic Games, Inc. All Rights Reserved.

#include "StylizationPP.h"

#define LOCTEXT_NAMESPACE "FStylizationPPModule"

void FStylizationPPModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("StylizationPP"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugins/StylizationPP"), PluginShaderDir);
}

void FStylizationPPModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FStylizationPPModule, StylizationPP)