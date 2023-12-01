// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldDynamicTerrainControlSubsystem.h"

#include "TerrainDataSubsystem.h"
#include "SettingActor/DynamicTerrainSettingActor.h"
#include "UI/WidgetActor.h"
#include "Utils/BlueprintUtils.h"
#include "Utils/MathUtils.h"

void UWorldDynamicTerrainControlSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Collection.InitializeDependency(UTerrainDataSubsystem::StaticClass());
	Super::Initialize(Collection);

	TerrainDataSubsystem = GetWorld()->GetSubsystem<UTerrainDataSubsystem>();

	int32 XSize = TerrainDataSubsystem->DynamicFieldSize.X;
	int32 YSize = TerrainDataSubsystem->DynamicFieldSize.Y;
	PrevHeightField.Init(XSize, YSize);
	PrevLastHeightField.Init(XSize, YSize);
	PrevVelocityField.Init(XSize, YSize);

	SignedDistanceField.Init(XSize, YSize);
	TargetHeightField.Init(XSize, YSize);
	InterpolationFactorField.Init(XSize, YSize);
	InterpolationBeginEndField.Init(XSize, YSize);

	TerrainDataSubsystem->HeightField.UniformSet(DeactivationHeight);
	TerrainDataSubsystem->LastHeightField.UniformSet(DeactivationHeight);
	TerrainDataSubsystem->BaseHeightField.UniformSet(DeactivationHeight);


	// GetWorld()->SpawnActor(ADynamicTerrainSettingActor::StaticClass());
}

void UWorldDynamicTerrainControlSubsystem::ResetActivationArea()
{
	TerrainDataSubsystem->ActivationShiftSetUniform(0);
}

void UWorldDynamicTerrainControlSubsystem::DeactivateAll()
{
	TerrainDataSubsystem->ActivationShiftSetUniform(DeactivationHeight);
}

void UWorldDynamicTerrainControlSubsystem::PartialActiveArea(const FVector& Center, float HaftExtend, float FallOff)
{
	TerrainDataSubsystem->ActivationShiftSetUniform(-1000);
	TerrainDataSubsystem->CenterActiveField(Center, HaftExtend, FallOff, 0, -1000);
}


void UWorldDynamicTerrainControlSubsystem::LeaveMainWorld()
{
	//preserve the main world

	PrevHeightField = TerrainDataSubsystem->HeightField;
	PrevLastHeightField = TerrainDataSubsystem->LastHeightField;
	PrevVelocityField = TerrainDataSubsystem->VelocityField;
}

void UWorldDynamicTerrainControlSubsystem::RecoverMainWorld()
{
	bUIActive = false;
	TerrainDataSubsystem->LastHeightField = PrevLastHeightField;
	TerrainDataSubsystem->MapDataToField();

	auto& HeightField = TerrainDataSubsystem->HeightField;
	auto& VelocityField = TerrainDataSubsystem->VelocityField;

	int32 XSize = HeightField.GetSizeX();
	int32 YSize = HeightField.GetSizeY();
	for (int32 j = 0; j < XSize; ++j)
	{
		for (int32 i = 0; i < YSize; ++i)
		{
			FIntVector2 Index = TerrainDataSubsystem->LocalIndexToLogicalIndex(FIntVector2(i, j));


			// float BeginDTime = FMath::Lerp(SequentialActiveInterpolationTime, SequentialActiveLastingTime,
			//                                FMath::Clamp(SignedDistanceField[Index] / SequentialActiveMaxDistance, 0.0f,
			//                                             1.0f));
			// float EndDTime = SequentialActiveLastingTime; //end in the same time

			float BeginDTime = FMath::Lerp(0, SequentialActiveLastingTime - SequentialActiveInterpolationTime,
			                               FMath::Clamp(
				                               (SignedDistanceField[Index]) /
				                               SequentialActiveMaxDistance, 0.0f, 1.0f));
			float EndDTime = FMath::Lerp(SequentialActiveInterpolationTime, SequentialActiveLastingTime,
			                             FMath::Clamp(
				                             (SignedDistanceField[Index]) /
				                             SequentialActiveMaxDistance, 0.0f, 1.0f));
			FMathUtils::HermiteInterpolation::Params(
				HeightField[Index],
				VelocityField[Index],
				PrevHeightField[Index],
				PrevVelocityField[Index],
				EndDTime - BeginDTime,
				InterpolationFactorField[Index]);

			InterpolationBeginEndField[Index] = TTuple<float, float>(BeginDTime, EndDTime);
		}
	}

	// TerrainDataSubsystem->bExternalAnimationControl = true;
	bOnSequentialUpdate = true;
	SequentialActiveBeginTime = GetWorld()->GetTimeSeconds();
}

void UWorldDynamicTerrainControlSubsystem::SequentialFieldInit(
	const TArray<FVector>& Centers,
	const TArray<float>& HaftExtends,
	const float BeginTime,
	const float LastingTime)
{
	// TerrainDataSubsystem->bExternalAnimationControl = true;
	bOnSequentialUpdate = true;
	SequentialActiveBeginTime = BeginTime;
	SequentialActiveLastingTime = LastingTime;
	auto& HeightField = TerrainDataSubsystem->HeightField;
	// auto& BaseHeightField = TerrainDataSubsystem->BaseHeightField;
	// auto& LastHeightField = TerrainDataSubsystem->LastHeightField;
	//compute the distance field of quad defined by Centers and HaftExtends
	int32 XSize = HeightField.GetSizeX();
	int32 YSize = HeightField.GetSizeY();

	//not a standard distance field function
	auto QuadDistance = [](const FVector& Center, float HaftExtend, const FVector& Position)
	{
		FVector2D p = FVector2D(Position.X - Center.X, Position.Y - Center.Y);
		float hx = FMath::Max(FMath::Abs(p.X), FMath::Abs(p.Y));
		return hx - HaftExtend;
	};


	for (int32 j = 0; j < XSize; ++j)
	{
		for (int32 i = 0; i < YSize; ++i)
		{
			FIntVector2 Index = TerrainDataSubsystem->LocalIndexToLogicalIndex(FIntVector2(i, j));
			auto WorldPos = TerrainDataSubsystem->LogicalIndexToWorldPosition(Index);
			float MinDistance = FLT_MAX;
			for (int32 k = 0; k < Centers.Num(); ++k)
			{
				float Distance = QuadDistance(Centers[k], HaftExtends[k], WorldPos);
				if (Distance < MinDistance)
				{
					MinDistance = Distance;
				}
			}
			SignedDistanceField[Index] = MinDistance;
			if (MinDistance > 0)
			{
				TargetHeightField[Index] = DeactivationHeight;
			}
			else
			{
				TargetHeightField[Index] = ActivationHeight;
			}
			// BaseHeightField[Index] = TargetHeightField[Index];
			// LastHeightField[Index] = TargetHeightField[Index];

			auto& VelocityField = TerrainDataSubsystem->VelocityField;

			// float BeginDTime = FMath::Lerp(0, SequentialActiveLastingTime - SequentialActiveInterpolationTime,
			//                                FMath::Clamp(
			// 	                               (SequentialActiveMaxDistance - SignedDistanceField[Index]) /
			// 	                               SequentialActiveMaxDistance, 0.0f, 1.0f)
			// );
			// float EndDTime = FMath::Lerp(SequentialActiveInterpolationTime, SequentialActiveLastingTime,
			//                              FMath::Clamp(
			// 	                             (SequentialActiveMaxDistance - SignedDistanceField[Index]) /
			// 	                             SequentialActiveMaxDistance, 0.0f, 1.0f)
			// );
			
			float BeginDTime = FMath::Lerp(0, SequentialActiveLastingTime - SequentialActiveInterpolationTime,
							   FMath::Clamp(
								   (SignedDistanceField[Index]) /
								   SequentialActiveMaxDistance, 0.0f, 1.0f));
			float EndDTime = FMath::Lerp(SequentialActiveInterpolationTime, SequentialActiveLastingTime,
										 FMath::Clamp(
											 (SignedDistanceField[Index]) /
											 SequentialActiveMaxDistance, 0.0f, 1.0f));
			FMathUtils::HermiteInterpolation::Params(
				HeightField[Index],
				VelocityField[Index],
				TargetHeightField[Index],
				0,
				EndDTime - BeginDTime,
				InterpolationFactorField[Index]);

			InterpolationBeginEndField[Index] = TTuple<float, float>(BeginDTime, EndDTime);
		}
	}
}


void UWorldDynamicTerrainControlSubsystem::SequentialFieldUpdate(const float Time)
{
	const float TimeDelta = Time - SequentialActiveBeginTime;
	if (TimeDelta > SequentialActiveLastingTime + SequentialActiveExternalWaitTime)
	{
		bOnSequentialUpdate = false;
		// SetFieldActivation(true);
		// TerrainDataSubsystem->bExternalAnimationControl = false;
		OnPageFinishInit();
		return;
	}
	auto& HeightField = TerrainDataSubsystem->BaseHeightField;
	int32 XSize = HeightField.GetSizeX();
	int32 YSize = HeightField.GetSizeY();
	for (int32 j = 0; j < XSize; ++j)
	{
		for (int32 i = 0; i < YSize; ++i)
		{
			FIntVector2 Index = FIntVector2(i, j);
			// const float Distance = SignedDistanceField[Index];
			const auto& [InterpolationBegin, InterpolationEnd] = InterpolationBeginEndField[Index];
			// float t = FMath::Clamp(TimeDistanceTo01MappingFunction(TimeDelta, Distance), 0.0f, 1.0f);
			float t = FMath::Clamp(TimeDelta - InterpolationBegin, 0.0f, InterpolationEnd - InterpolationBegin);
			FMathUtils::HermiteInterpolation::MotionVectors(
				InterpolationFactorField[Index],
				t,
				HeightField[Index]);
		}
	}
}

// void UWorldDynamicTerrainControlSubsystem::SequentialFieldUpdateDefault(
// 	const float Time, const float InterpolationTime, const float MaxDistance)
// {
// 	SequentialFieldUpdate(Time, [=](float TimeDelta, float Distance)
// 	{
// 		//time delay is relevant to distance
// 		const float t = (TimeDelta - TimeDelay) * Inv_InterpolationTime;
// 		return t;
// 	});
// }


void UWorldDynamicTerrainControlSubsystem::OnHoverFieldReaction(const FVector& Position)
{
	TerrainDataSubsystem->ApplyForce(Position, -10000, 3);
	TerrainDataSubsystem->AddPointForce(Position, 15000);
}

void UWorldDynamicTerrainControlSubsystem::OnClickFieldReaction(const FVector& Position)
{
	TerrainDataSubsystem->AddPointForce(Position, -15000);
}

void UWorldDynamicTerrainControlSubsystem::InitCurrentPage()
{
	bUIActive = true;
	//field value init
	UICenter = GetWorld()->GetFirstPlayerController()->GetPawn()->GetActorLocation();
	const auto LogicalPosition = TerrainDataSubsystem->WorldPositionToLogicalIndex(UICenter);
	UICenter = TerrainDataSubsystem->LogicalIndexToWorldPosition(LogicalPosition);
	SetFieldActivation(false);

	TArray<FVector> Centers;
	TArray<float> HaftExtends;

	if(PageStack.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("PageStack is empty!"));
		return;
	}
	const auto& CurrentPage = PageStack.Top();
	//object creation
	for (auto& [
		Position,
		Scale,
		WidgetActorClass,
		WidgetAction,
		PageJump] : CurrentPage->UIElements)
	{
		FVector ActorLocation = UICenter + Position;
		ActorLocation.Z += 100;
		FIntVector2 Index = TerrainDataSubsystem->WorldPositionToLogicalIndex(ActorLocation);
		AWidgetActor* WidgetActor = Cast<AWidgetActor>(
			GetWorld()->SpawnActor(WidgetActorClass, &ActorLocation, &FRotator::ZeroRotator));
		UIElements.Add(FUIElement{
				Index,
				WidgetActor
			}
		);
		WidgetActor->BindAction(WidgetAction, PageJump);

		TerrainDataSubsystem->ActivationShiftField[Index] = 0;

		Centers.Add(ActorLocation);
		HaftExtends.Add(TerrainDataSubsystem->CubeLength * Scale * 0.5);
	}


	SequentialFieldInit(Centers, HaftExtends,
	                    GetWorld()->GetTimeSeconds(),
	                    SequentialActiveLastingTime);
}

void UWorldDynamicTerrainControlSubsystem::OnPageFinishInit_Implementation()
{
	if (!bUIActive)
		SetFieldActivation(true);
}

void UWorldDynamicTerrainControlSubsystem::ReleaseCurrentPage()
{
	for (auto& Element : UIElements)
	{
		Element.WidgetActor->NotifyStartDestroy();
	}
	UIElements.Empty();
}

void UWorldDynamicTerrainControlSubsystem::UIActorSyncTick()
{
	for (auto Element : UIElements)
	{
		FVector WorldLocation = TerrainDataSubsystem->LogicalIndexToWorldPosition(Element.Index);
		WorldLocation.Z += 100;
		Element.WidgetActor->SetActorLocation(WorldLocation);
	}
}

void UWorldDynamicTerrainControlSubsystem::AddPage(UPageConfig* PageConfig)
{
	PageStack.Push(PageConfig);
}

void UWorldDynamicTerrainControlSubsystem::ToNextPage(UPageConfig* PageConfig)
{
	ReleaseCurrentPage();
	PageStack.Push(PageConfig);
	InitCurrentPage();
}

void UWorldDynamicTerrainControlSubsystem::ToPrevPage()
{
	ReleaseCurrentPage();
	PageStack.Pop();
	InitCurrentPage();	
}

void UWorldDynamicTerrainControlSubsystem::OnClickStart()
{
	ReleaseCurrentPage();
	UBlueprintUtils::SetGameSuspend(false);
	//terrain init
	TerrainDataSubsystem->MapDataToField();
	PrevHeightField = TerrainDataSubsystem->BaseHeightField;
	RecoverMainWorld();
}

void UWorldDynamicTerrainControlSubsystem::OnClickAbout()
{
	ReleaseCurrentPage();
}

void UWorldDynamicTerrainControlSubsystem::OnClickPause()
{
	bool bSuspend;
	UBlueprintUtils::IsGameSuspend(bSuspend);
	if(bSuspend) return;
	UBlueprintUtils::SetGameSuspend(true);
	//terrain save
	LeaveMainWorld();
	//load the pause page
	ToNextPage(PausePage);
}

void UWorldDynamicTerrainControlSubsystem::OnClickContinue()
{
	UBlueprintUtils::SetGameSuspend(false);
	ReleaseCurrentPage();
	RecoverMainWorld();
}


void UWorldDynamicTerrainControlSubsystem::OnClickReturn()
{
	ToPrevPage();
}

void UWorldDynamicTerrainControlSubsystem::OnClickExit()
{
	//exit the game
	GetWorld()->GetFirstPlayerController()->ConsoleCommand("quit");
}


void UWorldDynamicTerrainControlSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UWorldDynamicTerrainControlSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bOnSequentialUpdate)
		SequentialFieldUpdate(GetWorld()->GetTimeSeconds());

	UIActorSyncTick();
}
