// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/TimelineComponent.h"
#include "Curves/CurveVector.h"
#include "RemoteControlCar.generated.h"

class UCameraComponent;
class USpringArmComponent;

UENUM()
enum class EPower : uint8 {
	NONE,
	ELETRIC,
	TRAP,
	CLOCK
};

UCLASS()
class TOYRACING_API ARemoteControlCar : public APawn
{
	GENERATED_BODY()

	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* SpringArm;

	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* Camera;

	float AccelationFactor;
	float SteerFactor;

	float SidewaysTraction;
	float AngularTraction;
	bool IsMovingForward;
	FVector StraightVelocity;
	float StraightVelocityMagnitude;

public:
	// Sets default values for this pawn's properties
	ARemoteControlCar();

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UBoxComponent* BoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class USkeletalMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UHoverComponent* HoverComp1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UHoverComponent* HoverComp2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UHoverComponent* HoverComp3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UHoverComponent* HoverComp4;

	UPROPERTY(EditAnywhere, Category = "Effects")
		class UParticleSystemComponent* DamagedEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UMaterialInstanceConstant* CarMaterial_1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UMaterialInstanceConstant* CarMaterial_2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UMaterialInstanceConstant* CarMaterial_3;

	/** Audio component for the engine sound */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* EngineSoundComponent;

	/** Audio component for the trap release sound */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* TrapReleaseSoundComponent;

	/** Audio component for the Clock Power sound */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* ClockPowerSoundComponent;

	/** Audio component for the eletric sound */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* EletricSoundComponent;

	/** Audio component for the eletric sound */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* CrashSoundComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suspension")
		float RestLength;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suspension")
		float SpringTravel;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suspension")
		float SpringStiffness;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suspension")
		float DamperStiffness;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
		float WheelRadius;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Wheel")
		FRotator SteeringAngle;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Power")
		EPower CurrentPower;
	// Variable to hold the Curve asset
	UPROPERTY(EditAnywhere)
		UCurveVector* CarTimelineVectorCurve;
	UPROPERTY(EditAnywhere)
		UCurveFloat* CarTimelineFloatCurve;

	UPROPERTY(Replicated)
		int Position;
	UPROPERTY(Replicated)
		int LastCheckpoint;
	UPROPERTY(Replicated)
		int CurrentLap;
	UPROPERTY(Replicated)
		float DistanceFromNextCheckpoint;
	UPROPERTY(Replicated)
		bool IsWaiting;
	UPROPERTY(Replicated)
		bool IsRacing;
	UPROPERTY(ReplicatedUsing = OnRep_HasEnded)
		bool HasEnded;
	UPROPERTY(ReplicatedUsing = OnRep_MaximumVelocity)
		float MaximumVelocity;
	UPROPERTY(ReplicatedUsing = OnRep_FeltOnTrap)
		bool FeltOnTrap;
	UPROPERTY(ReplicatedUsing = OnRep_SetMaterial)
		UMaterialInterface* CarMaterial;

	UPROPERTY()
		TSubclassOf<UUserWidget> FinalMenuWidgetClass;

	UPROPERTY(Transient, Replicated)
		struct FRepMovement ReplicatedMove;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnChecklistDistanceUpdate();

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Projectile")
		TSubclassOf<class AProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Gameplay|Projectile")
		TSubclassOf<class AProjectile> TrapClass;

	//TimelineComponent to animate Shake car meshes
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UTimelineComponent* CarTimelineComp;

	//TimelineComponent to animate Turn car meshes
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		UTimelineComponent* CarTrapTimelineComp;

	/** Function for beginning weapon fire.*/
	UFUNCTION(BlueprintCallable, Category = "Gameplay")
		void StartFire();

	UFUNCTION()
		void HandleFire();

	/** Server function for spawning projectiles.*/
	UFUNCTION(Server, Reliable)
		void HandleFireServer();

	UFUNCTION(Server, Reliable)
		void HandleTrapServer();

	UFUNCTION(Server, Reliable)
		void ApplySlowPowerServer();

	UFUNCTION(Server, Reliable)
		void FinishTrapServer();

	UFUNCTION()
		void ApplySlowPower();

	UFUNCTION()
		void OnRep_HasEnded();

	UFUNCTION()
		void OnRep_MaximumVelocity();

	UFUNCTION()
		void OnRep_FeltOnTrap();

	UFUNCTION()
		void OnRep_SetMaterial();


	/** A timer handle used for providing the time to give player control back.*/
	FTimerHandle DamagedTimer;

	/** A timer handle used for providing the time to give player control back after trap.*/
	FTimerHandle TrappedTimer;

	/** A timer handle used for providing the time to give player velocity back.*/
	FTimerHandle SlowedTimer;

public:

	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }
	FORCEINLINE bool operator<(const ARemoteControlCar& item) const
	{
		if (CurrentLap < item.CurrentLap)
		{
			return false;
		}
		else if (CurrentLap == item.CurrentLap)
		{
			if (LastCheckpoint < item.LastCheckpoint)
			{
				return false;
			}
			else if (LastCheckpoint == item.LastCheckpoint)
			{
				if (DistanceFromNextCheckpoint > item.DistanceFromNextCheckpoint)
				{
					return false;
				}
			}
		}

		return true;
	}
	static FORCEINLINE FRotator Slerp(const FRotator& A, const FRotator& B, const float& Alpha)
	{
		return FQuat::Slerp(A.Quaternion(), B.Quaternion(), Alpha).Rotator();
	}

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	/*UFUNCTION(NetMulticast, Unreliable, WithValidation)
		void Move(FVector Location, FVector LinearVelocity, FVector AngularVelocity, FRotator Rotation);*/

	UFUNCTION()
		void MoveForward(float Val);

	UFUNCTION()
		void MoveRight(float Val);

	UFUNCTION()
		void Traction();

	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(Server, Unreliable, WithValidation)
		void ServerDefinePositions();

	UFUNCTION(Server, Unreliable, WithValidation)
		void ServerMove(FRepMovement NewMove);

	UFUNCTION(BlueprintCallable, Category = "Health")
		float TakeDamage(float DamageTaken, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	//Float Track Signature to handle our update track event
	FOnTimelineVector UpdateFunctionVector;

	//Function which updates our Car's relative location with the timeline graph
	UFUNCTION()
		void UpdateTimelineVectorComp(FVector Output);

	//Float Track Signature to handle our update track event
	FOnTimelineFloat UpdateFunctionFloat;

	//Function which updates our Car's relative location with the timeline graph
	UFUNCTION()
		void UpdateTimelineFloatComp(float Output);

	UFUNCTION()
		void ShowFinalMenu();

	UPROPERTY(Replicated)
		int NumPlayers;

	UPROPERTY(Replicated)
		int NumLaps;

	bool AreAllSpringsGrounded();
	bool IsAnySpringGrounded();
	void SetPhysics();
	void DamageEnd();
	void TrapEnd();
	void SlowEnd();
	void OnFeltOnTrap();

	void MoveReplication(float DeltaTime);
	void MoveReplication_Send();
	void MoveReplication_Receive(float DeltaTime);

	FTimerHandle MemberTimerHandle;
	bool TimerEnded;
	float DeltaSeconds;
	int SpawnOrder;
	bool MaterialChanged;

};
