// Fill out your copyright notice in the Description page of Project Settings.


#include "Wheel.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"

// Sets default values
AWheel::AWheel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AWheel::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AWheel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

