// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "SPickupActor.h"
#include "SWeaponPickup.generated.h"

/**
 * 
 */
UCLASS(Abstract)
class WILDKILLER_API ASWeaponPickup : public ASPickupActor
{
	GENERATED_BODY()

	ASWeaponPickup(const FObjectInitializer& ObjectInitializer);

	/* Class to add to inventory when picked up */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ASWeapon> WeaponClass;
	
	virtual void OnUsed(APawn* InstigatorPawn) override;
};
