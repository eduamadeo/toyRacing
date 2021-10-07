// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Power.generated.h"

UCLASS()
class TOYRACING_API APower : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APower();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* BoxComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class UMaterialInstanceConstant* BoxMaterial;

	UPROPERTY(EditAnywhere, Category = "Effects")
		class UParticleSystem* ExplosionEffect;

	/** Audio component for the crash sound */
	UPROPERTY(Category = Display, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UAudioComponent* BoxCrashSoundComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** A timer handle used for providing the time to give player control back.*/
	FTimerHandle InvisibleTimer;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlap(class UPrimitiveComponent* OverlappingComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void GivePower(class AActor* OtherActor);

	void InvisibleTimeEnd();
};
