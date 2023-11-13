#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CubeizationPlayerController.generated.h"

UCLASS()
class ACubeizationPlayerController : public APlayerController
{
	GENERATED_BODY()

	FVector PreviousPointingPosition;

protected:
	virtual void BeginPlay() override;

	// virtual void Tick(float DeltaSeconds) override;
public:
	
	FVector GetPointingPosition();

	void CameraViewInit();
	
};
