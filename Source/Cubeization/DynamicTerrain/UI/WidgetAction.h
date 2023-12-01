// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WidgetAction.generated.h"

/**
 * 
 */
class UPageConfig;

UCLASS()
class CUBEIZATION_API UWidgetAction : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TObjectPtr<UPageConfig> JumpPage;

	//todo improve this
	//cpp temp access only
	class UWorldDynamicTerrainControlSubsystem* ControlSubsystem;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Invoke();
	
	virtual void Invoke_Implementation()
	{
		UE_LOG(LogTemp, Warning, TEXT("Invoke Require Implementation"));
	};
};
