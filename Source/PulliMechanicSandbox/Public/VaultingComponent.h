// Andrea Pullia

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VaultingComponent.generated.h"


class UCapsuleComponent;
class UCharacterMovementComponent;

struct FVaultResult
{
	FVector FrontWallImpactPoint;
	FVector FrontWallNormal;
	FVector LedgePoint;
	FVector TargetLandingLocation;
	bool bShouldVault = false;
	bool bShouldMantle = false;
	bool bIsValid = false;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UVaultingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVaultingComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool TryDetectVault(FVaultResult& OutVaultData) const;
	void DoVault(const FVaultResult& VaultData);
	
protected:
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CharacterMovementComponent;
	
	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CharacterCapsuleComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Settings")
	float MinVaultingHeight = 30.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Settings")
	float MaxVaultingHeight = 200.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Settings")
	float MinWallDepth = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Settings")
	float MinSpeedToVault = .5f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Settings")
	float MaxDegreesBeforeVault = 75.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Settings")
	float MinSpaceToVaultFromLedgePoint = 55.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Settings|Trace")
	float TraceLength = 50.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Settings|Trace")
	float CapsuleZOffset = 5.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Settings|Trace")
	float GroundTraceTolerance = 10.f;
private:
	bool CanTryToVault() const;
	bool HasHitValidWall(const FHitResult& Hit, const FVector& TraceDirection) const;
	bool TrySearchingForValidLedgePoint(const FVector& LedgePointTraceStart, const FVector& LedgePointTraceEnd, FHitResult& VerticalWallHit, const FCollisionQueryParams& CollisionParams) const;

	float CachedVaultCosineThreshold = -1.f;
};

