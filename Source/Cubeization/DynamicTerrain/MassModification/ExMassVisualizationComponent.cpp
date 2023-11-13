// Fill out your copyright notice in the Description page of Project Settings.


#include "ExMassVisualizationComponent.h"
#include "Logging/LogMacros.h"
#include "ExMassInstancedStaticMeshInfo.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "PhysicsInterfaceDeclaresCore.h"
#include "PhysicsProxy/SingleParticlePhysicsProxyFwd.h"
#include "Chaos/ChaosEngineInterface.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

UExMassVisualizationComponent::UExMassVisualizationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UExMassVisualizationComponent::ExConstructStaticMeshComponents()
{
	UE_LOG(LogTemp, Warning, TEXT("ExConstructStaticMeshComponents"));
	AActor* ActorOwner = GetOwner();
	check(ActorOwner);
	ActorOwner->SetActorEnableCollision(true);

	UE_MT_SCOPED_WRITE_ACCESS(InstancedStaticMeshInfosDetector);
	for (FMassInstancedStaticMeshInfo& InfoSuper : InstancedStaticMeshInfos)
	{
		auto& Info = *static_cast<FExMassInstancedStaticMeshInfo*>(&InfoSuper);
		// Check if it is already created
		if (!Info.InstancedStaticMeshComponents.IsEmpty())
		{
			continue;
		}

		// Check if there are any specified meshes for this visual type
		if (Info.Desc.Meshes.Num() == 0)
		{
			//todo this has a link error
			// UE_LOG(LogMassRepresentation, Error, TEXT("No associated meshes for this intanced static mesh type"));
			continue;
		}
		for (const FStaticMeshInstanceVisualizationMeshDesc& MeshDesc : Info.Desc.Meshes)
		{
			FISMCSharedData* SharedData = ISMCSharedData.Find(MeshDesc);
			UInstancedStaticMeshComponent* ISMC = SharedData ? SharedData->ISMC : nullptr;
			if (SharedData)
			{
				SharedData->RefCount += 1;
			}
			else
			{
				ISMC = NewObject<UInstancedStaticMeshComponent>(ActorOwner);
				ISMC->SetStaticMesh(MeshDesc.Mesh);
				for (int32 ElementIndex = 0; ElementIndex < MeshDesc.MaterialOverrides.Num(); ++ElementIndex)
				{
					if (UMaterialInterface* MaterialOverride = MeshDesc.MaterialOverrides[ElementIndex])
					{
						ISMC->SetMaterial(ElementIndex, MaterialOverride);
					}
				}
				ISMC->SetCullDistances(0, 1000000);
				// @todo: Need to figure out what to do here, either LOD or cull distances.
				ISMC->SetupAttachment(ActorOwner->GetRootComponent());
				ISMC->SetCanEverAffectNavigation(false);
				ISMC->CanCharacterStepUpOn = ECanBeCharacterBase::ECB_Yes;
				ISMC->SetCollisionProfileName(UCollisionProfile::CustomCollisionProfileName);
				ISMC->SetCollisionObjectType(ECC_WorldDynamic);
				ISMC->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
				ISMC->SetCollisionResponseToChannel(ECC_Camera, ECR_Block);
				ISMC->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Ignore);
				ISMC->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Ignore);
				ISMC->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
				ISMC->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Block);
				ISMC->SetCollisionResponseToChannel(ECC_Vehicle, ECR_Block);
				ISMC->SetCollisionResponseToChannel(ECC_Destructible, ECR_Block);
				ISMC->SetCastShadow(MeshDesc.bCastShadows);
				ISMC->Mobility = MeshDesc.Mobility;
				ISMC->SetReceivesDecals(false);
				ISMC->RegisterComponent();

				ISMCSharedData.Emplace(MeshDesc, FISMCSharedData(ISMC));
			}

			Info.InstancedStaticMeshComponents.Add(ISMC);
		}

		// Build the LOD significance ranges
		TArray<float> AllLODSignificances;
		auto UniqueInsertOrdered = [&AllLODSignificances](const float Significance)
		{
			int i = 0;
			for (; i < AllLODSignificances.Num(); ++i)
			{
				// I did not use epsilon check here on purpose, because it will make it hard later meshes inside.
				if (Significance == AllLODSignificances[i])
				{
					return;
				}
				if (AllLODSignificances[i] > Significance)
				{
					break;
				}
			}
			AllLODSignificances.Insert(Significance, i);
		};
		for (const FStaticMeshInstanceVisualizationMeshDesc& MeshDesc : Info.Desc.Meshes)
		{
			UniqueInsertOrdered(MeshDesc.MinLODSignificance);
			UniqueInsertOrdered(MeshDesc.MaxLODSignificance);
		}
		Info.LODSignificanceRanges.SetNum(AllLODSignificances.Num() - 1);
		for (int i = 0; i < Info.LODSignificanceRanges.Num(); ++i)
		{
			FMassLODSignificanceRange& Range = Info.LODSignificanceRanges[i];
			Range.MinSignificance = AllLODSignificances[i];
			Range.MaxSignificance = AllLODSignificances[i + 1];
			Range.ISMCSharedDataPtr = &ISMCSharedData;

			for (int j = 0; j < Info.Desc.Meshes.Num(); ++j)
			{
				const FStaticMeshInstanceVisualizationMeshDesc& MeshDesc = Info.Desc.Meshes[j];
				const bool bAddMeshInRange = (Range.MinSignificance >= MeshDesc.MinLODSignificance && Range.
					MinSignificance < MeshDesc.MaxLODSignificance);
				if (bAddMeshInRange)
				{
					Range.StaticMeshRefs.Add(MeshDesc);
				}
			}
		}
	}
}

void UExMassVisualizationComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (FMassInstancedStaticMeshInfo& InfoSuper : InstancedStaticMeshInfos)
	{
		auto& Info = *static_cast<FExMassInstancedStaticMeshInfo*>(&InfoSuper);
		for (auto ISMC : Info.InstancedStaticMeshComponents)
		{
			if (ISMC->GetNumRenderInstances() != ISMC->InstanceBodies.Num())
			{
				UE_LOG(LogTemp, Display, TEXT("\n\
            		is collision enabled: %d,\n \
            		ShouldCreatePhysicsState: %d,\n\
            		IsPhysicsStateCreated: %d\n\
            		GetNumRenderInstances: %d\n\
            		InstanceBodies.Num(): %d"
				       ),
				       ISMC->IsCollisionEnabled(),
				       ISMC->ShouldCreatePhysicsState(),
				       ISMC->IsPhysicsStateCreated(),
				       ISMC->GetNumRenderInstances(),
				       ISMC->InstanceBodies.Num()
				);
				ISMC->RecreatePhysicsState();
				UE_LOG(LogTemp, Display, TEXT("\n\
					is collision enabled: %d,\n \
					ShouldCreatePhysicsState: %d,\n\
					IsPhysicsStateCreated: %d\n\
					GetNumRenderInstances: %d\n\
					InstanceBodies.Num(): %d"
				       ),
				       ISMC->IsCollisionEnabled(),
				       ISMC->ShouldCreatePhysicsState(),
				       ISMC->IsPhysicsStateCreated(),
				       ISMC->GetNumRenderInstances(),
				       ISMC->InstanceBodies.Num()
				);
			}
			auto InstanceTransforms = ISMC->PerInstancePrevTransform;
			auto& InstanceBodies = ISMC->InstanceBodies;
			//sync InstanceBodies with InstanceTransforms
			for (int32 InstanceIndex = 0; InstanceIndex < ISMC->InstanceBodies.Num(); ++InstanceIndex)
			{
				auto&& Transform = FTransform(InstanceTransforms[InstanceIndex]);
				// InstanceBodies[InstanceIndex]->SetBodyTransform(Transform, ETeleportType::TeleportPhysics);
				auto& ActorHandle = InstanceBodies[InstanceIndex]->GetPhysicsActorHandle();
				FPhysicsCommand::ExecuteWrite(ActorHandle, [&](const FPhysicsActorHandle& Actor)
				{
					// const bool bKinematic = FPhysicsInterface::IsKinematic_AssumesLocked(Actor);
					// const bool bSimulated = FPhysicsInterface::CanSimulate_AssumesLocked(Actor);
					// const bool bIsSimKinematic = bKinematic && bSimulated;
					// if (bIsSimKinematic)
					// {
					// // }
					// FPhysicsInterface::SetKinematicTarget_AssumesLocked(Actor, Transform);
					FPhysicsInterface::SetGlobalPose_AssumesLocked(Actor, Transform);
				});
			}
		}
	}
}

void UExMassVisualizationComponent::EnableInstancesCollision(bool bCond)
{
	// for (auto Element : COLLECTION)
	// {
	// 	
	// }
}

void UExMassVisualizationComponent::ExBeginVisualChanges()
{
	TRACE_CPUPROFILER_EVENT_SCOPE_STR("MassVisualizationComponent BeginVisualChanges")

	EnableInstancesCollision(true);

	// Conditionally construct static mesh components
	if (bNeedStaticMeshComponentConstruction)
	{
		UE_LOG(LogTemp, Warning, TEXT("bNeedStaticMeshComponentConstruction"));
		ExConstructStaticMeshComponents(); //redirect to the extended function
		bNeedStaticMeshComponentConstruction = false;
	}

	// Reset instance transform scratch buffers
	for (auto It = ISMCSharedData.CreateIterator(); It; ++It)
	{
		FISMCSharedData& SharedData = It.Value();
		SharedData.UpdateInstanceIds.Reset();
		SharedData.StaticMeshInstanceCustomFloats.Reset();
		SharedData.StaticMeshInstanceTransforms.Reset();
		SharedData.StaticMeshInstancePrevTransforms.Reset();
		SharedData.WriteIterator = 0;
	}
}
