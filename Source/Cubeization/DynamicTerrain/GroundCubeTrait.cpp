// Fill out your copyright notice in the Description page of Project Settings.

#include "GroundCubeTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "TerrainRepresentationSubsystem.h"

void UGroundCubeTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	BuildContext.AddFragment<FGroundCubeFragment>();

	UTerrainRepresentationSubsystem* TerrainSubsystem = World.GetSubsystem<UTerrainRepresentationSubsystem>();
	TerrainSubsystem->SetReplacingActorClass(ReplacingActorClass);
}
