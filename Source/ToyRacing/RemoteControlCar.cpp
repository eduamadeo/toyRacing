// Fill out your copyright notice in the Description page of Project Settings.


#include "RemoteControlCar.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "HoverComponent.h"
#include "Math/Vector.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "ATriggerCheckpoint.h"

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

	SetReplicates(true);
	SetReplicatingMovement(true);
	SetReplicateMovement(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	StaticMesh->SetCenterOfMass(FVector(0.0f, 0.0f, -180.0f));

	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMesh->SetRelativeScale3D(FVector(1.5f, 1.0f, 0.4f));
	}

	RootComponent = StaticMesh;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("RootComponent"));
	BoxComponent->SetBoxExtent(FVector(1, 1, 1));
	BoxComponent->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	BoxComponent->SetSimulatePhysics(true);
	BoxComponent->SetupAttachment(StaticMesh);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->TargetOffset = FVector(0.f, 0.f, 100.f);
	SpringArm->SetRelativeRotation(FRotator(-15.f, 0.f, 0.f));
	SpringArm->SetupAttachment(StaticMesh);
	SpringArm->TargetArmLength = 600.0f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.f;
	SpringArm->bInheritPitch = true;
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

	MaximumVelocity = 1200;
	AccelationFactor = 2000;
	SteerFactor = 2000000;
	SidewaysTraction = 5;
	AngularTraction = 10000;

	LastCheckpoint = 0;
	CurrentLap = 0;
	Position = 0;
}

// Called when the game starts or when spawned
void ARemoteControlCar::BeginPlay()
{
	Super::BeginPlay();

	
}

void ARemoteControlCar::OnChecklistDistanceUpdate()
{
	int Counter = 1;
	TArray<ARemoteControlCar*> RemoteControlCars;
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARemoteControlCar::StaticClass(), Actors);

	for (AActor* Actor : Actors)
	{
		ARemoteControlCar* RemoteControlCar = Cast<ARemoteControlCar>(Actor);
		RemoteControlCars.Push(RemoteControlCar);
	}

	RemoteControlCars.Sort();

	for (ARemoteControlCar* RCar : RemoteControlCars)
	{
		RCar->Position = Counter;
		Counter++;
	}
}

// Called every frame
void ARemoteControlCar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Warning, TEXT("%f"), FVector::DotProduct(StaticMesh->GetPhysicsLinearVelocity(), StaticMesh->GetRightVector()));

	//StaticMesh->AddForce(StaticMesh->GetRightVector() * FVector::DotProduct(StaticMesh->GetPhysicsLinearVelocity(), StaticMesh->GetRightVector() * -300));
	if (IsLocallyControlled() && Role < ROLE_Authority)
	{
		if (AreAllSpringsGrounded())
		{
			StaticMesh->SetAngularDamping(0.0f);
			FVector ProjectedForward = StaticMesh->GetForwardVector();
			StraightVelocity = ProjectedForward * FVector::DotProduct(StaticMesh->GetPhysicsLinearVelocity(), ProjectedForward);
			StraightVelocityMagnitude = StraightVelocity.Size();
			IsMovingForward = FVector::DotProduct(ProjectedForward, StraightVelocity.GetSafeNormal()) > 0.0f;
			ServerTraction();
		}
		else if (IsAnySpringGrounded())
		{
			StaticMesh->SetAngularDamping(0.0f);
		}
		else
		{
			StaticMesh->SetAngularDamping(10.0f);
		}
	}
	else if (Role == ROLE_Authority)
	{
		if (AreAllSpringsGrounded())
		{
			StaticMesh->SetAngularDamping(0.0f);
			FVector ProjectedForward = StaticMesh->GetForwardVector();
			StraightVelocity = ProjectedForward * FVector::DotProduct(StaticMesh->GetPhysicsLinearVelocity(), ProjectedForward);
			StraightVelocityMagnitude = StraightVelocity.Size();
			IsMovingForward = FVector::DotProduct(ProjectedForward, StraightVelocity.GetSafeNormal()) > 0.0f;
			ServerTraction();
		}
		else if (IsAnySpringGrounded())
		{
			StaticMesh->SetAngularDamping(0.0f);
		}
		else
		{
			StaticMesh->SetAngularDamping(10.0f);
		}

		OnChecklistDistanceUpdate();
	}

	// Placement logic
	TArray<AActor*> Checkpoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AATriggerCheckpoint::StaticClass(), Checkpoints);

	for (AActor* Actor : Checkpoints)
	{
		AATriggerCheckpoint* Checkpoint = Cast<AATriggerCheckpoint>(Actor);
		if (Checkpoint->OrderNumber == (LastCheckpoint + 1) % Checkpoints.Num())
		{
			DistanceFromNextCheckpoint = FVector::DistSquared(GetActorLocation(), Checkpoint->GetActorLocation());
			//UE_LOG(LogTemp, Warning, TEXT("%.02f %d %d %d"), DistanceFromNextCheckpoint, LastCheckpoint, CurrentLap, Position);
		}
	}
}

// Called to bind functionality to input
void ARemoteControlCar::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARemoteControlCar::ServerMoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARemoteControlCar::ServerMoveRight);
	PlayerInputComponent->BindAxis("LookUp");
	PlayerInputComponent->BindAxis("LookRight");
}

void ARemoteControlCar::OnRep_LastCheckpoint()
{
}

void ARemoteControlCar::OnRep_CurrentLap()
{
}

void ARemoteControlCar::OnRep_DistanceFromNextCheckpoint()
{
	OnChecklistDistanceUpdate();
}

bool ARemoteControlCar::AreAllSpringsGrounded()
{
	return HoverComp1->IsGrounded && HoverComp2->IsGrounded && HoverComp3->IsGrounded && HoverComp4->IsGrounded;
}

bool ARemoteControlCar::IsAnySpringGrounded()
{
	return HoverComp1->IsGrounded || HoverComp2->IsGrounded || HoverComp3->IsGrounded || HoverComp4->IsGrounded;
}

bool ARemoteControlCar::ServerMoveForward_Validate(float Val)
{
	return true;
}

void ARemoteControlCar::ServerMoveForward_Implementation(float Val)
{
	if (AreAllSpringsGrounded())
	{
		FVector ForwardProjected = FVector::VectorPlaneProject(StaticMesh->GetForwardVector(), HoverComp1->ImpactNormal);
		//UE_LOG(LogTemp, Warning, TEXT("%f %f %f - %f %f %f"), ForwardProjected.X, ForwardProjected.Y, ForwardProjected.Z, HoverComp1->ImpactNormal.X, HoverComp1->ImpactNormal.Y, HoverComp1->ImpactNormal.Z);
		if (Val > 0.0f)
		{
			if (!IsMovingForward || StraightVelocityMagnitude < MaximumVelocity)
			{
				StaticMesh->AddForce(ForwardProjected * AccelationFactor * Val, NAME_None, true);
			}
		}
		else if (Val < 0.0f)
		{
			if (IsMovingForward || StraightVelocityMagnitude < MaximumVelocity * 0.4f)
			{
				StaticMesh->AddForce(ForwardProjected * AccelationFactor * Val, NAME_None, true);
			}
		}
	}
}

//bool ARemoteControlCar::Move_Validate(FVector Location, FVector LinearVelocity, FVector AngularVelocity, FRotator Rotation)
//{
//	return true;
//}
//
//void ARemoteControlCar::Move_Implementation(FVector Location, FVector LinearVelocity, FVector AngularVelocity, FRotator Rotation)
//{
//	FVector difference = Location - this->GetActorLocation();
//	float distance = difference.Size();
//
//	if (distance > 2.0f)
//	{
//		this->SetActorLocation(Location);
//	}
//	else if (distance > 0.1f)
//	{
//		this->SetActorLocation(difference * 0.1f);
//	}
//
//	StaticMesh->SetPhysicsLinearVelocity(LinearVelocity);
//	StaticMesh->SetPhysicsAngularVelocity(AngularVelocity);
//
//	this->SetActorRotation(Rotation);
//}

bool ARemoteControlCar::ServerMoveRight_Validate(float Val)
{
	return true;
}

void ARemoteControlCar::ServerMoveRight_Implementation(float Val)
{
	//UE_LOG(LogTemp, Warning, TEXT("%f %s"), StraightVelocityMagnitude, (AreAllSpringsGrounded() ? TEXT("true") : TEXT("false")));
	if (StraightVelocityMagnitude > 0.01f || !AreAllSpringsGrounded())
	{
		if (!FMath::IsNearlyZero(Val, 0.01f))
		{
			StaticMesh->AddTorque(FVector(0.0f, 0.0f, SteerFactor * Val));
		}
	}
}

bool ARemoteControlCar::ServerTraction_Validate()
{
	return true;
}

void ARemoteControlCar::ServerTraction_Implementation()
{
	//StaticMesh->AddForce(StaticMesh->GetRightVector() * FVector::DotProduct(StaticMesh->GetPhysicsLinearVelocity(), StaticMesh->GetRightVector() * -300));

	// Sideways traction
	FVector ContrarySidewaysVelocity = -StaticMesh->GetPhysicsLinearVelocity().ProjectOnToNormal(this->GetActorRightVector());
	//UE_LOG(LogTemp, Warning, TEXT("%f %f %f"), ContrarySidewaysVelocity.X, ContrarySidewaysVelocity.Y, ContrarySidewaysVelocity.Z);
	if (ContrarySidewaysVelocity.SizeSquared() > 0.0f)
	{
		StaticMesh->AddForce(ContrarySidewaysVelocity * SidewaysTraction, NAME_None, true);
	}

	// Angular traction
	StaticMesh->AddTorque(-FVector(0.0f, 0.0f, StaticMesh->GetPhysicsAngularVelocity().Z) * AngularTraction);

	// Wheels traction
	const float WHEELS_ROLLING_RESISTANCE_COEFFICIENT = 0.2f;
	FVector WheelsRollingResistanceForce = -(StraightVelocity * WHEELS_ROLLING_RESISTANCE_COEFFICIENT);
	StaticMesh->AddForce(WheelsRollingResistanceForce, NAME_None, true);

	//Move(this->GetActorLocation(), StaticMesh->GetPhysicsLinearVelocity(), StaticMesh->GetPhysicsAngularVelocity(), this->GetActorRotation());
}

void ARemoteControlCar::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARemoteControlCar, LastCheckpoint);
	DOREPLIFETIME(ARemoteControlCar, CurrentLap);
	DOREPLIFETIME(ARemoteControlCar, DistanceFromNextCheckpoint);
}
