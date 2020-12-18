// Fill out your copyright notice in the Description page of Project Settings.


#include "RemoteControlCar.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "HoverComponent.h"

float MinLength;
float MaxLength;
float LastLength;
float SpringLength;
float SpringVelocity;
float SpringForce;
float DamperForce;

// Sets default values
ARemoteControlCar::ARemoteControlCar()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("RootComponent"));
	BoxComponent->SetBoxExtent(FVector(1, 1, 1));
	BoxComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	BoxComponent->SetSimulatePhysics(true);
	RootComponent = BoxComponent;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetupAttachment(RootComponent);

	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMesh->SetRelativeScale3D(FVector(1.5f, 1.0f, 0.4f));
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->TargetOffset = FVector(0.f, 0.f, 200.f);
	SpringArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	SpringArm->SetupAttachment(StaticMesh);
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.f;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritRoll = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.f;

	HoverComp1 = CreateDefaultSubobject<UHoverComponent>(TEXT("HoverComp1"));
	HoverComp1->SetRelativeLocation(FVector(40.0f, 40.0f, -0.01f));
	HoverComp1->SetupAttachment(StaticMesh);
	HoverComp1->InitComponent(StaticMesh);

	HoverComp2 = CreateDefaultSubobject<UHoverComponent>(TEXT("HoverComp2"));
	HoverComp2->SetRelativeLocation(FVector(40.0f, -40.0f, -0.01f));
	HoverComp2->SetupAttachment(StaticMesh);
	HoverComp2->InitComponent(StaticMesh);

	HoverComp3 = CreateDefaultSubobject<UHoverComponent>(TEXT("HoverComp3"));
	HoverComp3->SetRelativeLocation(FVector(-40.0f, 40.0f, -0.01f));
	HoverComp3->SetupAttachment(StaticMesh);
	HoverComp3->InitComponent(StaticMesh);

	HoverComp4 = CreateDefaultSubobject<UHoverComponent>(TEXT("HoverComp4"));
	HoverComp4->SetRelativeLocation(FVector(-40.0f, -40.0f, -0.01f));
	HoverComp4->SetupAttachment(StaticMesh);
	HoverComp4->InitComponent(StaticMesh);

	/*RestLength = 0.5f;
	SpringTravel = 0.2f;
	SpringStiffness = 30000.0f;
	DamperStiffness = 4000.0f;
	WheelRadius = 0.33f;*/
}

// Called when the game starts or when spawned
void ARemoteControlCar::BeginPlay()
{
	Super::BeginPlay();
	
	//MinLength = RestLength - SpringTravel;
	//MaxLength = RestLength + SpringTravel;
}

// Called every frame
void ARemoteControlCar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*FVector SuspensionForce;

	FVector Start = GetActorLocation() + (GetActorRotation().Vector() * 60.0f) + (GetActorRightVector() * 40.0f) + (GetActorUpVector() * 2 * WheelRadius);
	FVector End = Start - GetActorUpVector() * (MaxLength + WheelRadius);
	FHitResult TraceResult;
	FCollisionQueryParams CollisionParams;
	bool HasColided = GetWorld()->LineTraceSingleByChannel(TraceResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f, (uint8)0, 5.f);

	if (HasColided)
	{
		LastLength = SpringLength;
		SpringLength = TraceResult.Distance - WheelRadius;
		SpringLength = FMath::Clamp(SpringLength, MinLength, MaxLength);
		SpringVelocity = (LastLength - SpringLength) / DeltaTime;
		SpringForce = SpringStiffness * (RestLength - SpringLength);
		DamperForce = DamperStiffness * SpringVelocity;

		SuspensionForce = SpringForce * GetActorUpVector();
		BoxComponent->AddForceAtLocation(SuspensionForce, Start);
	}

	Start = GetActorLocation() + (GetActorRotation().Vector() * -60.0f) + (GetActorRightVector() * 40.0f) + (GetActorUpVector() * 2 * WheelRadius);
	End = Start - GetActorUpVector() * (MaxLength + WheelRadius);
	HasColided = GetWorld()->LineTraceSingleByChannel(TraceResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f, (uint8)0, 5.f);

	if (HasColided)
	{
		LastLength = SpringLength;
		SpringLength = TraceResult.Distance - WheelRadius;
		SpringLength = FMath::Clamp(SpringLength, MinLength, MaxLength);
		SpringVelocity = (LastLength - SpringLength) / DeltaTime;
		SpringForce = SpringStiffness * (RestLength - SpringLength);
		DamperForce = DamperStiffness * SpringVelocity;

		SuspensionForce = SpringForce * GetActorUpVector();
		BoxComponent->AddForceAtLocation(SuspensionForce, Start);
	}

	Start = GetActorLocation() + (GetActorRotation().Vector() * 60.0f) + (GetActorRightVector() * -40.0f) + (GetActorUpVector() * 2 * WheelRadius);
	End = Start - GetActorUpVector() * (MaxLength + WheelRadius);
	HasColided = GetWorld()->LineTraceSingleByChannel(TraceResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f, (uint8)0, 5.f);

	if (HasColided)
	{
		LastLength = SpringLength;
		SpringLength = TraceResult.Distance - WheelRadius;
		SpringLength = FMath::Clamp(SpringLength, MinLength, MaxLength);
		SpringVelocity = (LastLength - SpringLength) / DeltaTime;
		SpringForce = SpringStiffness * (RestLength - SpringLength);
		DamperForce = DamperStiffness * SpringVelocity;

		SuspensionForce = SpringForce * GetActorUpVector();
		BoxComponent->AddForceAtLocation(SuspensionForce, Start);
	}

	Start = GetActorLocation() + (GetActorRotation().Vector() * -60.0f) + (GetActorRightVector() * -40.0f) + (GetActorUpVector() * 2 * WheelRadius);
	End = Start - GetActorUpVector() * (MaxLength + WheelRadius);
	HasColided = GetWorld()->LineTraceSingleByChannel(TraceResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f, (uint8)0, 5.f);

	if (HasColided)
	{
		LastLength = SpringLength;
		SpringLength = TraceResult.Distance - WheelRadius;
		SpringLength = FMath::Clamp(SpringLength, MinLength, MaxLength);
		SpringVelocity = (LastLength - SpringLength) / DeltaTime;
		SpringForce = SpringStiffness * (RestLength - SpringLength);
		DamperForce = DamperStiffness * SpringVelocity;

		SuspensionForce = SpringForce * GetActorUpVector();
		BoxComponent->AddForceAtLocation(SuspensionForce, Start);
	}*/

}

// Called to bind functionality to input
void ARemoteControlCar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

