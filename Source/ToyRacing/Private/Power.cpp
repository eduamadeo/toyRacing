// Fill out your copyright notice in the Description page of Project Settings.


#include "Power.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Materials/MaterialInstanceConstant.h"
#include "UObject/ConstructorHelpers.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Components/PrimitiveComponent.h"
#include "Particles/ParticleSystem.h"
#include "Engine/Engine.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"
#include "../RemoteControlCar.h"

// Sets default values
APower::APower()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	BoxComp->SetNotifyRigidBodyCollision(true);
	BoxComp->BodyInstance.SetCollisionProfileName("OverlapAllDynamic");
	BoxComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
	BoxComp->SetWorldScale3D(FVector(1.5f, 1.5f, 1.5f));
	BoxComp->SetSimulatePhysics(false);
	BoxComp->SetEnableGravity(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetupAttachment(BoxComp);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -30.0f));
		StaticMesh->SetRelativeScale3D(FVector(0.60f, 0.60f, 0.60f));
		StaticMesh->BodyInstance.SetCollisionProfileName("OverlapAllDynamic");
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> BoxMaterialInstance(TEXT("MaterialInstanceConstant'/Game/Assets/box/Box_Mat_Inst.Box_Mat_Inst'"));
	BoxMaterial = BoxMaterialInstance.Object;

	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultExplosionEffect(TEXT("ParticleSystem'/Game/Assets/box/Box_Crash.Box_Crash'"));
	ExplosionEffect = DefaultExplosionEffect.Object;

	// Setup the audio component and allocate it a sound cue
	static ConstructorHelpers::FObjectFinder<USoundCue> SoundCue(TEXT("SoundCue'/Game/Assets/Sounds/box_crash_Cue.box_crash_Cue'"));
	BoxCrashSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BoxCrashSound"));
	if (SoundCue.Succeeded())
	{
		BoxCrashSoundComponent->SetSound(SoundCue.Object);
		BoxCrashSoundComponent->SetAutoActivate(false);
		BoxCrashSoundComponent->SetupAttachment(StaticMesh);
	}

	RootComponent = BoxComp;

	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &APower::OnOverlap);
}

// Called when the game starts or when spawned
void APower::BeginPlay()
{
	Super::BeginPlay();
	
	StaticMesh->SetMaterial(0, BoxMaterial);
}

// Called every frame
void APower::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APower::OnOverlap(class UPrimitiveComponent* OverlappingComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	//UE_LOG(LogTemp, Warning, TEXT("%s"), (RootComponent->IsVisible() ? TEXT("true") : TEXT("false")));
	if (StaticMesh->IsVisible())
	{
		GivePower(OtherActor);
		RootComponent->SetVisibility(false, true);
		GetWorldTimerManager().SetTimer(InvisibleTimer, this, &APower::InvisibleTimeEnd, 5.0f, false);

		FVector spawnLocation = GetActorLocation();
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplosionEffect, spawnLocation, FRotator::ZeroRotator, true, EPSCPoolMethod::AutoRelease);

		BoxCrashSoundComponent->Play();
	}
	//Destroy();
}

void APower::GivePower(AActor* OtherActor)
{
	ARemoteControlCar* RemoteCar = Cast<ARemoteControlCar>(OtherActor);

	if (RemoteCar && RemoteCar->CurrentPower == EPower::NONE)
	{
		if (RemoteCar->Position == 1)
		{
			RemoteCar->CurrentPower = EPower::TRAP;
		}
		else if (RemoteCar->Position < 4)
		{
			int RandomNum = FMath::RandRange(1, 2);
			if (RandomNum == 1)
			{
				RemoteCar->CurrentPower = EPower::ELETRIC;
			}
			else
			{
				RemoteCar->CurrentPower = EPower::TRAP;
			}
		}
		else
		{
			int RandomNum = FMath::RandRange(1, 2);
			if (RandomNum == 1)
			{
				RemoteCar->CurrentPower = EPower::ELETRIC;
			}
			else
			{
				RemoteCar->CurrentPower = EPower::CLOCK;
			}
		}
	}
}

void APower::InvisibleTimeEnd()
{
	RootComponent->SetVisibility(true, true);
}

