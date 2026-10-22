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
	PrimaryComponentTick.bStartWithTickEnabled = false;

	// ...
}

FVector UWallRunComponent::CalculateCurrentMovementDirection()
{
	FVector CapsuleLocation = CharacterCapsuleComponent->GetComponentLocation();
	FVector HandLocation;
	FVector HandRaycastEndLocation;
	
	if (CurrentHand == ESelectedWallRunHand::LeftHand)
	{
		HandLocation = CapsuleLocation - TotalHandOffset;
		HandRaycastEndLocation = HandLocation - CharacterCapsuleComponent->GetRightVector() * InitialHandRaycastLength;
	}
	else if (CurrentHand == ESelectedWallRunHand::RightHand)
	{
		HandLocation = CapsuleLocation + TotalHandOffset;
		HandRaycastEndLocation = HandLocation + CharacterCapsuleComponent->GetRightVector() * InitialHandRaycastLength;
	}
	
	bool bHasHandHit = GetWorld()->SweepSingleByChannel(
		CurrentChosenHandHitResult,
		HandLocation,
		HandRaycastEndLocation,
		FQuat::Identity,
		ECC_WALL_RUN,
		SphereShape
	);
	
	// before this, we should also check if there is a 90° internal turn using a forward sweep!
	if (!bHasHandHit)
	{
		UE_LOG(LogWallRun, Warning, TEXT("In this case should try to check if there is 90° external turn!"));
	}
	
	FVector MovementDirection = FVector::CrossProduct(CurrentChosenHandHitResult.ImpactNormal, FVector::DownVector);
	
	FVector VelocityXY{CharacterMovementComponent->Velocity};
	VelocityXY.Z = 0.0f;
	
	// if velocity and wall movement direction are opposite, then invert the MovementDirection
	// because we want to face the current forward of the player character
	if (FVector::DotProduct(MovementDirection, VelocityXY) < 0.0f)
	{ 
		MovementDirection = -MovementDirection;
	}
	
	DrawDebugDirectionalArrow(
		GetWorld(),
		CurrentChosenHandHitResult.ImpactPoint,
		CurrentChosenHandHitResult.ImpactPoint + MovementDirection * InitialHandRaycastLength,
		10.0f,
		FColor::Green,
		true,
		3.0f,
		10
	);
	
	// for now, it should work, in the future maybe I would want a tiny fall during the wallRun
	MovementDirection.Z = 0.0f;
	
	return MovementDirection;
}

bool UWallRunComponent::StartWallRunIfPossible()
{
	if (IsWallRunning()) return false;
	
	FHitResult LeftHandHitResult;
	FHitResult RightHandHitResult;
	
	FVector CapsuleLocation = CharacterCapsuleComponent->GetComponentLocation();
	float CapsuleWidth = CharacterCapsuleComponent->GetScaledCapsuleRadius();
	TotalHandOffset = CharacterCapsuleComponent->GetRightVector() * (CapsuleWidth + HandOffsetFromRadius);

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
	
	if (bShouldUseRightHand)
	{
		CurrentChosenHandHitResult = RightHandHitResult;
		CurrentHand = ESelectedWallRunHand::RightHand;
	}
	else
	{
		CurrentChosenHandHitResult = LeftHandHitResult;
		CurrentHand = ESelectedWallRunHand::LeftHand;
	}
	
	FVector MovementDirection = CalculateCurrentMovementDirection();
	
	// start WallRun logic in the tick!
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
	
	CharacterMovementComponent = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	CharacterCapsuleComponent = GetOwner()->FindComponentByClass<UCapsuleComponent>();
	
	SphereShape = FCollisionShape::MakeSphere(HandRaycastRadius);
}


// Called every frame
void UWallRunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (!IsWallRunning()) return;
	
	FVector MovementDirection = CalculateCurrentMovementDirection();
	float ForwardSpeed = FVector::DotProduct(CharacterMovementComponent->Velocity, MovementDirection);
	
	CharacterMovementComponent->Velocity = MovementDirection * ForwardSpeed;
}

