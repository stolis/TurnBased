// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Grenades/Grenade.h"
#include "G1190.generated.h"

/**
 * 
 */
UCLASS()
class TURNBASED_API AG1190 : public AGrenade
{
	GENERATED_BODY()
	
	
public:

	AG1190() {
		this->ModelName = "G1-190";
		this->ImpactRadius = 4;
	}
	
};
