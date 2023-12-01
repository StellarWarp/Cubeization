// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WidgetAction.h"
#include "GameFramework/Actor.h"
#include "WidgetActor.generated.h"


UCLASS()
class CUBEIZATION_API AWidgetActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<UWidgetAction> Action;
public:
	// Sets default values for this actor's properties
	AWidgetActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//this event need to bind in blueprint and call parent function
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void NotifySelected();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void NotifyStartDestroy();
	virtual void NotifyStartDestroy_Implementation();

	void BindAction(TSubclassOf<UWidgetAction> ActionClass, TObjectPtr<UPageConfig> JumpPage);
};
