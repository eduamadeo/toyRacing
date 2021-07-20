// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "RemoteControlCar.generated.h"

class UCameraComponent;
class USpringArmComponent;

UCLASS()
class TOYRACING_API ARemoteControlCar : public APawn
{
	GENERATED_BODY()

	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		USpringArmComponent* SpringArm;

	UPROPERTY(Category = Camera, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCameraComponent* Camera;

	float MaximumVelocity;
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
	int Position;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UBoxComponent* BoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UHoverComponent* HoverComp1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UHoverComponent* HoverComp2;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UHoverComponent* HoverComp3;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UHoverComponent* HoverComp4;

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

	UPROPERTY(ReplicatedUsing = OnRep_LastCheckpoint)
		int LastCheckpoint;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentLap)
		int CurrentLap;
	UPROPERTY(ReplicatedUsing = OnRep_DistanceFromNextCheckpoint)
		float DistanceFromNextCheckpoint;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void OnChecklistDistanceUpdate();

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

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	/*UFUNCTION(NetMulticast, Unreliable, WithValidation)
		void Move(FVector Location, FVector LinearVelocity, FVector AngularVelocity, FRotator Rotation);*/

	UFUNCTION(Server, Unreliable, WithValidation)
		void ServerMoveForward(float Val);

	UFUNCTION(Server, Unreliable, WithValidation)
		void ServerMoveRight(float Val);

	UFUNCTION(Server, Unreliable, WithValidation)
		void ServerTraction();

	UFUNCTION()
		void OnRep_LastCheckpoint();
	UFUNCTION()
		void OnRep_CurrentLap();
	UFUNCTION()
		void OnRep_DistanceFromNextCheckpoint();

	bool AreAllSpringsGrounded();

	bool IsAnySpringGrounded();

};
