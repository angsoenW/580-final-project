// Fill out your copyright notice in the Description page of Project Settings.


#include "StylizationSubsystem.h"
#include "CustomSceneViewExtension.h"
#include "SceneViewExtension.h"

void UStylizationSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	CustomSceneViewExtension = FSceneViewExtensions::NewExtension<FCustomSceneViewExtension>();
	UE_LOG(LogTemp, Log, TEXT("SceneViewExtensionTemplate: Subsystem initialized & SceneViewExtension created"));
}

void UStylizationSubsystem::Deinitialize()
{
	{
		CustomSceneViewExtension->IsActiveThisFrameFunctions.Empty();

		FSceneViewExtensionIsActiveFunctor IsActiveFunctor;

		IsActiveFunctor.IsActiveFunction = [](const ISceneViewExtension* SceneViewExtension, const FSceneViewExtensionContext& Context)
			{
				return TOptional<bool>(false);
			};

		CustomSceneViewExtension->IsActiveThisFrameFunctions.Add(IsActiveFunctor);
	}

	CustomSceneViewExtension.Reset();
	CustomSceneViewExtension = nullptr;
}
