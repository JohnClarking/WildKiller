// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class WildKillerServerTarget : TargetRules
{
	public WildKillerServerTarget(TargetInfo Target) : base(Target)
    {
		Type = TargetType.Server;
        ExtraModuleNames.AddRange(new string[] { "WildKiller" });
    }
}
