// Andrea Pullia


#include "VaultingComponent.h"

#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

UVaultingComponent::UVaultingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVaultingComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterMovementComponent = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
	CharacterCapsuleComponent = GetOwner()->FindComponentByClass<UCapsuleComponent>();
	
	CachedVaultCosineThreshold = FMath::Cos(FMath::DegreesToRadians(MaxDegreesBeforeVault));
}


void UVaultingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UVaultingComponent::HasHitValidWall(const FHitResult& Hit, const FVector& TraceDirection, const bool bHasHitSomething) const
{
	return bHasHitSomething &&
		FVector::DotProduct(TraceDirection, Hit.ImpactNormal) <= -CachedVaultCosineThreshold;
}

bool UVaultingComponent::TrySearchingForValidLedgePoint(const FVector& LedgePointTraceStart, const FVector& LedgePointTraceEnd, FHitResult& VerticalWallHit) const
{
	return GetWorld()->LineTraceSingleByChannel(
		VerticalWallHit,
		LedgePointTraceStart,
		LedgePointTraceEnd,
		ECC_WorldStatic
	) && !VerticalWallHit.bStartPenetrating;
}

bool UVaultingComponent::TryDetectVault(FVaultResult& OutVaultData) const
{
	if (!CanTryToVault()) return false;
	
	FHitResult ForwardWallHit;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());
	FCollisionShape CapsuleShape = CharacterCapsuleComponent->GetCollisionShape();
	
	FVector CurrentVelocity = CharacterMovementComponent->Velocity;
	FVector Forward = GetOwner()->GetActorForwardVector();
	FVector TraceDirection;
	
	if (FMath::IsNearlyZero(FVector::DotProduct(CurrentVelocity, Forward)))
	{
		TraceDirection = GetOwner()->GetActorForwardVector();
	}
	else
	{
		TraceDirection = CurrentVelocity.GetSafeNormal();
	}
	
	FVector SweepStartLocation = CharacterCapsuleComponent->GetComponentLocation();
	SweepStartLocation.Z += CapsuleZOffset;
	
	bool bHasHitSomething = GetWorld()->SweepSingleByChannel(
		ForwardWallHit,
		SweepStartLocation,
		SweepStartLocation + TraceDirection * TraceLength,
		CharacterCapsuleComponent->GetComponentQuat(),
		ECC_WorldStatic,
		CapsuleShape,
		CollisionParams
		);
	
	if (HasHitValidWall(ForwardWallHit, TraceDirection, bHasHitSomething))
	{
		FVector ImpactPointInsideWall = ForwardWallHit.ImpactPoint - ForwardWallHit.ImpactNormal * MinWallDepth;
		FVector LedgePointTraceStart = FVector{ImpactPointInsideWall.X, ImpactPointInsideWall.Y, MaxVaultingHeight};
		FVector LedgePointTraceEnd = FVector{ImpactPointInsideWall.X, ImpactPointInsideWall.Y, MinVaultingHeight};
		
		DrawDebugDirectionalArrow(
			GetWorld(),
			LedgePointTraceStart,
			LedgePointTraceEnd,
			5.f,
			FColor::Black,
			false,
			5.f,
			10,
			1.f
			);
		
		FHitResult VerticalWallHit;
		if (TrySearchingForValidLedgePoint(LedgePointTraceStart, LedgePointTraceEnd, VerticalWallHit))
		{
			OutVaultData.bIsValid = true;
			
			DrawDebugCapsule(
				GetWorld(),
				ForwardWallHit.Location,
				CapsuleShape.GetCapsuleHalfHeight(),
				CapsuleShape.GetCapsuleRadius(),
				CharacterCapsuleComponent->GetComponentQuat(),
				FColor::Red,
				false,
				5.f
			);
			
			DrawDebugPoint(GetWorld(), VerticalWallHit.ImpactPoint, 5, FColor::Purple, false, 5.f, 10.f);
			return true;
		}
		
		return false;
	}
	
	return false;
}

void UVaultingComponent::DoVault(const FVaultResult& VaultData)
{
	GEngine->AddOnScreenDebugMessage(-1,1, FColor::Blue, "DoVault!");
}

bool UVaultingComponent::CanTryToVault() const
{
	FVector CurrentVelocity = CharacterMovementComponent->Velocity;
	FVector Forward = GetOwner()->GetActorForwardVector();
	float ForwardSpeed = FVector::DotProduct(CurrentVelocity, Forward);
	
	return ForwardSpeed >= MinSpeedToVault;
}

