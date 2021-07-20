// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ToyRacingGameMode.h"
#include "RemoteControlCar.h"
#include "ToyRacingHud.h"
#include "MPTest.h"

AToyRacingGameMode::AToyRacingGameMode()
{
	DefaultPawnClass = ARemoteControlCar::StaticClass();
	HUDClass = AToyRacingHud::StaticClass();
}
