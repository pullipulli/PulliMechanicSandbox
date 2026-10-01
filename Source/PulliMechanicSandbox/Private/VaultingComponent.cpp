// Andrea Pullia


#include "VaultingComponent.h"

#include "MotionWarpingComponent.h"
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
	CharacterMotionWarpingComponent = GetOwner()->FindComponentByClass<UMotionWarpingComponent>();
	SkeletonAnimInstance = GetOwner()->FindComponentByClass<USkeletalMeshComponent>()->GetAnimInstance();
	
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
	
	if (FVector2D(CurrentVelocity).IsNearlyZero())
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
			OutVaultData.LandingActor = ForwardWallHit.GetActor();
			
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
			
			DrawDebugPoint(GetWorld(), OutVaultData.LedgePoint, 5, FColor::Purple, false, 5.f, 10.f);
			
			FVector MantleFinalPosition = VerticalWallHit.ImpactPoint;
			MantleFinalPosition.Z += CapsuleShape.GetCapsuleHalfHeight();
			
			FVector VaultTraceStart = MantleFinalPosition;
			VaultTraceStart += (-OutVaultData.FrontWallNormal).GetSafeNormal2D() * MinSpaceToVaultFromLedgePoint;
			
			FVector VaultTraceEnd = FVector{VaultTraceStart.X, VaultTraceStart.Y, FloorZ};
			
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
			
			bool bHasHitGround = GetWorld()->SweepSingleByChannel(
				MantleHitResult,
				VaultTraceStart,
				VaultTraceEnd,
				CharacterCapsuleComponent->GetComponentQuat(),
				ECC_WorldStatic,
				CapsuleShape,
				CollisionParams
			);
			
			if (!bHasHitGround || MantleHitResult.bStartPenetrating)
			{
				OutVaultData.bShouldMantle = true;
				OutVaultData.TargetLandingLocation = MantleFinalPosition;
			}
			else
			{
				float HitGroundZ = MantleHitResult.ImpactPoint.Z;
				float GroundDiff = FMath::Abs(HitGroundZ - FloorZ);
				if (GroundDiff <= GroundTraceTolerance)
				{
					OutVaultData.bShouldMantle = false;
					OutVaultData.TargetLandingLocation = MantleHitResult.Location;
				}
				else
				{
					OutVaultData.bShouldMantle = true;
					OutVaultData.TargetLandingLocation = MantleFinalPosition;
				}
			}

			DrawDebugCapsule(
				GetWorld(),
				OutVaultData.TargetLandingLocation,
				CapsuleShape.GetCapsuleHalfHeight(),
				CapsuleShape.GetCapsuleRadius(),
				CharacterCapsuleComponent->GetComponentQuat(),
				FColor::White,
				false,
				5.f
			);
			
			return true;
		}
		
		return false;
	}
	
	return false;
}

void UVaultingComponent::DoVault(const FVaultResult& VaultData)
{
	UAnimMontage* SelectedMontage;
	if (VaultData.bShouldMantle)
	{
		GEngine->AddOnScreenDebugMessage(-1,1, FColor::Blue, "DoMantle!");
		SelectedMontage = MantleMontage;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1,1, FColor::Blue, "DoVault!");
		SelectedMontage = VaultMontage;
	}
	
	CurrentVaultData = VaultData;
	
	CharacterMovementComponent->SetMovementMode(MOVE_Flying);
	CharacterCapsuleComponent->IgnoreActorWhenMoving(VaultData.LandingActor, true);
	
	CharacterMotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
		FName("Landing"),
		VaultData.TargetLandingLocation,
		(-VaultData.FrontWallNormal).Rotation()
	);
	
	SkeletonAnimInstance->Montage_Play(SelectedMontage);

	//GetOwner()->SetActorLocation(VaultData.TargetLandingLocation);
}



bool UVaultingComponent::CanTryToVault() const
{
	FVector CurrentVelocity = CharacterMovementComponent->Velocity;
	FVector Forward = GetOwner()->GetActorForwardVector();
	float ForwardSpeed = FVector::DotProduct(CurrentVelocity, Forward);
	
	return ForwardSpeed >= MinSpeedToVault;
}

