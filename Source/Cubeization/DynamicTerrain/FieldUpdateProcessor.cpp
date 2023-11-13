// // Fill out your copyright notice in the Description page of Project Settings.
//
//
// #include "FieldUpdateProcessor.h"
//
// #include "FieldGlobalData.h"
// #include "MassCommonTypes.h"
// #include "MassExecutionContext.h"
// #include "Kismet/GameplayStatics.h"
//
// UFieldUpdateProcessor::UFieldUpdateProcessor()
// {
// 	bAutoRegisterWithProcessingPhases = true;
// 	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::All);
// 	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Avoidance);
// }
//
// void UFieldUpdateProcessor::ConfigureQueries()
// {
// }
//
// void UFieldUpdateProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
// {
// 	UFieldDataSubsystem* FieldData = Context.GetWorld()->GetSubsystem<UFieldDataSubsystem>();
// 	
// 	auto& HeightField = FieldData->HeightField;
// 	auto& VelocityField = FieldData->VelocityField;
//
// 	//position update
// 	auto& rawHeightField = HeightField.GetData();
// 	auto& rawVelocityField = VelocityField.GetData();
//
// 	for (int i = 0; i < rawHeightField.Num(); ++i)
// 	{
// 		rawHeightField[i] += rawVelocityField[i] * Context.GetDeltaTimeSeconds();
// 	}
// }
