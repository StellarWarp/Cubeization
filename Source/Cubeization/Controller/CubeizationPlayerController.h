#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CubeizationPlayerController.generated.h"

//declare a suspend event
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSuspend); // the suspend is a fake pause
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnResume);

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

private:
	bool bSuspended;

public:
	UFUNCTION(BlueprintCallable)
	bool IsSuspended() const { return bSuspended; }

	UFUNCTION(BlueprintCallable)
	void SetSuspended(bool bSuspend);

	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
	FOnSuspend OnSuspend;
	UPROPERTY(VisibleAnywhere, BlueprintAssignable)
	FOnResume OnResume;
};
