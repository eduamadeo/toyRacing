// Fill out your copyright notice in the Description page of Project Settings.

#include "Projectile.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "ATriggerCheckpoint.h"
#include "../RemoteControlCar.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = false;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
	SphereComponent->InitSphereRadius(5.0f);
	SphereComponent->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RootComponent = SphereComponent;

	if (GetLocalRole() == ROLE_Authority)
	{
		SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnProjectileOverlap);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetupAttachment(RootComponent);

	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> LaserMaterialInstance(TEXT("MaterialInstanceConstant'/Game/Assets/laser/M_Glow_Inst.M_Glow_Inst'"));
	LaserMaterial = LaserMaterialInstance.Object;

	//Set the Static Mesh and its position/scale if we successfully found a mesh asset to use.
	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -37.5f));
		StaticMesh->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.3f));
	}

	TrailEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particles"));
	TrailEffect->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultTrailEffect(TEXT("ParticleSystem'/Game/Assets/laser/Trail.Trail'"));
	if (DefaultTrailEffect.Succeeded())
	{
		TrailEffect->SetTemplate(DefaultTrailEffect.Object);
		TrailEffect->SetRelativeLocation(FVector(0.0f, 0.0f, -37.5f));
	}

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovementComponent->SetUpdatedComponent(SphereComponent);
	ProjectileMovementComponent->InitialSpeed = 4000.0f;
	ProjectileMovementComponent->MaxSpeed = 4000.0f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 0.0f;

	DamageType = UDamageType::StaticClass();
	Damage = 10.0f;

	// Setup the audio component and allocate it a sound cue
	static ConstructorHelpers::FObjectFinder<USoundCue> SoundCue(TEXT("SoundCue'/Game/Assets/Sounds/laser_Cue.laser_Cue'"));
	if (SoundCue.Succeeded())
	{
		LaserSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("LaserSound"));
		LaserSoundComponent->SetSound(SoundCue.Object);
		LaserSoundComponent->SetIsReplicated(true);
		LaserSoundComponent->SetAutoActivate(true);
		LaserSoundComponent->SetupAttachment(RootComponent);
	}
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	
	StaticMesh->SetMaterial(0, LaserMaterial);
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetIsReplicated())
	{
		DestroyProjectileClient();
	}
}

void AProjectile::OnProjectileOverlap(class UPrimitiveComponent* OverlappingComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && (!OtherActor->IsA(AATriggerCheckpoint::StaticClass())) && OtherActor != GetOwner())
	{
		UGameplayStatics::ApplyPointDamage(OtherActor, Damage, FVector(0.0f, 0.0f, 0.0f), SweepResult, GetInstigator()->Controller, this, DamageType);
		Destroy();
	}

}

void AProjectile::DestroyProjectileClient_Implementation()
{
	ARemoteControlCar* RemoteCar = Cast<ARemoteControlCar>(GetOwner());
	if (RemoteCar)
	{
		if (!GetOwner()->HasAuthority() && RemoteCar->IsLocallyControlled())
		{
			RootComponent->SetWorldLocation(FVector(0.0f, 0.0f, 0.0f));
		}
	}
}

