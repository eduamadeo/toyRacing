// Fill out your copyright notice in the Description page of Project Settings.


#include "RemoteControlCar.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "HoverComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Sound/SoundCue.h"
#include "Blueprint/UserWidget.h"
#include "Components/AudioComponent.h"
#include "Math/Vector.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/EngineTypes.h"
#include "Math/UnrealMathUtility.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"
#include "Projectile.h"
#include "Trap.h"
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
	SetReplicatingMovement(false);
	SetReplicateMovement(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> DefaultMesh(TEXT("/Game/StarterContent/Shapes/Shape_Cube.Shape_Cube"));
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	if (DefaultMesh.Succeeded())
	{
		StaticMesh->SetStaticMesh(DefaultMesh.Object);
		StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		StaticMesh->SetWorldScale3D(FVector(1.0f, 1.0f, 0.5f));
		StaticMesh->SetNotifyRigidBodyCollision(true);
		StaticMesh->SetVisibility(false);
	}

	RootComponent = StaticMesh;
	StaticMesh->OnComponentHit.AddDynamic(this, &ARemoteControlCar::OnHit);

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> CarMesh(TEXT("SkeletalMesh'/Game/PolygonTown/Meshes/Vehicles/SK_Veh_Convertable_01.SK_Veh_Convertable_01'"));
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
	if (CarMesh.Succeeded())
	{
		Mesh->SetSkeletalMesh(CarMesh.Object);
		Mesh->SetupAttachment(StaticMesh);
		Mesh->SetRelativeScale3D(FVector(0.4f, 0.4f, 1.0f));
		Mesh->SetRelativeLocation(FVector(0.0f, 0.0f, -65.0f));

		static ConstructorHelpers::FClassFinder<UObject> AnimBPClass(TEXT("/Game/PolygonTown/Meshes/Vehicles/SK_Veh_Convertable_01_Skeleton_AnimBlueprint"));
		if (AnimBPClass.Succeeded())
		{
			Mesh->SetAnimInstanceClass(AnimBPClass.Class);
		}
	}

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->TargetOffset = FVector(0.f, 0.f, 100.f);
	SpringArm->SetRelativeRotation(FRotator(-10.f, 0.f, 0.f));
	SpringArm->SetupAttachment(StaticMesh);
	SpringArm->TargetArmLength = 450.0f;
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

	HoverComp2 = CreateDefaultSubobject<UHoverComponent>(TEXT("HoverComp2"));
	HoverComp2->SetRelativeLocation(FVector(40.0f, -40.0f, -0.01f));
	HoverComp2->SetupAttachment(StaticMesh);

	HoverComp3 = CreateDefaultSubobject<UHoverComponent>(TEXT("HoverComp3"));
	HoverComp3->SetRelativeLocation(FVector(-40.0f, 40.0f, -0.01f));
	HoverComp3->SetupAttachment(StaticMesh);

	HoverComp4 = CreateDefaultSubobject<UHoverComponent>(TEXT("HoverComp4"));
	HoverComp4->SetRelativeLocation(FVector(-40.0f, -40.0f, -0.01f));
	HoverComp4->SetupAttachment(StaticMesh);

	HoverComp1->InitComponent(StaticMesh);
	HoverComp2->InitComponent(StaticMesh);
	HoverComp3->InitComponent(StaticMesh);
	HoverComp4->InitComponent(StaticMesh);

	//Initialize projectile class
	ProjectileClass = AProjectile::StaticClass();

	TrapClass = ATrap::StaticClass();

	CarTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("CarTimelineComp"));
	CarTimelineComp->SetTimelineLength(0.4);
	CarTimelineComp->SetLooping(true);

	CarTrapTimelineComp = CreateDefaultSubobject<UTimelineComponent>(TEXT("CarTrapTimelineComp"));
	CarTrapTimelineComp->SetTimelineLength(0.8);
	CarTrapTimelineComp->SetLooping(true);

	static ConstructorHelpers::FObjectFinder<UCurveVector> CurveVectorClass(TEXT("CurveVector'/Game/Assets/ShakeCurve.ShakeCurve'"));
	CarTimelineVectorCurve = CurveVectorClass.Object;

	static ConstructorHelpers::FObjectFinder<UCurveFloat> CurveFloatClass(TEXT("CurveFloat'/Game/Assets/XCarCurve.XCarCurve'"));
	CarTimelineFloatCurve = CurveFloatClass.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> CarMaterialInstance_1(TEXT("MaterialInstanceConstant'/Game/PolygonTown/Materials/Mat_PolygonTown_02_A_Inst.Mat_PolygonTown_02_A_Inst'"));
	CarMaterial_1 = CarMaterialInstance_1.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> CarMaterialInstance_2(TEXT("MaterialInstanceConstant'/Game/PolygonTown/Materials/Mat_PolygonTown_03_A_Inst.Mat_PolygonTown_03_A_Inst'"));
	CarMaterial_2 = CarMaterialInstance_2.Object;

	static ConstructorHelpers::FObjectFinder<UMaterialInstanceConstant> CarMaterialInstance_3(TEXT("MaterialInstanceConstant'/Game/PolygonTown/Materials/Mat_PolygonTown_04_A_Inst.Mat_PolygonTown_04_A_Inst'"));
	CarMaterial_3 = CarMaterialInstance_3.Object;

	DamagedEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particles"));
	DamagedEffect->SetupAttachment(StaticMesh);
	static ConstructorHelpers::FObjectFinder<UParticleSystem> DefaultTrailEffect(TEXT("ParticleSystem'/Game/StarterContent/Particles/P_Sparks.P_Sparks'"));
	if (DefaultTrailEffect.Succeeded())
	{
		DamagedEffect->SetTemplate(DefaultTrailEffect.Object);
		DamagedEffect->SetAutoActivate(false);
		DamagedEffect->SetRelativeLocation(FVector(100.0f, 0.0f, 10.0f));
	}

	// Setup the audio component and allocate it a sound cue
	static ConstructorHelpers::FObjectFinder<USoundCue> SoundCue(TEXT("SoundCue'/Game/Assets/Sounds/Engine_Cue.Engine_Cue'"));
	if (SoundCue.Succeeded())
	{
		EngineSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineSound"));
		EngineSoundComponent->SetSound(SoundCue.Object);
		EngineSoundComponent->SetIsReplicated(true);
		EngineSoundComponent->SetupAttachment(StaticMesh);
	}

	static ConstructorHelpers::FObjectFinder<USoundCue> TrapSoundCue(TEXT("SoundCue'/Game/Assets/Sounds/trap_realease_Cue.trap_realease_Cue'"));
	if (TrapSoundCue.Succeeded())
	{
		TrapReleaseSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("TrapReleaseSound"));
		TrapReleaseSoundComponent->SetSound(TrapSoundCue.Object);
		TrapReleaseSoundComponent->SetIsReplicated(true);
		TrapReleaseSoundComponent->SetAutoActivate(false);
		TrapReleaseSoundComponent->SetupAttachment(StaticMesh);
	}

	static ConstructorHelpers::FObjectFinder<USoundCue> ClockPowerSoundCue(TEXT("SoundCue'/Game/Assets/Sounds/clock_power_Cue.clock_power_Cue'"));
	if (ClockPowerSoundCue.Succeeded())
	{
		ClockPowerSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("ClockPowerSound"));
		ClockPowerSoundComponent->SetSound(ClockPowerSoundCue.Object);
		ClockPowerSoundComponent->SetIsReplicated(true);
		ClockPowerSoundComponent->SetAutoActivate(false);
		ClockPowerSoundComponent->SetupAttachment(StaticMesh);
	}

	static ConstructorHelpers::FObjectFinder<USoundCue> EletricSoundCue(TEXT("SoundCue'/Game/Assets/Sounds/eletric_Cue.eletric_Cue'"));
	if (EletricSoundCue.Succeeded())
	{
		EletricSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("EletricSound"));
		EletricSoundComponent->SetSound(EletricSoundCue.Object);
		EletricSoundComponent->SetIsReplicated(true);
		EletricSoundComponent->SetAutoActivate(false);
		EletricSoundComponent->SetupAttachment(StaticMesh);
	}

	static ConstructorHelpers::FObjectFinder<USoundCue> CrashSoundCue(TEXT("SoundCue'/Game/Assets/Sounds/car_crash_Cue.car_crash_Cue'"));
	if (CrashSoundCue.Succeeded())
	{
		CrashSoundComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("CrashSound"));
		CrashSoundComponent->SetSound(CrashSoundCue.Object);
		CrashSoundComponent->SetIsReplicated(true);
		CrashSoundComponent->SetAutoActivate(false);
		CrashSoundComponent->SetupAttachment(StaticMesh);
	}

	static ConstructorHelpers::FClassFinder<UUserWidget> FinalMenuWidget(TEXT("/Game/FinalMenu/W_FinalMenu"));
	if (ensure(FinalMenuWidget.Class != nullptr))
	{
		FinalMenuWidgetClass = FinalMenuWidget.Class;
	}

	MaximumVelocity = 1500;
	AccelationFactor = 2000;
	SteerFactor = 7;
	SidewaysTraction = 5;
	AngularTraction = 0.05;
	SteeringAngle = FRotator(0.0f, 0.0f, 0.0f);
	CurrentPower = EPower::NONE;

	LastCheckpoint = 0;
	CurrentLap = 1;
	Position = 0;

	TimerEnded = false;
	FeltOnTrap = false;

	IsWaiting = false;
	IsRacing = false;
	HasEnded = false;
	MaterialChanged = false;
}

// Called when the game starts or when spawned
void ARemoteControlCar::BeginPlay()
{
	Super::BeginPlay();

	StaticMesh->SetMassOverrideInKg(NAME_None, 100.0f);
	StaticMesh->SetCenterOfMass(FVector(0.0f, 0.0f, -180.0f));

	GetWorldTimerManager().SetTimer(MemberTimerHandle, this, &ARemoteControlCar::SetPhysics, 1.0f, true, 2.0f);

	//Binding our float track to our UpdateTimelineComp Function's output
	UpdateFunctionVector.BindDynamic(this, &ARemoteControlCar::UpdateTimelineVectorComp);
	UpdateFunctionFloat.BindDynamic(this, &ARemoteControlCar::UpdateTimelineFloatComp);

	OnCalculateCustomPhysics.BindUObject(this, &ARemoteControlCar::CustomPhysics);

	//If we have a float curve, bind it's graph to our update function
	if (CarTimelineVectorCurve)
	{
		CarTimelineComp->AddInterpVector(CarTimelineVectorCurve, UpdateFunctionVector);
	}
	if (CarTimelineFloatCurve)
	{
		CarTrapTimelineComp->AddInterpFloat(CarTimelineFloatCurve, UpdateFunctionFloat);
	}

	EngineSoundComponent->Play();

	auto PlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
	if (ensure(PlayerController != nullptr))
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
	}
}

// Called every frame
void ARemoteControlCar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DeltaSeconds = DeltaTime;

	if (StaticMesh->GetBodyInstance() != NULL)
	{
		StaticMesh->GetBodyInstance()->AddCustomPhysics(OnCalculateCustomPhysics);
	}

	MoveReplication(DeltaTime);

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
	if (IsLocallyControlled() && !HasEnded)
	{
		ServerDefinePositions();
	}

	EngineSoundComponent->SetFloatParameter("RPM", StraightVelocityMagnitude / MaximumVelocity);

	if (!MaterialChanged)
	{
		if (SpawnOrder == 1)
		{
			MaterialChanged = true;
		}
		else if (SpawnOrder == 2)
		{
			CarMaterial = CarMaterial_1;
			Mesh->SetMaterial(0, CarMaterial_1);
			MaterialChanged = true;
		}
		else if (SpawnOrder == 3)
		{
			CarMaterial = CarMaterial_2;
			Mesh->SetMaterial(0, CarMaterial_2);
			MaterialChanged = true;
		}
		else if (SpawnOrder == 4)
		{
			CarMaterial = CarMaterial_3;
			Mesh->SetMaterial(0, CarMaterial_3);
			MaterialChanged = true;
		}
	}
}

void ARemoteControlCar::PhysicsTick_Implementation(float SubstepDeltaTime)
{
	if (AreAllSpringsGrounded())
	{
		FTransform WorldTransform = StaticMesh->GetBodyInstance()->GetUnrealWorldTransform_AssumesLocked();
		StaticMesh->SetAngularDamping(0.0f);
		FVector ProjectedForward = WorldTransform.GetUnitAxis(EAxis::X);
		StraightVelocity = ProjectedForward * FVector::DotProduct(StaticMesh->GetPhysicsLinearVelocity(), ProjectedForward);
		StraightVelocityMagnitude = StraightVelocity.Size();
		IsMovingForward = FVector::DotProduct(ProjectedForward, StraightVelocity.GetSafeNormal()) > 0.0f;
		Traction();
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

void ARemoteControlCar::CustomPhysics(float DeltaTime, FBodyInstance* BodyInstance)
{
	PhysicsTick(DeltaTime);
}

// Called to bind functionality to input
void ARemoteControlCar::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// set up gameplay key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARemoteControlCar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARemoteControlCar::MoveRight);
	PlayerInputComponent->BindAxis("LookUp");
	PlayerInputComponent->BindAxis("LookRight");
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ARemoteControlCar::StartFire);
}

void ARemoteControlCar::StartFire()
{
	switch(CurrentPower)
	{
	case EPower::ELETRIC:
		HandleFire();
		CurrentPower = EPower::NONE;
		break;
	case EPower::CLOCK:
		ApplySlowPowerServer();
		ClockPowerSoundComponent->Play();
		CurrentPower = EPower::NONE;
		break;
	case EPower::TRAP:
		HandleTrapServer();
		TrapReleaseSoundComponent->Play();
		CurrentPower = EPower::NONE;
		break;
	default:
		CurrentPower = EPower::NONE;
	}
}

void ARemoteControlCar::HandleFire()
{
	if (!HasAuthority())
	{
		//UE_LOG(LogTemp, Warning, TEXT("%s"), (bReplicates ? TEXT("true") : TEXT("false")));
		FVector ForwardProjected = FVector::VectorPlaneProject(GetActorForwardVector(), HoverComp1->ImpactNormal);
		FVector spawnLocation = GetActorLocation() + (ForwardProjected * 100.0f) + (GetActorUpVector() * 10.0f);
		FRotator spawnRotation = ForwardProjected.Rotation();

		FActorSpawnParameters spawnParameters;
		spawnParameters.Instigator = GetInstigator();
		spawnParameters.Owner = this;

		AProjectile* spawnedProjectile = GetWorld()->SpawnActor<AProjectile>(spawnLocation, spawnRotation, spawnParameters);
	}

	HandleFireServer();
}

void ARemoteControlCar::ApplySlowPower()
{
	// Play animation
	CarTimelineComp->Play();
	ClockPowerSoundComponent->Play();
	GetWorldTimerManager().SetTimer(SlowedTimer, this, &ARemoteControlCar::SlowEnd, 8.0f, false);
}

void ARemoteControlCar::OnRep_HasEnded()
{
	IsRacing = false;
	DisableInput(Cast<APlayerController>(this));
}

void ARemoteControlCar::OnRep_MaximumVelocity()
{
	if (MaximumVelocity < 1500.0f)
	{
		ApplySlowPower();
	}
}

void ARemoteControlCar::OnRep_FeltOnTrap()
{
	OnFeltOnTrap();
}

void ARemoteControlCar::OnRep_SetMaterial()
{
	Mesh->SetMaterial(0, CarMaterial);
}

void ARemoteControlCar::OnFeltOnTrap()
{
	if (FeltOnTrap)
	{
		// Brake
		if (StraightVelocityMagnitude > 0.0f)
		{
			MoveForward(StraightVelocityMagnitude * -0.03f);
		}

		// Play animation
		CarTrapTimelineComp->Play();

		DisableInput(Cast<APlayerController>(this));
		GetWorldTimerManager().SetTimer(TrappedTimer, this, &ARemoteControlCar::TrapEnd, 3.2f, false);
	}
}

void ARemoteControlCar::HandleFireServer_Implementation()
{
	FVector ForwardProjected = FVector::VectorPlaneProject(GetActorForwardVector(), HoverComp1->ImpactNormal);
	FVector spawnLocation = GetActorLocation() + (ForwardProjected * 100.0f) + (GetActorUpVector() * 10.0f);
	FRotator spawnRotation = ForwardProjected.Rotation();

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = GetInstigator();
	spawnParameters.Owner = this;

	AProjectile* spawnedProjectile = GetWorld()->SpawnActor<AProjectile>(spawnLocation, spawnRotation, spawnParameters);
	spawnedProjectile->SetReplicates(true);
}

void ARemoteControlCar::HandleTrapServer_Implementation()
{
	FVector ForwardProjected = FVector::VectorPlaneProject(GetActorForwardVector(), HoverComp1->ImpactNormal);
	FVector spawnLocation = GetActorLocation() - (ForwardProjected * 120.0f) + (GetActorUpVector() * 35.0f);
	FRotator spawnRotation = ForwardProjected.Rotation();

	FActorSpawnParameters spawnParameters;
	spawnParameters.Instigator = GetInstigator();
	spawnParameters.Owner = this;

	ATrap* spawnedProjectile = GetWorld()->SpawnActor<ATrap>(spawnLocation, spawnRotation, spawnParameters);
	spawnedProjectile->SetReplicates(true);
}

void ARemoteControlCar::ApplySlowPowerServer_Implementation()
{
	TArray<ARemoteControlCar*> RemoteControlCars;
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARemoteControlCar::StaticClass(), Actors);

	for (AActor* Actor : Actors)
	{
		ARemoteControlCar* RemoteControlCar = Cast<ARemoteControlCar>(Actor);
		if (RemoteControlCar != this)
		{
			RemoteControlCar->MaximumVelocity = 800.0f;

			if (RemoteControlCar->HasAuthority())
			{
				RemoteControlCar->ApplySlowPower();
			}
		}
	}
}

void ARemoteControlCar::FinishTrapServer_Implementation()
{
	this->FeltOnTrap = false;
}

void ARemoteControlCar::UpdateTimelineVectorComp(FVector Output)
{
	FRotator CarNewRotation = FRotator(Output.X, Output.Y, Output.Z);
	Mesh->SetRelativeRotation(CarNewRotation);
}

void ARemoteControlCar::UpdateTimelineFloatComp(float Output)
{
	FRotator CarRotation = Mesh->GetRelativeRotation();
	FRotator CarNewRotation = FRotator(CarRotation.Pitch, Output, CarRotation.Roll);
	Mesh->SetRelativeRotation(CarNewRotation);
}

void ARemoteControlCar::ShowFinalMenu()
{
	UUserWidget* MyWidget = CreateWidget<UUserWidget>(GetWorld(), FinalMenuWidgetClass);

	MyWidget->AddToViewport();

	// Step 0 get player controller.
	auto PlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
	if (ensure(PlayerController != nullptr))
	{
		// Step 1 setup an input mode. There are multiple such as game only or game and UI as well.
		FInputModeUIOnly InputModeData;
		// Step 2 config is specific to the type
		InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputModeData.SetWidgetToFocus(MyWidget->TakeWidget()); //Because UMG wraps Slate

		// Step 3 set the mode for the player controller
		PlayerController->SetInputMode(InputModeData);

		// Step 4 enable cursor so you know what to click on:
		PlayerController->bShowMouseCursor = true;
	}
}

float ARemoteControlCar::TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	// Brake
	if (StraightVelocityMagnitude > 0.0f)
	{
		MoveForward(StraightVelocityMagnitude * -0.03f);
	}

	DamagedEffect->Activate();
	EletricSoundComponent->Play();

	DisableInput(Cast<APlayerController>(this));
	GetWorldTimerManager().SetTimer(DamagedTimer, this, &ARemoteControlCar::DamageEnd, 3.0f, false);
	return DamageTaken;
}

bool ARemoteControlCar::ServerDefinePositions_Validate()
{
	return true;
}


void ARemoteControlCar::ServerDefinePositions_Implementation()
{
	int Counter = 1;
	TArray<ARemoteControlCar*> RemoteControlCars;
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ARemoteControlCar::StaticClass(), Actors);

	bool ShouldRun = true;
	for (AActor* Actor : Actors)
	{
		ARemoteControlCar* RemoteControlCar = Cast<ARemoteControlCar>(Actor);
		if (RemoteControlCar != nullptr)
		{
			if (!RemoteControlCar->HasEnded)
			{
				RemoteControlCars.Push(RemoteControlCar);
			}
			else
			{
				Counter++;
			}
		}
		else
		{
			ShouldRun = false;
		}
	}

	if (ShouldRun)
	{
		RemoteControlCars.Sort();

		for (ARemoteControlCar* RCar : RemoteControlCars)
		{
			RCar->Position = Counter;
			Counter++;
		}
	}
}

bool ARemoteControlCar::AreAllSpringsGrounded()
{
	return HoverComp1->IsGrounded && HoverComp2->IsGrounded && HoverComp3->IsGrounded && HoverComp4->IsGrounded;
}

bool ARemoteControlCar::IsAnySpringGrounded()
{
	return HoverComp1->IsGrounded || HoverComp2->IsGrounded || HoverComp3->IsGrounded || HoverComp4->IsGrounded;
}

void ARemoteControlCar::SetPhysics()
{
	TimerEnded = true;

	GetWorldTimerManager().ClearTimer(MemberTimerHandle);
}

void ARemoteControlCar::DamageEnd()
{
	DamagedEffect->Deactivate();
	EletricSoundComponent->Stop();
	EnableInput(Cast<APlayerController>(this));
}

void ARemoteControlCar::TrapEnd()
{
	EnableInput(Cast<APlayerController>(this));

	// Stop animation
	CarTrapTimelineComp->Stop();
	Mesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	FinishTrapServer();
}

void ARemoteControlCar::SlowEnd()
{
	// Stop Animation
	CarTimelineComp->Stop();
	Mesh->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	MaximumVelocity = 1500.0f;
}

void ARemoteControlCar::MoveForward(float Val)
{
	if (IsRacing)
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
}

void ARemoteControlCar::MoveRight(float Val)
{
	//UE_LOG(LogTemp, Warning, TEXT("%f %s"), StraightVelocityMagnitude, (AreAllSpringsGrounded() ? TEXT("true") : TEXT("false")));
	if (IsRacing)
	{
		if (StraightVelocityMagnitude > 0.01f || !AreAllSpringsGrounded())
		{
			FTransform LocalTransform = Mesh->GetSocketTransform(TEXT("SM_Veh_Convertable_Wheel_fr"), RTS_Component);
			if (!FMath::IsNearlyZero(Val, 0.01f))
			{
				if (IsMovingForward)
				{
					StaticMesh->AddTorqueInRadians(FVector(0.0f, 0.0f, SteerFactor * Val), NAME_None, true);
				}
				else
				{
					StaticMesh->AddTorqueInRadians(FVector(0.0f, 0.0f, -SteerFactor * Val), NAME_None, true);
				}
				SteeringAngle = Slerp(LocalTransform.Rotator(), FRotator(0.0f, Val * 30.0f, 0.0f), 0.1);
			}
			else
			{
				SteeringAngle = Slerp(LocalTransform.Rotator(), FRotator(0.0f, 0.0f, 0.0f), 0.1);
			}
		}
	}
}

void ARemoteControlCar::Traction()
{
	// Sideways traction
	FVector ContrarySidewaysVelocity = -StaticMesh->GetPhysicsLinearVelocity().ProjectOnToNormal(this->GetActorRightVector());
	if (ContrarySidewaysVelocity.SizeSquared() > 0.0f)
	{
		StaticMesh->GetBodyInstance()->AddForce(ContrarySidewaysVelocity * SidewaysTraction, false, true);
	}

	// Angular traction
	StaticMesh->GetBodyInstance()->AddTorqueInRadians(-FVector(0.0f, 0.0f, StaticMesh->GetPhysicsAngularVelocity().Z) * AngularTraction, false, true);

	// Wheels traction
	const float WHEELS_ROLLING_RESISTANCE_COEFFICIENT = 0.2f;
	FVector WheelsRollingResistanceForce = -(StraightVelocity * WHEELS_ROLLING_RESISTANCE_COEFFICIENT);
	StaticMesh->GetBodyInstance()->AddForce(WheelsRollingResistanceForce, false, true);
}

void ARemoteControlCar::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	CrashSoundComponent->SetWorldLocation(Hit.ImpactPoint);
	if (!CrashSoundComponent->IsPlaying() && StraightVelocityMagnitude > 800.0f)
	{
		CrashSoundComponent->Play();
	}
}

bool ARemoteControlCar::ServerMove_Validate(FRepMovement NewMove)
{
	return true;
}

void ARemoteControlCar::ServerMove_Implementation(FRepMovement NewMove)
{
	ReplicatedMove = NewMove;
}

void ARemoteControlCar::MoveReplication(float DeltaTime)
{
	if (IsLocallyControlled())
	{
		MoveReplication_Send();
	}
	else
	{
		MoveReplication_Receive(DeltaTime);
	}
}

void ARemoteControlCar::MoveReplication_Send()
{
	FRepMovement NewMove = FRepMovement();
	NewMove.Location = GetActorLocation();
	NewMove.Rotation = GetActorRotation();
	NewMove.AngularVelocity = StaticMesh->GetPhysicsAngularVelocity();
	NewMove.LinearVelocity = StaticMesh->GetPhysicsLinearVelocity();
	ServerMove(NewMove);
}

void ARemoteControlCar::MoveReplication_Receive(float DeltaTime)
{
	if (ReplicatedMove.Location.X > 0.1f && ReplicatedMove.Location.Y > 0.1f && ReplicatedMove.Location.Z > 0.1f)
	{
		FVector NewLocation = FMath::VInterpTo(GetActorLocation(), ReplicatedMove.Location, DeltaTime, 2.0);
		FVector NewVelocity = FMath::VInterpTo(StaticMesh->GetPhysicsLinearVelocity(), ReplicatedMove.LinearVelocity, DeltaTime, 2.0);
		FVector NewAngularVelocity = FMath::VInterpTo(StaticMesh->GetPhysicsAngularVelocity(), ReplicatedMove.AngularVelocity, DeltaTime, 2.0);
		FRotator NewRotation = Slerp(GetActorRotation(), ReplicatedMove.Rotation, 0.8);
		SetActorLocation(NewLocation);
		SetActorRotation(NewRotation);
		StaticMesh->SetPhysicsLinearVelocity(NewVelocity);
		StaticMesh->SetPhysicsAngularVelocity(NewAngularVelocity);
	}
}

void ARemoteControlCar::GetLifetimeReplicatedProps(TArray <FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARemoteControlCar, Position);
	DOREPLIFETIME(ARemoteControlCar, LastCheckpoint);
	DOREPLIFETIME(ARemoteControlCar, CurrentLap);
	DOREPLIFETIME(ARemoteControlCar, DistanceFromNextCheckpoint);
	DOREPLIFETIME(ARemoteControlCar, IsWaiting);
	DOREPLIFETIME(ARemoteControlCar, IsRacing);
	DOREPLIFETIME(ARemoteControlCar, HasEnded);
	DOREPLIFETIME(ARemoteControlCar, ReplicatedMove);
	DOREPLIFETIME(ARemoteControlCar, MaximumVelocity);
	DOREPLIFETIME(ARemoteControlCar, FeltOnTrap);
	DOREPLIFETIME(ARemoteControlCar, CarMaterial);
	DOREPLIFETIME(ARemoteControlCar, NumPlayers);
	DOREPLIFETIME(ARemoteControlCar, NumLaps);
}
