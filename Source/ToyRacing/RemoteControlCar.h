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

public:
	// Sets default values for this pawn's properties
	ARemoteControlCar();

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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }
	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
