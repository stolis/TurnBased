// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "Grenade.generated.h"

/**
 * 
 */
UCLASS()
class TURNBASED_API AGrenade : public AWeapon
{
	GENERATED_BODY()
	
public:
	AGrenade() {
		this->WeaponType = EWeaponTypeEnum::WTE_Grenade;
	}
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HandgunProperty", meta = (AllowPrivateAcces = "true"))
		int32 ImpactRadius;
	
};
