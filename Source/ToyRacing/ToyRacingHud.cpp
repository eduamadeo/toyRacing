// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ToyRacingHud.h"
#include "ToyRacingPawn.h"
#include "WheeledVehicle.h"
#include "RenderResource.h"
#include "Shader.h"
#include "Engine/Canvas.h"
#include "WheeledVehicleMovementComponent.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "RemoteControlCar.h"

#define LOCTEXT_NAMESPACE "VehicleHUD"

#ifndef HMD_MODULE_INCLUDED
#define HMD_MODULE_INCLUDED 0
#endif

AToyRacingHud::AToyRacingHud()
{
	static ConstructorHelpers::FObjectFinder<UFont> Font(TEXT("/Engine/EngineFonts/RobotoDistanceField"));
	HUDFont = Font.Object;
}

void AToyRacingHud::DrawHUD()
{
	Super::DrawHUD();

	// Calculate ratio from 720p
	const float HUDXRatio = Canvas->SizeX / 1280.f;
	const float HUDYRatio = Canvas->SizeY / 720.f;

	bool bWantHUD = true;
#if HMD_MODULE_INCLUDED
	bWantHUD = !GEngine->IsStereoscopic3D();
#endif // HMD_MODULE_INCLUDED
	// We dont want the onscreen hud when using a HMD device	
	if (bWantHUD == true)
	{
		// Get our vehicle so we can check if we are in car. If we are we don't want onscreen HUD
		ARemoteControlCar* Vehicle = Cast<ARemoteControlCar>(GetOwningPawn());
		if ((Vehicle != nullptr))
		{
			FVector2D ScaleVec(HUDYRatio * 1.4f, HUDYRatio * 1.4f);

			FText SpeedDisplayString = FText::Format(LOCTEXT("SpeedFormat", "{0}/3"), FText::AsNumber(Vehicle->Position));

			// Speed
			FCanvasTextItem SpeedTextItem(FVector2D(HUDXRatio * 805.f, HUDYRatio * 455), SpeedDisplayString, HUDFont, FLinearColor::White);
			SpeedTextItem.Scale = ScaleVec;
			Canvas->DrawItem(SpeedTextItem);
		}
	}
}


#undef LOCTEXT_NAMESPACE
