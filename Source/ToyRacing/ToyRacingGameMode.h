// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/GameModeBase.h"
#include "ToyRacingGameMode.generated.h"

UCLASS(minimalapi)
class AToyRacingGameMode : public AGameModeBase
{
	GENERATED_BODY()

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

public:
	AToyRacingGameMode();

	UFUNCTION()
	void NotifyLapCompletion(int Position, int LapNum);

	int NumPlayers;
	int MaxLaps;
};



