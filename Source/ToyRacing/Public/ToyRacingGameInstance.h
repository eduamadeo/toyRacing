// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ToyRacingGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class TOYRACING_API UToyRacingGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
	int NumPlayers;
};
