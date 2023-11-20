#include "GameplayAbility_Weapon.h"

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
	}

	//direct raycast and spawn effect and damage

	auto Actor = ActorInfo->AvatarActor.Get();
	auto World = Actor->GetWorld();

	FHitResult HitResult;
	World->LineTraceSingleByChannel(HitResult,
	                                Actor->GetActorLocation(),
	                                Actor->GetActorLocation() + Actor->GetActorForwardVector() * 10000,
	                                ECollisionChannel::ECC_Visibility,
	                                FCollisionQueryParams::DefaultQueryParam,
	                                FCollisionResponseParams::DefaultResponseParam);


	//get the hit actor
	AActor* HitActor = HitResult.GetActor();

	//effect

	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
	//                                  FString::Printf(
	// 	                                 TEXT("Hit Actor: %s"), HitActor ? *HitActor->GetName() : TEXT("None")));
	//
	//
	// DrawDebugLine(World,
	//               HitResult.TraceStart,
	//               HitResult.Location, FColor::Red, false, 5.f, 0, 5.f);

	//logic process
	IWeaponRespond* WeaponRespond = Cast<IWeaponRespond>(HitActor);
	if (!HitActor || !WeaponRespond)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	WeaponRespond->NotifyWeaponHit_Implementation({
		HitResult.Location,
		HitResult.ImpactNormal,
		HitResult.ImpactNormal, //impose,
		HitResult,
	});


	// World->SpawnActor<AActor>(DestructionFieldClass, HitResult.Location, FRotator::ZeroRotator);

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


	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
