// Fill out your copyright notice in the Description page of Project Settings.


#include "CubeGeometryCollectionComponent.h"


void UCubeGeometryCollectionComponent::OnChaosRemoval(const FChaosRemovalEvent& RemovalEvent)
{
	//destroy actor when all chunks are removed
}

UCubeGeometryCollectionComponent::UCubeGeometryCollectionComponent()
{
	//destroy actor when all chunks are removed
	// OnChaosRemovalEvent.AddDynamic(this, &UCubeGeometryCollectionComponent::OnChaosRemoval);
}
