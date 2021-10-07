// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.
#pragma once
#include "GameFramework/HUD.h"
#include "ToyRacingHud.generated.h"


UCLASS(config = Game)
class AToyRacingHud : public AHUD
{
	GENERATED_BODY()

public:
	AToyRacingHud();

	FTimerHandle CounterTimer;
	int Counter;

	/** Font used to render the vehicle info */
	UPROPERTY()
	UFont* HUDFont;

	UTexture* HurricaneTexture;
	UTexture* EletricTexture;
	UTexture* MagnetTexture;

	// Begin AHUD interface
	virtual void DrawHUD() override;
	// End AHUD interface

	void ChangeCounter();
};
