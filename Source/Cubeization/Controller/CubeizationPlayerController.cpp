#include "CubeizationPlayerController.h"

class UEnhancedInputLocalPlayerSubsystem;

void ACubeizationPlayerController::BeginPlay()
{
	Super::BeginPlay();
	CameraViewInit();
	SetShowMouseCursor(true);

	
}



FVector ACubeizationPlayerController::GetPointingPosition()
{
	//cast ray from camera to world
	FHitResult HitResult;
	GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, HitResult);

	if(!HitResult.bBlockingHit) return PreviousPointingPosition;
	PreviousPointingPosition = HitResult.Location;

	return HitResult.Location;
}

void ACubeizationPlayerController::CameraViewInit()
{
	//look down 45 degree
	SetControlRotation(FRotator(-45, 45, 0));
}

void ACubeizationPlayerController::SetSuspended(bool bSuspend)
{
	if (bSuspend)
	{
		OnSuspend.Broadcast();
	}
	else
	{
		OnResume.Broadcast();
	}
	bSuspended = bSuspend;
}

