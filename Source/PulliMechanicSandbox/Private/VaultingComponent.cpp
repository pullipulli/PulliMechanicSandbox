// Andrea Pullia


#include "VaultingComponent.h"

#include "GameFramework/CharacterMovementComponent.h"

UVaultingComponent::UVaultingComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UVaultingComponent::BeginPlay()
{
	Super::BeginPlay();

	CharacterMovementComponent = GetOwner()->FindComponentByClass<UCharacterMovementComponent>();
}


// Called every frame
void UVaultingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UVaultingComponent::CanVaultOrMantle()
{
	FVector CurrentVelocity = CharacterMovementComponent->Velocity;
	FVector Forward = CharacterMovementComponent->GetForwardVector();
	float ForwardSpeed = FVector::DotProduct(CurrentVelocity, Forward);
	
	return ForwardSpeed >= MinSpeedToVault;
}

void UVaultingComponent::DoVaultOrMantle()
{
	if (!CanVaultOrMantle())
		return;
	
	UE_LOG(LogTemp, Warning, TEXT("Can Vault!"));
}

