// Fill out your copyright notice in the Description page of Project Settings.

#include "ATriggerCheckpoint.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "Engine/EngineTypes.h"
#include "Engine/Engine.h"
#include "../RemoteControlCar.h"
#include "../ToyRacingGameMode.h"
#include "DrawDebugHelpers.h"

AATriggerCheckpoint::AATriggerCheckpoint()
{
    //Register Events
    OnActorBeginOverlap.AddDynamic(this, &AATriggerCheckpoint::OnOverlap);

    OrderNumber = 0;
}

// Called when the game starts or when spawned
void AATriggerCheckpoint::BeginPlay()
{
    Super::BeginPlay();

    //DrawDebugBox(GetWorld(), GetActorLocation(), GetComponentsBoundingBox().GetExtent(), FColor::Purple, true, -1, 0, 5);

}

void AATriggerCheckpoint::OnOverlap(AActor* OverlappedActor, AActor* OtherActor){
    ARemoteControlCar* RemoteControlCar = Cast<ARemoteControlCar>(OtherActor);

    if (RemoteControlCar) {
        // check if Actors do not equal nullptr and that 
        if (OtherActor && (OtherActor != this)) {
            if (OrderNumber == (RemoteControlCar->LastCheckpoint + 1))
            {
                RemoteControlCar->LastCheckpoint = OrderNumber;
            } else if (OrderNumber == 1 && RemoteControlCar->LastCheckpoint == 27) {
                RemoteControlCar->CurrentLap += 1;
                RemoteControlCar->LastCheckpoint = OrderNumber;
                ServerNotifyLapCompletion(RemoteControlCar->Position, RemoteControlCar->CurrentLap);
            }
        }
    }
}

void AATriggerCheckpoint::ServerNotifyLapCompletion_Implementation(int Position, int CurrentLap)
{
    AToyRacingGameMode* GameMode = Cast<AToyRacingGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (GameMode)
    {
        GameMode->NotifyLapCompletion(Position, CurrentLap);
    }
}
