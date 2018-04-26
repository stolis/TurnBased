// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WeaponModule.generated.h"

UENUM(BlueprintType)		//"BlueprintType" is essential to include
enum class EWeaponModuleTypeEnum : uint8
{
	WMTE_Scope 			UMETA(DisplayName = "Scope"),
	WMTE_ClipLoader 	UMETA(DisplayName = "Clip Loader"),
	WMTE_Receiver		UMETA(DisplayName = "Receiver"),
	WMTE_Stock			UMETA(DisplayName = "Stock")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TURNBASED_API UWeaponModule : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponModule();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
	
};
