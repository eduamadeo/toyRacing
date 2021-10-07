// Fill out your copyright notice in the Description page of Project Settings.


#include "Trap.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "ATriggerCheckpoint.h"
#include "../RemoteControlCar.h"

// Sets default values
ATrap::ATrap()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("RootComponent"));
	CapsuleComponent->InitCapsuleSize(20.0f, 60.0f);
	CapsuleComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	CapsuleComponent->SetSimulatePhysics(false);
	RootComponent = CapsuleComponent;

	if (GetLocalRole() == ROLE_Authority)
	{
		CapsuleComponent->OnComponentBeginOverlap.AddDynamic(this, &ATrap::OnProjectileOverlap);
	}

	TrapEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particles"));
	TrapEffect->SetRelativeScale3D(FVector(0.2f));
	TrapEffect->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultTrapEffect(TEXT("ParticleSystem'/Game/AdvancedMagicFX13/Particles/P_ky_thunderStorm.P_ky_thunderStorm'"));
	if (DefaultTrapEffect.Succeeded())
	{
		TrapEffect->SetTemplate(DefaultTrapEffect.Object);
		TrapEffect->SetRelativeLocation(FVector(0.0f, 0.0f, -50.0f));
	}

	// Setup the audio component and allocate it a sound cue
	static ConstructorHelpers::FObjectFinder<USoundCue> SoundCue(TEXT("SoundCue'/Game/Assets/Sounds/Hurricane_Cue.Hurricane_Cue'"));
	if (SoundCue.Succeeded())
	{
		HurricaneSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("HurricaneSound"));
		HurricaneSoundComponent->SetSound(SoundCue.Object);
		HurricaneSoundComponent->SetupAttachment(RootComponent);
	}

}

// Called when the game starts or when spawned
void ATrap::BeginPlay()
{
	Super::BeginPlay();

	HurricaneSoundComponent->Play();
}

// Called every frame
void ATrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ATrap::OnProjectileOverlap(class UPrimitiveComponent* OverlappingComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (!OtherActor->IsA(AATriggerCheckpoint::StaticClass())))
	{
		SetTrapServer(OtherActor);
		Destroy();
	}

}

void ATrap::SetTrapServer_Implementation(class AActor* Actor)
{
	ARemoteControlCar* RemoteCar = Cast<ARemoteControlCar>(Actor);
	if (RemoteCar)
	{
		RemoteCar->FeltOnTrap = true;
		if (HasAuthority())
		{
			RemoteCar->OnFeltOnTrap();
		}
	}
}

