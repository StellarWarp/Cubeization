#pragma once
#include "WidgetActor.h"
#include "Engine/DataAsset.h"
#include "UPageConfig.generated.h"


class UWidgetAction;

USTRUCT()
struct FPageElement
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FVector Position;
	UPROPERTY(EditAnywhere)
	float Scale = 1;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWidgetActor> WidgetActorClass;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UWidgetAction> WidgetAction;
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UPageConfig> PageJump;
};


UCLASS()
class UPageConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FPageElement> UIElements;
};
