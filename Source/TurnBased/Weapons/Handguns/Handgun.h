// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "Handgun.generated.h"

/**
 * 
 */
UCLASS()
class TURNBASED_API AHandgun : public AWeapon
{
	GENERATED_BODY()
	
public:
	AHandgun() {
		this->WeaponType = EWeaponTypeEnum::WTE_Handgun;
	};

	UFUNCTION(BluePrintCallable, Category = "HandgunFunction")
		virtual int32 GetAmmoRemainingOnClip() {
		return AmmoRemainingOnClip;
	}

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HandgunProperty", meta = (AllowPrivateAcces = "true"))
		int32 AmmoCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HandgunProperty", meta = (AllowPrivateAcces = "true"))
		int32 AmmoRemainingOnClip;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HandgunProperty", meta = (AllowPrivateAcces = "true"))
		int32 FireRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HandgunProperty", meta = (AllowPrivateAcces = "true"))
		int32 Accuracy;
};
