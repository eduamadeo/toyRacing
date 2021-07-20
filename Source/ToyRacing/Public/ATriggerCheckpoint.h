// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TriggerBox.h"
#include "ATriggerCheckpoint.generated.h"

UCLASS()
class TOYRACING_API AATriggerCheckpoint : public ATriggerBox
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Order")
	int OrderNumber;
	
protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	// constructor sets default values for this actor's properties
	AATriggerCheckpoint();

	// declare overlap begin function
	UFUNCTION()
	void OnOverlap(AActor* OverlappedActor, AActor* OtherActor);

};
