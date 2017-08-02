// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "WildKiller.h"
#include "SOpenWorldGameMode.h"





ASOpenWorldGameMode::ASOpenWorldGameMode(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	/* Open world content is free-for-all */
	bAllowFriendlyFireDamage = true;
}


bool ASOpenWorldGameMode::CanSpectate(APlayerController* Viewer, APlayerState* ViewTarget)
{
	return false;
}
