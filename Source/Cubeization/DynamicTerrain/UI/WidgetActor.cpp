// Fill out your copyright notice in the Description page of Project Settings.


#include "WidgetActor.h"
#include "DynamicTerrain/WorldDynamicTerrainControlSubsystem.h"


// Sets default values
AWidgetActor::AWidgetActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AWidgetActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWidgetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWidgetActor::NotifySelected_Implementation()
{
	Action->ControlSubsystem = GetWorld()->GetSubsystem<UWorldDynamicTerrainControlSubsystem>();
	Action->Invoke();
}

void AWidgetActor::NotifyStartDestroy_Implementation()
{
}

void AWidgetActor::BindAction(TSubclassOf<UWidgetAction> ActionClass,TObjectPtr<UPageConfig> JumpPage = nullptr)
{
	if(!ActionClass)
	{
		UE_LOG(LogTemp,Warning,TEXT("ActionClass is null"));
		return;
	}
	Action = NewObject<UWidgetAction>(this, ActionClass);
	Action->JumpPage = JumpPage;
}

