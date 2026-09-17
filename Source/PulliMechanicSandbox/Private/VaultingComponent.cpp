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
}


void UVaultingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UVaultingComponent::TryDetectVault(FVaultResult& OutVaultData)
{
	if (!CanTryToVault()) return false;
	
	FHitResult Hit;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());
	FVector CurrentVelocity = CharacterMovementComponent->Velocity;
	FCollisionShape CapsuleShape = CharacterCapsuleComponent->GetCollisionShape();
	
	FVector SweepStartLocation = CharacterCapsuleComponent->GetComponentLocation();
	SweepStartLocation.Z += CapsuleZOffset;
	
	bool hasHittedSomething = GetWorld()->SweepSingleByChannel(
		Hit,
		SweepStartLocation,
		SweepStartLocation + CurrentVelocity.GetSafeNormal() * TraceLength,
		CharacterCapsuleComponent->GetComponentQuat(),
		ECC_WorldStatic,
		CapsuleShape,
		CollisionParams
		);
	
	if (hasHittedSomething)
	{
		DrawDebugCapsule(
			GetWorld(),
			Hit.Location,
			CapsuleShape.GetCapsuleHalfHeight(),
			CapsuleShape.GetCapsuleRadius(),
			CharacterCapsuleComponent->GetComponentQuat(),
			FColor::Red,
			false,
			5.f
			);
		return true;
	}
	
	return false;
}

void UVaultingComponent::DoVault(const FVaultResult& VaultData)
{
	GEngine->AddOnScreenDebugMessage(-1,1, FColor::Blue, "DoVault!");
}

bool UVaultingComponent::CanTryToVault()
{
	FVector CurrentVelocity = CharacterMovementComponent->Velocity;
	FVector Forward = GetOwner()->GetActorForwardVector();
	float ForwardSpeed = FVector::DotProduct(CurrentVelocity, Forward);
	
	return ForwardSpeed >= MinSpeedToVault;
}

