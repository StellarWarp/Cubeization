// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponRespond.h"


// Add default functionality here for any IWeaponRespond functions that are not pure virtual.
void IWeaponRespond::NotifyWeaponHit_Implementation(FWeaponHitInfo HitInfo)
{
	UE_LOG(LogTemp, Warning, TEXT("IWeaponRespond::NotifyWeaponHit"));
}
