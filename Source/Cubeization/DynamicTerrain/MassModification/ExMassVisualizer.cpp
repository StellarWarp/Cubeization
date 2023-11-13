// Fill out your copyright notice in the Description page of Project Settings.


#include "ExMassVisualizer.h"
#include "ExMassVisualizationComponent.h"

AExMassVisualizer::AExMassVisualizer(): AMassVisualizer()
{
	VisComponent->DestroyComponent();
	VisComponent = CreateDefaultSubobject<UExMassVisualizationComponent>(TEXT("ExVisualizerComponent"));
}
