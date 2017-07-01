// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/CharacterMovementComponent.h"
#include "SCharacterMovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class WILDKILLER_API USCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	virtual float GetMaxSpeed() const override;

};
