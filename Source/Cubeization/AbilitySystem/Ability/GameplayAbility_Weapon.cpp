#include "GameplayAbility_Weapon.h"

#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Weapons/AtomProjectile.h"
#include "Weapons/WeaponRespond.h"

void UGameplayAbility_Weapon::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                              const FGameplayAbilityActorInfo* ActorInfo,
                                              const FGameplayAbilityActivationInfo ActivationInfo,
                                              const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	//log LastFireTime and time

	//direct raycast and spawn effect and damage

	auto Actor = ActorInfo->AvatarActor.Get();
	auto World = Actor->GetWorld();

	FVector StartLocation = Actor->GetActorLocation();
	FVector Direction = Actor->GetActorForwardVector();

	FHitResult HitResult;
	World->LineTraceSingleByChannel(HitResult,
	                                StartLocation,
	                                StartLocation + Direction * 10000,
	                                ECollisionChannel::ECC_Visibility,
	                                FCollisionQueryParams::DefaultQueryParam,
	                                FCollisionResponseParams::DefaultResponseParam);


	//get the hit actor
	AActor* HitActor = HitResult.GetActor();

	//niagara effect
	if(HitNiagaraEffect && TrailNiagaraEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, HitNiagaraEffect, HitResult.Location);
		auto Trail = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, TrailNiagaraEffect,
																	StartLocation,
																	Actor->GetActorRotation());
		Trail->SetFloatParameter(L"Length", HitResult.Distance);
	}


	//logic process
	IWeaponRespond* WeaponRespond = Cast<IWeaponRespond>(HitActor);
	if (!HitActor || !WeaponRespond)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	//call cpp only
	WeaponRespond->NotifyWeaponHit_Implementation({
		HitResult.Location,
		HitResult.ImpactNormal,
		Direction * ImpulseMultiplier,
		HitResult,
	});


	World->SpawnActor<AActor>(DestructionFieldClass, HitResult.Location, FRotator::ZeroRotator);

	// //generate projectile
	// FTransform SpawnTransform;
	// SpawnTransform.SetLocation(HitResult.ImpactPoint);
	// SpawnTransform.SetRotation(HitResult.ImpactNormal.Rotation().Quaternion());
	// SpawnTransform.SetScale3D(FVector(1.0f));
	//
	// FActorSpawnParameters SpawnParameters;
	// SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// SpawnParameters.Owner = Actor;
	// SpawnParameters.Instigator = Cast<APawn>(Actor);

	// AActor* SpawnedActor = World->SpawnActor<AActor>(AAtomProjectile::StaticClass(), SpawnTransform, SpawnParameters);
	// //set the velocity
	// if(SpawnedActor)
	// {
	// 	if(auto Projectile = Cast<AAtomProjectile>(SpawnedActor))
	// 	{
	// 		Projectile->SetVelocity(HitResult.ImpactNormal);
	// 	}
	// }
	//get the effect
	// UParticleSystem* Effect = Cast<UParticleSystem>(EffectClass.LoadSynchronous());


	// //spawn the effect
	// UGameplayStatics::SpawnEmitterAtLocation(ActorInfo->AvatarActor->GetWorld(), Effect, HitResult.ImpactPoint);



	// Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	//delay end ability
	FTimerHandle TimerHandle;
	World->GetTimerManager().SetTimer(TimerHandle, [this]()
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}, FireInterval, false);
	
}
