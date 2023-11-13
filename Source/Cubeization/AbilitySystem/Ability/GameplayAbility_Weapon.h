#pragma once
#include "Abilities/GameplayAbility.h"
#include "GameplayAbility_Weapon.generated.h"

UCLASS()
class UGameplayAbility_Weapon: public UGameplayAbility
{
	GENERATED_BODY()
public:

	//projectile class
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayAbility")
	TSubclassOf<class AAtomProjectile> ProjectileClass;

	//effect to apply on hit
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GameplayAbility")
	TSubclassOf<class UGameplayEffect> DamageEffect;

	//fx to play on hit 
	
	

	
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:

	// UFUNCTION()
	// void OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData);
	//
	// UFUNCTION()
	// void OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData);
	//
	// UFUNCTION()
	// void EventReceived(FGameplayTag EventTag, FGameplayEventData EventData);
	
};
