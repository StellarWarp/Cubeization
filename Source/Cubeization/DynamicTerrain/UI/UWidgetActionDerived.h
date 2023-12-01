// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetAction.h"
#include "UObject/Object.h"
#include "DynamicTerrain/WorldDynamicTerrainControlSubsystem.h"
#include "UWidgetActionDerived.generated.h"

UCLASS()
class CUBEIZATION_API UWidgetActionStart : public UWidgetAction
{
	GENERATED_BODY()

public:
	virtual void Invoke_Implementation() override
	{
		ControlSubsystem->OnClickStart();
	}
};

UCLASS()
class CUBEIZATION_API UWidgetActionExit : public UWidgetAction
{
	GENERATED_BODY()

public:
	virtual void Invoke_Implementation() override
	{
		GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
	}
};

UCLASS()
class CUBEIZATION_API UWidgetActionPageJump : public UWidgetAction
{
	GENERATED_BODY()

public:
	virtual void Invoke_Implementation() override
	{
		if(!JumpPage)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("JumpPage is null"));
			return;
		}
		ControlSubsystem->ToNextPage(JumpPage);
	}
};


UCLASS()
class CUBEIZATION_API UWidgetActionPageReturn : public UWidgetAction
{
	GENERATED_BODY()

public:
	virtual void Invoke_Implementation() override
	{
		ControlSubsystem->ToPrevPage();
	}
};

UCLASS()
class CUBEIZATION_API UWidgetActionPageResume : public UWidgetAction
{
	GENERATED_BODY()

public:
	virtual void Invoke_Implementation() override
	{
		ControlSubsystem->OnClickContinue();
	}
};