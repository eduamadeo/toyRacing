// Fill out your copyright notice in the Description page of Project Settings.


#include "MPTest.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/InputComponent.h"
#include "Engine/Engine.h"

// Sets default values
AMPTest::AMPTest()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bReplicateMovement = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/StarterContent/Shapes/Shape_NarrowCapsule.Shape_NarrowCapsule"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetupAttachment(RootComponent);

	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMesh->SetRelativeScale3D(FVector(1.5f, 1.0f, 0.4f));
	}

}

// Called when the game starts or when spawned
void AMPTest::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AMPTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMPTest::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMPTest::MoveForward);

}

void AMPTest::MoveForward(float Val)
{
	if (Val == 1)
	{
		RootComponent->AddWorldOffset(StaticMesh->GetForwardVector() * 100);
	}
}

