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

bool UVaultingComponent::HasHitValidWall(const FHitResult& Hit, const FVector& TraceDirection) const
{
	return Hit.bBlockingHit &&
		FVector::DotProduct(TraceDirection, Hit.ImpactNormal) <= -CachedVaultCosineThreshold;
}

bool UVaultingComponent::TrySearchingForValidLedgePoint(const FVector& LedgePointTraceStart, const FVector& LedgePointTraceEnd, FHitResult& VerticalWallHit, const FCollisionQueryParams& CollisionParams) const
{
	return GetWorld()->LineTraceSingleByChannel(
		VerticalWallHit,
		LedgePointTraceStart,
		LedgePointTraceEnd,
		ECC_WorldStatic,
		CollisionParams
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
	FVector TraceDirection;
	
	if (CurrentVelocity.IsNearlyZero())
	{
		TraceDirection = GetOwner()->GetActorForwardVector();
	}
	else
	{
		TraceDirection = CurrentVelocity.GetSafeNormal2D();
	}
	
	FVector SweepStartLocation = CharacterCapsuleComponent->GetComponentLocation();
	SweepStartLocation.Z += CapsuleZOffset;
	
	GetWorld()->SweepSingleByChannel(
		ForwardWallHit,
		SweepStartLocation,
		SweepStartLocation + TraceDirection * TraceLength,
		CharacterCapsuleComponent->GetComponentQuat(),
		ECC_WorldStatic,
		CapsuleShape,
		CollisionParams
		);
	
	if (HasHitValidWall(ForwardWallHit, TraceDirection))
	{
		float FloorZ =  GetOwner()->GetActorLocation().Z - CapsuleShape.GetCapsuleHalfHeight();
		FVector ImpactPointInsideWall = ForwardWallHit.ImpactPoint - ForwardWallHit.ImpactNormal * MinWallDepth;
		FVector LedgePointTraceStart = FVector{ImpactPointInsideWall.X, ImpactPointInsideWall.Y, FloorZ + MaxVaultingHeight};
		FVector LedgePointTraceEnd = FVector{ImpactPointInsideWall.X, ImpactPointInsideWall.Y, FloorZ + MinVaultingHeight};
		
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
		if (TrySearchingForValidLedgePoint(LedgePointTraceStart, LedgePointTraceEnd, VerticalWallHit, CollisionParams))
		{
			OutVaultData.bIsValid = true;
			OutVaultData.FrontWallNormal = ForwardWallHit.ImpactNormal;
			OutVaultData.FrontWallImpactPoint = ForwardWallHit.ImpactPoint;
			OutVaultData.LedgePoint = VerticalWallHit.ImpactPoint;
			
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
			
			FVector MantleFinalPosition = VerticalWallHit.ImpactPoint;
			MantleFinalPosition.Z += CapsuleShape.GetCapsuleHalfHeight();
			
			FVector VaultTraceStart = MantleFinalPosition;
			VaultTraceStart += -OutVaultData.FrontWallNormal * MinSpaceToVaultFromLedgePoint;
			
			FVector VaultTraceEnd = FVector{VaultTraceStart.X, VaultTraceStart.Y, GetOwner()->GetActorLocation().Z};
			
			DrawDebugDirectionalArrow(
				GetWorld(),
				VaultTraceStart,
				VaultTraceEnd,
				5.f,
				FColor::Green,
				false,
				5.f,
				10,
				1.f
				);
			
			FHitResult MantleHitResult;
			
			bool bShouldMantle = GetWorld()->SweepSingleByChannel(
				MantleHitResult,
				VaultTraceStart,
				VaultTraceEnd,
				CharacterCapsuleComponent->GetComponentQuat(),
				ECC_WorldStatic,
				CapsuleShape,
				CollisionParams
			);
			
			OutVaultData.bShouldMantle = bShouldMantle || MantleHitResult.bStartPenetrating;
			
			if (OutVaultData.bShouldMantle)
			{
				DrawDebugCapsule(
					GetWorld(),
					MantleFinalPosition,
					CapsuleShape.GetCapsuleHalfHeight(),
					CapsuleShape.GetCapsuleRadius(),
					CharacterCapsuleComponent->GetComponentQuat(),
					FColor::Red,
					false,
					5.f
				);
			}
			else
			{
				DrawDebugCapsule(
					GetWorld(),
					VaultTraceEnd,
					CapsuleShape.GetCapsuleHalfHeight(),
					CapsuleShape.GetCapsuleRadius(),
					CharacterCapsuleComponent->GetComponentQuat(),
					FColor::White,
					false,
					5.f
				);
			}
			
			return true;
		}
		
		return false;
	}
	
	return false;
}

void UVaultingComponent::DoVault(const FVaultResult& VaultData)
{
	if (VaultData.bShouldMantle)
	{
		GEngine->AddOnScreenDebugMessage(-1,1, FColor::Blue, "DoMantle!");
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1,1, FColor::Blue, "DoVault!");
	}

}

bool UVaultingComponent::CanTryToVault() const
{
	FVector CurrentVelocity = CharacterMovementComponent->Velocity;
	FVector Forward = GetOwner()->GetActorForwardVector();
	float ForwardSpeed = FVector::DotProduct(CurrentVelocity, Forward);
	
	return ForwardSpeed >= MinSpeedToVault;
}

