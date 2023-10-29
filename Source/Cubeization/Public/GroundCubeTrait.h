// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MassEntityTraitBase.h"
#include "MassEntityTypes.h"
#include "GroundCubeTrait.generated.h"


USTRUCT()
struct FGroundCubeFragment : public FMassFragment
{
	GENERATED_BODY()

	UPROPERTY()
	FIntVector2 Index;
};

UCLASS()
class CUBEIZATION_API UGroundCubeTrait : public UMassEntityTraitBase
{
	GENERATED_BODY()

protected:
	void BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const override;
};
