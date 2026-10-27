// Andrea Pullia 2026

#pragma once

#define ECC_WALL_RUN ECC_GameTraceChannel1

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallRunComponent.generated.h"

class UCapsuleComponent;
class UCharacterMovementComponent;

UENUM(BlueprintType)
enum class ESelectedWallRunHand : uint8 {
	LeftHand UMETA(DisplayName = "Left Hand"),
	RightHand UMETA(DisplayName = "Right Hand")
};

UENUM(BlueprintType)
enum class EWallRunApproachStatus : uint8 {
	Frontal UMETA(DisplayName = "Frontal"),
	Approaching UMETA(DisplayName = "Approaching"),
	Leaving UMETA(DisplayName = "Leaving")
};

USTRUCT(BlueprintType)
struct PULLIMECHANICSANDBOX_API FWallRunAnimationData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category="WallRun")
	ESelectedWallRunHand Hand;
	
	UPROPERTY(BlueprintReadOnly, Category="WallRun")
	FVector MovementDirection;
	
	UPROPERTY(BlueprintReadOnly, Category="WallRun")
	FVector WallNormal;
	
	UPROPERTY(BlueprintReadOnly, Category="WallRun")
	FVector Velocity;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWallRunComponentEvent, const FWallRunAnimationData&, WallAnimationData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWallRunComponentEventEnd);

DECLARE_LOG_CATEGORY_CLASS(LogWallRun, Log, All);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PULLIMECHANICSANDBOX_API UWallRunComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWallRunComponent();

	UFUNCTION(BlueprintCallable)
	bool StartWallRunIfPossible();
	
	UFUNCTION(BlueprintCallable)
	void EndWallRun();
	
	bool IsWallRunning() const;
	
	UPROPERTY(BlueprintAssignable, Category="WallRun")
	FWallRunComponentEvent OnWallRunStarted;
	
	UPROPERTY(BlueprintAssignable, Category="WallRun")
	FWallRunComponentEvent OnWallRunTick;
	
	UPROPERTY(BlueprintAssignable, Category="WallRun")
	FWallRunComponentEventEnd OnWallRunEnd;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="WallRun|Hands")
	float HandOffsetFromRadius = 0.0f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="WallRun|Hands")
	float InitialHandRaycastLength = 10.0f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="WallRun|Hands")
	float HandRaycastLengthWhileWallRunning = 2.0f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="WallRun|Hands")
	float HandRaycastRadius = 2.0f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="WallRun|Approaching", meta = (ClampMax = 179.0f, ClampMin = 91.0f))
	float MinApproachAngle = 91.0f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="WallRun|Approaching", meta = (ClampMax = 179.0f, ClampMin = 91.0f))
	float MaxApproachAngle = 160.0f;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	bool bIsWallRunning = false;
	FCollisionShape SphereShape;
	ESelectedWallRunHand CurrentHand;
	FHitResult CurrentChosenHandHitResult;
	FVector TotalHandOffset;
	
	FVector CalculateCurrentMovementDirection();
	EWallRunApproachStatus CalculateCurrentWallRunApproachStatus(FVector WallNormal) const;
	
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CharacterMovementComponent;
	
	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CharacterCapsuleComponent;
};
