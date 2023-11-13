#include "CubeAttributeSet.h"

#include "AbilitySystem/CubeAbilitySystemComponent.h"

UWorld* UCubeAttributeSet::GetWorld() const
{
	const UObject* Outer = GetOuter();
	check(Outer);

	return Outer->GetWorld();
}

UCubeAbilitySystemComponent* UCubeAttributeSet::GetCubeAbilitySystemComponent() const
{
	return Cast<UCubeAbilitySystemComponent>(GetOwningAbilitySystemComponent());
}
