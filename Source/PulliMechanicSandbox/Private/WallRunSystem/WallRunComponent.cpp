// Andrea Pullia 2026

#include "WallRunSystem/WallRunComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values for this component's properties
UWallRunComponent::UWallRunComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

bool UWallRunComponent::StartWallRunIfPossible()
{
	if (IsWallRunning()) return false;
	
	FHitResult LeftHandHitResult;
	FHitResult RightHandHitResult;
	
	FVector CapsuleLocation = CharacterCapsuleComponent->GetComponentLocation();
	float CapsuleWidth = CharacterCapsuleComponent->GetScaledCapsuleRadius();
	FVector TotalHandOffset = CharacterCapsuleComponent->GetRightVector() * (CapsuleWidth + HandOffsetFromRadius);
	
	FVector RightHandLocation = CapsuleLocation + TotalHandOffset;
	FVector LeftHandLocation = CapsuleLocation - TotalHandOffset;
	
	
	bool bHasRightHandHit = GetWorld()->SweepSingleByChannel(
		RightHandHitResult,
		RightHandLocation,
		RightHandLocation + CharacterCapsuleComponent->GetRightVector() * InitialHandRaycastLength,
		FQuat::Identity,
		ECC_WALL_RUN,
		SphereShape
	);
	
	bool bHasLeftHandHit = GetWorld()->SweepSingleByChannel(
		LeftHandHitResult,
		LeftHandLocation,
		LeftHandLocation + -CharacterCapsuleComponent->GetRightVector() * InitialHandRaycastLength,
		FQuat::Identity,
		ECC_WALL_RUN,
		SphereShape
	);
	
	bool bShouldUseRightHand;
	
	if (bHasRightHandHit && bHasLeftHandHit)
	{
		float LeftHandDistance = FVector::DistSquared(LeftHandHitResult.ImpactPoint, CapsuleLocation);
		float RightHandDistance = FVector::DistSquared(RightHandHitResult.ImpactPoint, CapsuleLocation);
		
		bShouldUseRightHand = RightHandDistance <= LeftHandDistance;
	}
	else if (bHasRightHandHit)
	{
		bShouldUseRightHand = true;
	}
	else if (bHasLeftHandHit)
	{
		bShouldUseRightHand = false;
	}
	else
	{
		bIsWallRunning = false;
		return false;
	}
	
	bIsWallRunning = true;
	
	FHitResult ChosenHandHitResult;
	
	if (bShouldUseRightHand)
	{
		ChosenHandHitResult = RightHandHitResult;
		CurrentHand = ESelectedWallRunHand::RightHand;
	}
	else
	{
		ChosenHandHitResult = LeftHandHitResult;
		CurrentHand = ESelectedWallRunHand::LeftHand;
	}
		
	//TODO StartWallRun logic!
	
	FVector MovementDirection = FVector::CrossProduct(ChosenHandHitResult.ImpactNormal, FVector::DownVector);
	
	DrawDebugDirectionalArrow(
		GetWorld(),
		ChosenHandHitResult.ImpactPoint,
		ChosenHandHitResult.ImpactPoint + MovementDirection * InitialHandRaycastLength,
		10.0f,
		FColor::Green,
		true,
		3.0f,
		10
	);
	
	SetComponentTickEnabled(true);
	return true;
}

void UWallRunComponent::EndWallRun()
{
	SetComponentTickEnabled(false);
	bIsWallRunning = false;
}

bool UWallRunComponent::IsWallRunning() const
{
	return bIsWallRunning;
}


// Called when the game starts
void UWallRunComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
	
	CharacterMovementComponent = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	CharacterCapsuleComponent = GetOwner()->FindComponentByClass<UCapsuleComponent>();
	
	SphereShape = FCollisionShape::MakeSphere(HandRaycastRadius);
}


// Called every frame
void UWallRunComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                      FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

