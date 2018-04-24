// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Handguns/Laserguns/Lasergun.h"
#include "LZ343.generated.h"

/**
 * 
 */
UCLASS()
class TURNBASED_API ALZ343 : public ALasergun
{
	GENERATED_BODY()
	
public:
	ALZ343() {
		this->ModelName = "Lz-343";
		this->AmmoCapacity = 10;
		this->AmmoRemainingOnClip = 10;
		this->FireRate = 3;
		this->Accuracy = 6;
		this->FireRange = 1000;
	}
};
