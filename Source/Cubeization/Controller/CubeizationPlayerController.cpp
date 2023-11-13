#include "CubeizationPlayerController.h"

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

