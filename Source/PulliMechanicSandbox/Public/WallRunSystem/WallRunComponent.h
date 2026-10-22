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
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
private:
	bool bIsWallRunning = false;
	FCollisionShape SphereShape;
	ESelectedWallRunHand CurrentHand;
	
	UPROPERTY()
	TObjectPtr<UCharacterMovementComponent> CharacterMovementComponent;
	
	UPROPERTY()
	TObjectPtr<UCapsuleComponent> CharacterCapsuleComponent;
};
