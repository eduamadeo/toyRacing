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

	IsGrounded = false;
	TraceLength = 40.0f;
	HoverForce = 2000.0f;
	DampingCoefficient = 400.0f;
	CurrentLength = TraceLength;
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

	float PreviousLength = CurrentLength;
	float SpringVelocity = 0.0f;

	FVector Start = GetComponentLocation();
	FVector End = GetComponentLocation() + (GetUpVector() * (-1) * TraceLength);
	FHitResult TraceResult;
	FCollisionQueryParams CollisionParams;
	bool HasColided = GetWorld()->LineTraceSingleByChannel(TraceResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);
	//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 1.f, (uint8)0, 5.f);

	if (HasColided) {
		IsGrounded = true;
		ImpactNormal = TraceResult.ImpactNormal;
		ImpactPoint = TraceResult.ImpactPoint;
		CurrentLength = TraceResult.Distance;
		SpringVelocity = (CurrentLength - PreviousLength) / DeltaTime;

		ParentComponent->AddForceAtLocation(GetUpVector() * (-DampingCoefficient * SpringVelocity - HoverForce * (CurrentLength - TraceLength)), GetComponentLocation());
	}
	else 
	{
		IsGrounded = false;
	}
}

void UHoverComponent::InitComponent(UStaticMeshComponent * HoverCarMesh)
{
	ParentComponent = Cast<UPrimitiveComponent>(HoverCarMesh);
	ParentComponent->SetSimulatePhysics(true);
}
