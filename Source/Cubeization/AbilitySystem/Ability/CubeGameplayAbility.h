#pragma once
#include "Abilities/GameplayAbility.h"
#include "CubeGameplayAbility.generated.h"

UCLASS()
class UCubeGameplayAbility: public UGameplayAbility
{
	GENERATED_BODY()
public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};
