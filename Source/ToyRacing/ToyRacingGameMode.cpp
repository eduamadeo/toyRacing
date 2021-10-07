// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ToyRacingGameMode.h"
#include "RemoteControlCar.h"
#include "ToyRacingGameInstance.h"
#include "ToyRacingHud.h"
#include "MPTest.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerStart.h"

AToyRacingGameMode::AToyRacingGameMode()
{
	DefaultPawnClass = ARemoteControlCar::StaticClass();
	HUDClass = AToyRacingHud::StaticClass();

	NumPlayers = 0;
	MaxLaps = 3;
}

void AToyRacingGameMode::NotifyLapCompletion(int Position, int LapNum)
{
	if (LapNum > MaxLaps)
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARemoteControlCar::StaticClass(), Actors);

		for (AActor* Actor : Actors)
		{
			ARemoteControlCar* RemoteCar = Cast<ARemoteControlCar>(Actor);
			if (RemoteCar && RemoteCar->Position == Position)
			{
				RemoteCar->IsRacing = false;
				RemoteCar->HasEnded = true;
				if (RemoteCar->IsLocallyControlled() && RemoteCar->HasAuthority())
				{
					RemoteCar->ShowFinalMenu();
				}
			}
		}
	}
}

AActor* AToyRacingGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);

	for (AActor* Actor : Actors)
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(Actor);
		if (PlayerStart->PlayerStartTag != TEXT("Taken"))
		{
			PlayerStart->PlayerStartTag = "Taken";
			return Actor;
		}
	}
	return nullptr;
}

void AToyRacingGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int MaxPlayers = 4;
	UToyRacingGameInstance* GameInstance = Cast<UToyRacingGameInstance>(UGameplayStatics::GetGameInstance(GetWorld()));
	if (GameInstance)
	{
		MaxPlayers = GameInstance->NumPlayers;
	}

	if (NumPlayers < (MaxPlayers - 1))
	{
		NumPlayers++;
	}
	else
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARemoteControlCar::StaticClass(), Actors);

		int count = 1;
		for (AActor* Actor : Actors)
		{
			ARemoteControlCar* RemoteCar = Cast<ARemoteControlCar>(Actor);
			RemoteCar->IsWaiting = true;
			RemoteCar->SpawnOrder = count;
			RemoteCar->NumPlayers = MaxPlayers;
			RemoteCar->NumLaps = MaxLaps;
			count++;
		}
	}
}
