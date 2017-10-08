// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "WildKiller.h"
#include "SCharacterMovementComponent.h"
#include "SCharacter.h"



float USCharacterMovementComponent::GetMaxSpeed() const
{
	float MaxSpeed = Super::GetMaxSpeed();

	const ASCharacter* CharOwner = Cast<ASCharacter>(PawnOwner);
	if (CharOwner)
	{
		// Slow down during targeting, but don't further reduce movement speed while also crouching
		if (CharOwner->IsTargeting() && !IsCrouching())
		{
			MaxSpeed *= CharOwner->GetTargetingSpeedModifier();
		}
		else if (CharOwner->IsSprinting())
		{
			MaxSpeed *= CharOwner->GetSprintingSpeedModifier();
		}
	}

	return MaxSpeed;
}
