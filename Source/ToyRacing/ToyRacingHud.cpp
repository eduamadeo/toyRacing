// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ToyRacingHud.h"
#include "ToyRacingPawn.h"
#include "WheeledVehicle.h"
#include "RenderResource.h"
#include "ToyRacingGameMode.h"
#include "Shader.h"
#include "TimerManager.h"
#include "Engine/Canvas.h"
#include "WheeledVehicleMovementComponent.h"
#include "Engine/Font.h"
#include "CanvasItem.h"
#include "Sound/AmbientSound.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "RemoteControlCar.h"

#define LOCTEXT_NAMESPACE "VehicleHUD"

#ifndef HMD_MODULE_INCLUDED
#define HMD_MODULE_INCLUDED 0
#endif

AToyRacingHud::AToyRacingHud()
{
	static ConstructorHelpers::FObjectFinder<UFont> Font(TEXT("Font'/Game/Assets/lightsideracad_Font.lightsideracad_Font'"));
	HUDFont = Font.Object;

	static ConstructorHelpers::FObjectFinder<UTexture2D> PowerTexture(TEXT("Texture2D'/Game/Assets/textures/Hurricane2.Hurricane2'"));
	if (PowerTexture.Succeeded())
	{
		HurricaneTexture = PowerTexture.Object;
	}
	static ConstructorHelpers::FObjectFinder<UTexture2D> EletricUTexture(TEXT("Texture2D'/Game/Assets/textures/Laser2.Laser2'"));
	if (EletricUTexture.Succeeded())
	{
		EletricTexture = EletricUTexture.Object;
	}
	static ConstructorHelpers::FObjectFinder<UTexture2D> MagnetUTexture(TEXT("Texture2D'/Game/Assets/textures/magnet2.magnet2'"));
	if (MagnetUTexture.Succeeded())
	{
		MagnetTexture = MagnetUTexture.Object;
	}

	Counter = 4;
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
			FVector2D ScaleVec(HUDYRatio * 1.0f, HUDYRatio * 1.0f);

			if (Vehicle->IsRacing)
			{
				FText PositionDisplayString = FText::Format(LOCTEXT("PositionFormat", "Pos. {0}/{1}"), FText::AsNumber(Vehicle->Position), FText::AsNumber(Vehicle->NumPlayers));
				FCanvasTextItem PositionTextItem(FVector2D(HUDXRatio * 1050.f, HUDYRatio * 620.f), PositionDisplayString, HUDFont, FLinearColor::White);
				PositionTextItem.Scale = ScaleVec;
				Canvas->DrawItem(PositionTextItem);


				FText LapDisplayString = FText::Format(LOCTEXT("LapFormat", "Lap {0}/{1}"), FText::AsNumber(Vehicle->CurrentLap), FText::AsNumber(Vehicle->NumLaps));
				FCanvasTextItem LapTextItem(FVector2D(HUDXRatio * 50.f, HUDYRatio * 620.f), LapDisplayString, HUDFont, FLinearColor::White);
				LapTextItem.Scale = ScaleVec;
				Canvas->DrawItem(LapTextItem);
			}
			else if (Vehicle->IsWaiting)
			{
				FVector2D ScaleVecNum(HUDYRatio * 1.5f, HUDYRatio * 1.5f);

				FText ConterDisplayString = FText::Format(LOCTEXT("CounterFormat", "{0}"), FText::AsNumber(Counter));
				FCanvasTextItem ConterTextItem(FVector2D(HUDXRatio * 620.f, HUDYRatio * 360), ConterDisplayString, HUDFont, FLinearColor::White);
				ConterTextItem.Scale = ScaleVecNum;
				Canvas->DrawItem(ConterTextItem);
				if (Counter == 4)
				{
					Counter--;
					TArray<AActor*> Actors;
					UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAmbientSound::StaticClass(), Actors);

					for (AActor* Actor : Actors)
					{
						AAmbientSound* AmbientSound = Cast<AAmbientSound>(Actor);
						if (AmbientSound && AmbientSound->Tags.Num() > 0 && AmbientSound->Tags.Contains("Countdown"))
						{
							AmbientSound->Play();
						}
					}
					GetWorldTimerManager().SetTimer(CounterTimer, this, &AToyRacingHud::ChangeCounter, 1.0f, true);
				}
			}
			else if (Vehicle->HasEnded)
			{
				FText EndedDisplayString = FText::Format(LOCTEXT("EndedFormat", "You ended in the position {0} of {1}"), FText::AsNumber(Vehicle->Position), FText::AsNumber(Vehicle->NumPlayers));
				FCanvasTextItem EndedTextItem(FVector2D(HUDXRatio * 300.f, HUDYRatio * 360), EndedDisplayString, HUDFont, FLinearColor::White);
				EndedTextItem.Scale = ScaleVec;
				Canvas->DrawItem(EndedTextItem);
			}
			else
			{
				FText WaitDisplayString = FText::Format(LOCTEXT("WaitingFormat", "{0}"), FText::FromString("Waiting for other players"));
				FCanvasTextItem WaitTextItem(FVector2D(HUDXRatio * 400.f, HUDYRatio * 360), WaitDisplayString, HUDFont, FLinearColor::White);
				WaitTextItem.Scale = ScaleVec;
				Canvas->DrawItem(WaitTextItem);
			}

			if (Vehicle->CurrentPower == EPower::TRAP)
			{
				FCanvasIcon PowerIcon;
				PowerIcon.Texture = HurricaneTexture;
				Canvas->SetDrawColor(FColor(255.0f, 255.0f, 255.0f, 255.0f));
				Canvas->DrawIcon(PowerIcon, HUDXRatio * 1100.0f, HUDYRatio * 50.0f, 0.3f);
			}
			else if (Vehicle->CurrentPower == EPower::ELETRIC)
			{
				FCanvasIcon PowerIcon;
				PowerIcon.Texture = EletricTexture;
				Canvas->SetDrawColor(FColor(255.0f, 255.0f, 255.0f, 255.0f));
				Canvas->DrawIcon(PowerIcon, HUDXRatio * 1100.0f, HUDYRatio * 50.0f, 0.3f);
			}
			else if (Vehicle->CurrentPower == EPower::CLOCK)
			{
				FCanvasIcon PowerIcon;
				PowerIcon.Texture = MagnetTexture;
				Canvas->SetDrawColor(FColor(255.0f, 255.0f, 255.0f, 255.0f));
				Canvas->DrawIcon(PowerIcon, HUDXRatio * 1100.0f, HUDYRatio * 50.0f, 0.3f);
			}
		}
	}
}

void AToyRacingHud::ChangeCounter()
{
	Counter--;

	if (Counter == 0)
	{
		CounterTimer.Invalidate();

		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAmbientSound::StaticClass(), Actors);

		for (AActor* Actor : Actors)
		{
			AAmbientSound* AmbientSound = Cast<AAmbientSound>(Actor);
			if (AmbientSound && AmbientSound->Tags.Num() > 0 && AmbientSound->Tags.Contains("Music"))
			{
				AmbientSound->Play();
			}
		}
		ARemoteControlCar* Vehicle = Cast<ARemoteControlCar>(GetOwningPawn());
		if ((Vehicle != nullptr))
		{
			Vehicle->IsRacing = true;
			Vehicle->IsWaiting = false;
		}
	}
}


#undef LOCTEXT_NAMESPACE
