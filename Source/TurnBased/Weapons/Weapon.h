// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponTypeEnum : uint8
{
	WTE_Handgun	UMETA(DisplayName="Hand Gun"),
	WTE_Rifle UMETA(DisplayName="Rifle"),
	WTE_Grenade UMETA(DisplayName="Grenade")
};

UCLASS()
class TURNBASED_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperty", meta = (AllowPrivateAcces = "true"))
		FString Name;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponProperty", meta = (AllowPrivateAcces = "true"))
		FString ModelName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WeaponProperty", meta = (AllowPrivateAcces = "true"))
		EWeaponTypeEnum WeaponType;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponProperty", meta = (AllowPrivateAcces = "true"))
		float FireRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "WeaponProperty", meta = (AllowPrivateAcces = "true"))
		float Damage;
	


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
