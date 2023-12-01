// Fill out your copyright notice in the Description page of Project Settings.


#include "PageSetting.h"

#include "UPageConfig.h"
#include "DynamicTerrain/TerrainDataSubsystem.h"
#include "DynamicTerrain/WorldDynamicTerrainControlSubsystem.h"
#include "Utils/BlueprintUtils.h"


// Sets default values
APageSetting::APageSetting()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
}

// Called when the game starts or when spawned
void APageSetting::BeginPlay()
{
	Super::BeginPlay();
	UWorldDynamicTerrainControlSubsystem* WorldDynamicTerrainControlSubsystem = GetWorld()->GetSubsystem<UWorldDynamicTerrainControlSubsystem>();
	WorldDynamicTerrainControlSubsystem->StartPage = StartPage;
	WorldDynamicTerrainControlSubsystem->PausePage = PausePage;
	WorldDynamicTerrainControlSubsystem->AddPage(StartPage);
	WorldDynamicTerrainControlSubsystem->SetFieldDeactivationParameter(DeactivationParameter);
	WorldDynamicTerrainControlSubsystem->SetFieldActivationParameter(ActivationParameter);
	WorldDynamicTerrainControlSubsystem->ActivationHeight = ActivationHeight;
	WorldDynamicTerrainControlSubsystem->DeactivationHeight = DeactivationHeight;

	if(bMainMenuInit)
	{
		InitCurrentPage();
		UBlueprintUtils::SetGameSuspend(true);
	}
	
}

void APageSetting::InitCurrentPage()
{
	UWorldDynamicTerrainControlSubsystem* WorldDynamicTerrainControlSubsystem = GetWorld()->GetSubsystem<UWorldDynamicTerrainControlSubsystem>();
	//parameters
	WorldDynamicTerrainControlSubsystem->SequentialActiveLastingTime = SequentialActivationTime;
	WorldDynamicTerrainControlSubsystem->SequentialActiveExternalWaitTime = ExternalWaitTime;
	WorldDynamicTerrainControlSubsystem->SequentialActiveInterpolationTime = InterpolationTime;
	WorldDynamicTerrainControlSubsystem->SequentialActiveMaxDistance = MaxDistance;
	WorldDynamicTerrainControlSubsystem->InitCurrentPage();
}


void APageSetting::EnterGame()
{
	// UWorldDynamicTerrainControlSubsystem* WorldDynamicTerrainControlSubsystem = GetWorld()->GetSubsystem<UWorldDynamicTerrainControlSubsystem>();
	// WorldDynamicTerrainControlSubsystem->SetFieldActivation(true);
	// WorldDynamicTerrainControlSubsystem->ResetActivationArea();
}

