// Andrea Pullia

#pragma once

#include "CoreMinimal.h"
#include "VaultResult.h"
#include "Components/ActorComponent.h"
#include "VaultingComponent.generated.h"


class UVaultingNotifyState;
class UMotionWarpingComponent;
class UCapsuleComponent;
class UCharacterMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UVaultingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVaultingComponent();

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool TryDetectVault(FVaultResult& OutVaultData) const;
	void DoVault(const FVaultResult& VaultData);
	bool IsVaulting() const;
	
protected:
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CharacterMovementComponent;
	
	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CharacterCapsuleComponent;
	
	UPROPERTY()
	TObjectPtr<UMotionWarpingComponent> CharacterMotionWarpingComponent;
	
	UPROPERTY()
	TObjectPtr<UAnimInstance> SkeletonAnimInstance;
	
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
	float GroundTraceCapsuleTolerance = 10.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Settings|Trace")
	float GroundTraceWhileVaultingTolerance = 0.1f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Settings|Tuning")
	FVector HandOffset = FVector::ZeroVector;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Animation")
	TObjectPtr<UAnimMontage> VaultMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting|Animation")
	TObjectPtr<UAnimMontage> MantleMontage;
	
	UFUNCTION()
	void OnVaultNotifyBegin();
	
	UFUNCTION()
	void OnVaultNotifyEnd();
	
	UFUNCTION()
	void OnAnimMontageEnd(UAnimMontage* Montage, bool bInterrupted);
	
	UFUNCTION()
	void CheckForGround();
private:
	bool CanTryToVault() const;
	bool HasHitValidWall(const FHitResult& Hit, const FVector& TraceDirection) const;
	bool TrySearchingForValidLedgePoint(const FVector& LedgePointTraceStart, const FVector& LedgePointTraceEnd, FHitResult& VerticalWallHit, const FCollisionQueryParams& CollisionParams) const;
	void OnGroundHitAfterVault();
	
	FVaultResult CurrentVaultData;
	
	UPROPERTY()
	TObjectPtr<UVaultingNotifyState> MantleNotifyState = nullptr;
	
	UPROPERTY()
	TObjectPtr<UVaultingNotifyState> VaultNotifyState = nullptr;
	
	UPROPERTY()
	TObjectPtr<UVaultingNotifyState> ActiveVaultingNotifyState = nullptr;
	
	UPROPERTY()
	TObjectPtr<UAnimMontage> ActiveMontage;
	
	float CachedVaultCosineThreshold = -1.f;
	bool bIsVaulting = false;
};

