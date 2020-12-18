// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Wheel.generated.h"

UCLASS()
class TOYRACING_API AWheel : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWheel();

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
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
