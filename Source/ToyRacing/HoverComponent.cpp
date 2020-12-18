// Fill out your copyright notice in the Description page of Project Settings.


#include "HoverComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"

// Sets default values for this component's properties
UHoverComponent::UHoverComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	TraceLength = 40.0f;
	HoverForce = 60000.0f;
}


// Called when the game starts
void UHoverComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector Start = GetComponentLocation();
	FVector End = GetComponentLocation() + (GetUpVector() * (-1) * TraceLength);
	FHitResult TraceResult;
	FCollisionQueryParams CollisionParams;
	bool HasColided = GetWorld()->LineTraceSingleByChannel(TraceResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);
	DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f, (uint8)0, 5.f);

	if (HasColided) {
		ParentComponent->AddForceAtLocation(TraceResult.ImpactNormal * FMath::Lerp(HoverForce, 0.0f, (TraceResult.Distance / TraceLength)), GetComponentLocation());
		ParentComponent->SetLinearDamping(3.0f);
		ParentComponent->SetAngularDamping(5.0f);
	}
}

void UHoverComponent::InitComponent(UStaticMeshComponent * HoverCarMesh)
{
	ParentComponent = Cast<UPrimitiveComponent>(HoverCarMesh);
	ParentComponent->SetSimulatePhysics(true);
}
