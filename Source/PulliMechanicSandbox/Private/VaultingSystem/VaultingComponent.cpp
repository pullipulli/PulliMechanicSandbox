// Andrea Pullia


#include "VaultingSystem/VaultingComponent.h"

#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "VaultingSystem/VaultingNotifyState.h"

UVaultingComponent::UVaultingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
			
			FVector VaultTraceStart = MantleFinalPosition;
			VaultTraceStart.Z += CapsuleShape.GetCapsuleHalfHeight();
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
				if (GroundDiff <= GroundTraceCapsuleTolerance)
				{
					OutVaultData.bShouldVault = true;
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
	if (!VaultData.bIsValid) return;
	
	if (VaultData.bShouldMantle)
	{
		GEngine->AddOnScreenDebugMessage(-1,1, FColor::Blue, "DoMantle!");
		ActiveMontage = MantleMontage;
	}
	if (VaultData.bShouldVault)
	{
		GEngine->AddOnScreenDebugMessage(-1,1, FColor::Blue, "DoVault!");
		ActiveMontage = VaultMontage;
	}
	
	CurrentVaultData = VaultData;
	
	CharacterMotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
		FName("Landing"),
		VaultData.TargetLandingLocation,
		(-VaultData.FrontWallNormal).Rotation()
	);
	
	FVector WorldSpaceHandOffset = GetOwner()->GetActorRotation().RotateVector(HandOffset);
	
	CharacterMotionWarpingComponent->AddOrUpdateWarpTargetFromLocationAndRotation(
		FName("LedgePoint"),
		VaultData.LedgePoint + WorldSpaceHandOffset,
		(-VaultData.FrontWallNormal).Rotation()
	);
	
	for (const auto& NotifyEvent : ActiveMontage->Notifies)
	{
		if (auto* VaultNotify = Cast<UVaultingNotifyState>(NotifyEvent.NotifyStateClass))
		{
			VaultNotify->VaultAnimationStart.AddDynamic(this, &UVaultingComponent::OnVaultNotifyBegin);
			VaultNotify->VaultAnimationStop.AddDynamic(this, &UVaultingComponent::OnVaultNotifyEnd);
			
			ActiveVaultingNotifyState = VaultNotify;
			break;
		}
	}
	
	SkeletonAnimInstance->OnMontageEnded.AddDynamic(this, &UVaultingComponent::OnAnimMontageEnd);
	SkeletonAnimInstance->Montage_Play(ActiveMontage);
	if (CurrentVaultData.bShouldVault)
	{
		ActiveVaultingNotifyState->VaultAnimationTick.AddDynamic(this, &UVaultingComponent::CheckForGround);
	}
	
	//GetOwner()->SetActorLocation(VaultData.TargetLandingLocation);
}


void UVaultingComponent::OnVaultNotifyBegin()
{
	CharacterMovementComponent->SetMovementMode(MOVE_Flying);
	if (IsValid(CurrentVaultData.LandingActor))
		CharacterCapsuleComponent->IgnoreActorWhenMoving(CurrentVaultData.LandingActor, true);
}

void UVaultingComponent::OnVaultNotifyEnd()
{
	if (IsValid(ActiveVaultingNotifyState))
	{
		ActiveVaultingNotifyState->VaultAnimationStart.RemoveDynamic(this, &UVaultingComponent::OnVaultNotifyBegin);
		ActiveVaultingNotifyState->VaultAnimationStop.RemoveDynamic(this, &UVaultingComponent::OnVaultNotifyEnd);
	}

	SkeletonAnimInstance->OnMontageEnded.RemoveDynamic(this, &UVaultingComponent::OnAnimMontageEnd);
	CharacterMovementComponent->SetMovementMode(MOVE_Walking);
	if (IsValid(CurrentVaultData.LandingActor))
		CharacterCapsuleComponent->IgnoreActorWhenMoving(CurrentVaultData.LandingActor, false);
	
	ActiveVaultingNotifyState = nullptr;
	ActiveMontage = nullptr;
	CurrentVaultData.bIsValid = false;
}

void UVaultingComponent::OnAnimMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == ActiveMontage && bInterrupted)
	{
		OnVaultNotifyEnd();
	}
}

void UVaultingComponent::CheckForGround()
{
	FHitResult GroundHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.AddIgnoredActor(CurrentVaultData.LandingActor);
	
	if (GetWorld()->LineTraceSingleByChannel(
		GroundHit,
		CharacterMovementComponent->GetActorFeetLocation(),
		CharacterMovementComponent->GetActorFeetLocation() - CharacterCapsuleComponent->GetUpVector() * GroundTraceWhileVaultingTolerance,
		ECC_WorldStatic,
		QueryParams
		))
	{
		OnGroundHitAfterVault();
	}
}

void UVaultingComponent::OnGroundHitAfterVault()
{
	if (CurrentVaultData.bShouldVault)
	{
		FAlphaBlendArgs AlphaBlend;
		AlphaBlend.BlendOption = EAlphaBlendOption::QuadraticInOut;
		AlphaBlend.BlendTime = 0.1f;
		SkeletonAnimInstance->Montage_StopWithBlendOut(AlphaBlend, VaultMontage);
		
		ActiveVaultingNotifyState->VaultAnimationTick.RemoveDynamic(this, &UVaultingComponent::CheckForGround);
		UE_LOG(LogTemp, Log, TEXT("GROUND HIT WHILE VAULTING, STOP MONTAGE"));
	}
}

bool UVaultingComponent::CanTryToVault() const
{
	FVector CurrentVelocity = CharacterMovementComponent->Velocity;
	FVector Forward = GetOwner()->GetActorForwardVector();
	float ForwardSpeed = FVector::DotProduct(CurrentVelocity, Forward);
	
	return ForwardSpeed >= MinSpeedToVault;
}

