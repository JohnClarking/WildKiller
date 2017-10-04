// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "WildKiller.h"
#include "SGameMode.h"
#include "SPlayerController.h"
#include "SPlayerState.h"
#include "SGameState.h"
#include "SCharacter.h"
#include "SHUD.h"
#include "STypes.h"
#include "SSpectatorPawn.h"
#include "SZombieAIController.h"
#include "SZombieCharacter.h"
#include "SCharacter.h"
#include "SPlayerStart.h"


ASGameMode::ASGameMode(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	/* Assign the class types used by this gamemode */
	PlayerControllerClass = ASPlayerController::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();
	GameStateClass = ASGameState::StaticClass();
	SpectatorClass = ASSpectatorPawn::StaticClass();

	bAllowFriendlyFireDamage = false;
	bSpawnZombiesAtNight = true;

	/* Start the game at 16:00 */
	TimeOfDayStart = 16 * 60;
	BotSpawnInterval = 5.0f;

	/* Default team is 1 for players and 0 for enemies */
	PlayerTeamNum = 1;
}


void ASGameMode::InitGameState()
{
	Super::InitGameState();

	ASGameState* MyGameState = Cast<ASGameState>(GameState);
	if (MyGameState)
	{
		MyGameState->ElapsedGameMinutes = TimeOfDayStart;
	}
}


void ASGameMode::StartMatch()
{
	if (!HasMatchStarted())
	{
		/* Spawn a new bot every 5 seconds (bothandler will opt-out based on his own rules for example to only spawn during night time) */
		GetWorldTimerManager().SetTimer(TimerHandle_BotSpawns, this, &ASGameMode::SpawnBotHandler, BotSpawnInterval, true);
	}

	Super::StartMatch();
}


void ASGameMode::DefaultTimer()
{
	/* This function is called every 1 second. */
	Super::DefaultTimer();

	/* Immediately start the match while playing in editor */
	//if (GetWorld()->IsPlayInEditor())
	{
		if (GetMatchState() == MatchState::WaitingToStart)
		{
			StartMatch();
		}
	}

	/* Only increment time of day while game is active */
	if (IsMatchInProgress())
	{
		ASGameState* MyGameState = Cast<ASGameState>(GameState);
		if (MyGameState)
		{
			/* Increment our time of day */
			MyGameState->ElapsedGameMinutes += MyGameState->GetTimeOfDayIncrement();

			/* Determine our state */
			MyGameState->GetAndUpdateIsNight();

			/* Trigger events when night starts or ends */
			bool CurrentIsNight = MyGameState->GetIsNight();
			if (CurrentIsNight != LastIsNight)
			{
				FString MessageText = CurrentIsNight ? "SURVIVE!" : "You Survived! Now prepare for the coming night!";

				ASGameState* MyGameState = Cast<ASGameState>(GameState);
				if (MyGameState)
				{
					MyGameState->BroadcastGameMessage(MessageText);
				}

				/* The night just ended, respawn all dead players */
				if (!CurrentIsNight)
				{
					OnNightEnded();
				}

				/* Update bot states */
				if (CurrentIsNight)
				{
					WakeAllBots();
				}
				else
				{
					PassifyAllBots();
				}
			}

			LastIsNight = MyGameState->bIsNight;
		}
	}
}


bool ASGameMode::CanDealDamage(class ASPlayerState* DamageCauser, class ASPlayerState* DamagedPlayer) const
{
	if (bAllowFriendlyFireDamage)
		return true;

	/* Allow damage to self */
	if (DamagedPlayer == DamageCauser)
		return true;

	// Compare Team Numbers
	return DamageCauser && DamagedPlayer && (DamageCauser->GetTeamNumber() != DamagedPlayer->GetTeamNumber());
}


FString ASGameMode::InitNewPlayer(class APlayerController* NewPlayerController, const TSharedPtr<FUniqueNetId>& UniqueId, const FString& Options, const FString& Portal)
{
	FString Result = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	ASPlayerState* NewPlayerState = Cast<ASPlayerState>(NewPlayerController->PlayerState);
	if (NewPlayerState)
	{
		NewPlayerState->SetTeamNumber(PlayerTeamNum);
	}

	return Result;
}


float ASGameMode::ModifyDamage(float Damage, AActor* DamagedActor, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) const
{
	float ActualDamage = Damage;

	ASBaseCharacter* DamagedPawn = Cast<ASBaseCharacter>(DamagedActor);
	if (DamagedPawn && EventInstigator)
	{
		ASPlayerState* DamagedPlayerState = Cast<ASPlayerState>(DamagedPawn->PlayerState);
		ASPlayerState* InstigatorPlayerState = Cast<ASPlayerState>(EventInstigator->PlayerState);

		// Check for friendly fire
		if (!CanDealDamage(InstigatorPlayerState, DamagedPlayerState))
		{
			ActualDamage = 0.f;
		}
	}

	return ActualDamage;
}


bool ASGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	/* Always pick a random location */
	return false;
}


AActor* ASGameMode::ChoosePlayerStart(AController* Player)
{
	TArray<APlayerStart*> PreferredSpawns;
	TArray<APlayerStart*> FallbackSpawns;

	for (int32 i = 0; i < PlayerStarts.Num(); i++)
	{
		APlayerStart* TestStart = PlayerStarts[i];
		if (IsSpawnpointAllowed(TestStart, Player))
		{
			if (IsSpawnpointPreferred(TestStart, Player))
			{
				PreferredSpawns.Add(TestStart);
			}
			else
			{
				FallbackSpawns.Add(TestStart);
			}
		}
	}

	APlayerStart* BestStart = nullptr;
	if (PreferredSpawns.Num() > 0)
	{
		BestStart = PreferredSpawns[FMath::RandHelper(PreferredSpawns.Num())];
	}
	else if (FallbackSpawns.Num() > 0)
	{
		BestStart = FallbackSpawns[FMath::RandHelper(FallbackSpawns.Num())];
	}

	return BestStart ? BestStart : Super::ChoosePlayerStart(Player);
}


bool ASGameMode::IsSpawnpointAllowed(APlayerStart* SpawnPoint, AController* Controller)
{
	if (Controller == nullptr || Controller->PlayerState == nullptr)
		return true;

	/* Check for extended playerstart class */
	ASPlayerStart* MyPlayerStart = Cast<ASPlayerStart>(SpawnPoint);
	if (MyPlayerStart)
	{
		return MyPlayerStart->GetIsPlayerOnly() && !Controller->PlayerState->bIsABot;
	}

	/* Cast failed, Anyone can spawn at the base playerstart class */
	return true;
}


bool ASGameMode::IsSpawnpointPreferred(APlayerStart* SpawnPoint, AController* Controller)
{
	if (SpawnPoint)
	{
		/* Iterate all pawns to check for collision overlaps with the spawn point */
		const FVector SpawnLocation = SpawnPoint->GetActorLocation();
		for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
		{
			ACharacter* OtherPawn = Cast<ACharacter>(*It);
			if (OtherPawn)
			{
				const float CombinedHeight = (SpawnPoint->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * 2.0f;
				const float CombinedWidth = SpawnPoint->GetCapsuleComponent()->GetScaledCapsuleRadius() + OtherPawn->GetCapsuleComponent()->GetScaledCapsuleRadius();
				const FVector OtherLocation = OtherPawn->GetActorLocation();

				// Check if player overlaps the playerstart
				if (FMath::Abs(SpawnLocation.Z - OtherLocation.Z) < CombinedHeight && (SpawnLocation - OtherLocation).Size2D() < CombinedWidth)
				{
					return false;
				}
			}
		}

		/* Check if spawnpoint is exclusive to players */
		ASPlayerStart* MyPlayerStart = Cast<ASPlayerStart>(SpawnPoint);
		if (MyPlayerStart)
		{
			return MyPlayerStart->GetIsPlayerOnly() && !Controller->PlayerState->bIsABot;
		}
	}

	return false;
}


void ASGameMode::SpawnNewBot()
{
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.bNoCollisionFail = true;

	ASZombieAIController* AIC = GetWorld()->SpawnActor<ASZombieAIController>(SpawnInfo);
	RestartPlayer(AIC);
}

/* Used by RestartPlayer() to determine the pawn to create and possess when a bot or player spawns */
UClass* ASGameMode::GetDefaultPawnClassForController(AController* InController)
{
	if (Cast<ASZombieAIController>(InController))
	{
		return BotPawnClass;
	}

	return Super::GetDefaultPawnClassForController(InController);
}


bool ASGameMode::CanSpectate(APlayerController* Viewer, APlayerState* ViewTarget)
{
	/* Don't allow spectating of other non-player bots */
	return (ViewTarget && !ViewTarget->bIsABot);
}


void ASGameMode::PassifyAllBots()
{
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		ASZombieCharacter* AIPawn = Cast<ASZombieCharacter>(*It);
		if (AIPawn)
		{
			AIPawn->SetBotType(EBotBehaviorType::Passive);
		}
	}
}


void ASGameMode::WakeAllBots()
{
	for (FConstPawnIterator It = GetWorld()->GetPawnIterator(); It; It++)
	{
		ASZombieCharacter* AIPawn = Cast<ASZombieCharacter>(*It);
		if (AIPawn)
		{
			AIPawn->SetBotType(EBotBehaviorType::Patrolling);
		}
	}
}


void ASGameMode::SpawnBotHandler()
{
	if (!bSpawnZombiesAtNight)
		return;

	ASGameState* MyGameState = Cast<ASGameState>(GameState);
	if (MyGameState)
	{
		/* Only spawn bots during night time */
		if (MyGameState->GetIsNight())
		{
			/* This could be any dynamic number based on difficulty (eg. increasing after having survived a few nights) */
			const int32 MaxPawns = 10;

			/* Check number of available pawns (players included) */
			if (GetWorld()->GetNumPawns() < MaxPawns)
			{
				SpawnNewBot();
			}
		}
	}
}


void ASGameMode::OnNightEnded()
{
	// Do nothing (can be used to apply score or trigger other time of day events)
}

void ASGameMode::Killed(AController* Killer, AController* VictimPlayer, APawn* VictimPawn, const UDamageType* DamageType)
{
	// Do nothing (can we used to apply score or keep track of kill count)
}
