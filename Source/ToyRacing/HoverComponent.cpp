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

	CalculateCustomPhysics.BindUObject(this, &UHoverComponent::SubstepTick);
	
}


// Called every frame
void UHoverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ParentComponent->GetBodyInstance() != NULL)
	{
		ParentComponent->GetBodyInstance()->AddCustomPhysics(CalculateCustomPhysics);
	}
}

void UHoverComponent::SubstepTick(float DeltaTime, FBodyInstance* BodyInstance)
{
	float PreviousLength = CurrentLength;
	float SpringVelocity = 0.0f;

	const auto ComponentWorldTransform = GetRelativeTransform() * BodyInstance->GetUnrealWorldTransform();
	const auto ComponentLocation = ComponentWorldTransform.GetLocation();

	FVector Start = ComponentLocation;
	FVector End = ComponentLocation + (GetUpVector() * (-1) * TraceLength);
	FHitResult TraceResult;
	FCollisionQueryParams CollisionParams;
	bool HasColided = GetWorld()->LineTraceSingleByChannel(TraceResult, Start, End, ECollisionChannel::ECC_Visibility, CollisionParams);

	if (HasColided) {
		IsGrounded = true;
		ImpactNormal = TraceResult.ImpactNormal;
		ImpactPoint = TraceResult.ImpactPoint;
		CurrentLength = TraceResult.Distance;
		SpringVelocity = (CurrentLength - PreviousLength) / DeltaTime;

		ParentComponent->GetBodyInstance()->AddForceAtPosition(GetUpVector() * (-DampingCoefficient * SpringVelocity - HoverForce * (CurrentLength - TraceLength)), ComponentLocation, false);
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
