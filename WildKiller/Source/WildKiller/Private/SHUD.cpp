// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "WildKiller.h"
#include "SHUD.h"
#include "SCharacter.h"
#include "SUsableActor.h"
#include "SPlayerController.h"


ASHUD::ASHUD(const class FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> HUDCenterDotObj(TEXT("/Game/UI/HUD/T_CenterDot_M.T_CenterDot_M"));

	CenterDotIcon = UCanvas::MakeIcon(HUDCenterDotObj.Object);
}


void ASHUD::DrawHUD()
{
	Super::DrawHUD();

	DrawCenterDot();
}


void ASHUD::DrawCenterDot()
{
	float CenterX = Canvas->ClipX / 2;
	float CenterY = Canvas->ClipY / 2;

	float CenterDotScale = 0.07f;

	ASPlayerController* PCOwner = Cast<ASPlayerController>(PlayerOwner);
	if (PCOwner)
	{
		ASCharacter* Pawn = Cast<ASCharacter>(PCOwner->GetPawn());
		if (Pawn /*&& Pawn->IsAlive()*/) // Disabled this check until Death & Respawn is implemented.
		{
			// Boost size when hovering over a switchable object.
			ASUsableActor* usable = Pawn->GetUsableInView();
			if (usable)
				CenterDotScale *= 1.5f;

			Canvas->SetDrawColor(255, 255, 255, 255);
			Canvas->DrawIcon(CenterDotIcon,
				CenterX - CenterDotIcon.UL*CenterDotScale / 2.0f,
				CenterY - CenterDotIcon.VL*CenterDotScale / 2.0f, CenterDotScale);
		}
	}
}
