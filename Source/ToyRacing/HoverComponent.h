// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HoverComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TOYRACING_API UHoverComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHoverComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suspension")
		float TraceLength;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suspension")
		float HoverForce;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Suspension")
		float DampingCoefficient;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suspension")
		bool IsGrounded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UPrimitiveComponent* ParentComponent;

	FVector ImpactNormal;
	FVector ImpactPoint;
	float CurrentLength;

private:
	// The delegate used to register substepped physics
	FCalculateCustomPhysics CalculateCustomPhysics;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// The function used to apply substepped physics
	virtual void SubstepTick(float DeltaTime, FBodyInstance* BodyInstance);

	void InitComponent(UStaticMeshComponent* HoverCarMesh);

};
