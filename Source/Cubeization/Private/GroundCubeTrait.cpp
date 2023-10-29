// Fill out your copyright notice in the Description page of Project Settings.

#include "GroundCubeTrait.h"
#include "MassEntityTemplateRegistry.h"

void UGroundCubeTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FGroundCubeFragment>();
}
