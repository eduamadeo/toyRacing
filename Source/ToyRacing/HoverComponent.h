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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UPrimitiveComponent* ParentComponent;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void InitComponent(UStaticMeshComponent* HoverCarMesh);

};
