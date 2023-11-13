#pragma once
#include "AttributeSet.h"
#include "GameplayEffect.h"
#include "AbilitySystem/CubeAbilitySystemComponent.h"
#include "CubeAttributeSet.generated.h"

/**
 * This macro defines a set of helper functions for accessing and initializing attributes.
 *
 * The following example of the macro:
 *		ATTRIBUTE_ACCESSORS(ULyraHealthSet, Health)
 * will create the following functions:
 *		static FGameplayAttribute GetHealthAttribute();
 *		float GetHealth() const;
 *		void SetHealth(float NewVal);
 *		void InitHealth(float NewVal);
 */
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)


// Delegate used to broadcast attribute events.
DECLARE_MULTICAST_DELEGATE_FourParams(FAttributeEvent, AActor* /*EffectInstigator*/, AActor* /*EffectCauser*/, const FGameplayEffectSpec& /*EffectSpec*/, float /*EffectMagnitude*/);


UCLASS()
class UCubeAttributeSet :  public UAttributeSet
{
	GENERATED_BODY()
public:
	virtual UWorld* GetWorld() const override;
	UCubeAbilitySystemComponent* GetCubeAbilitySystemComponent() const;
};
