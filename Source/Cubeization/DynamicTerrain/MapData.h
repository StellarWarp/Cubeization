// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Utils/FieldArray.h"
#include "MapData.generated.h"

/**
 * 
 */
UCLASS()
class CUBEIZATION_API UMapData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<float> MapRawData;

	UPROPERTY(EditAnywhere)
	uint32 MapSizeX;
	
	UPROPERTY(EditAnywhere)
	uint32 MapSizeY;


	TFieldArray<float> ToFieldArray() const
	{
		return TFieldArray<float>(MapRawData, MapSizeX, MapSizeY);
	}
};
