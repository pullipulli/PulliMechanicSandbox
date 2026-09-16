// Andrea Pullia

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "VaultingComponent.generated.h"


class UCharacterMovementComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UVaultingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UVaultingComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	bool CanVaultOrMantle();
	void DoVaultOrMantle();
	
protected:
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UCharacterMovementComponent> CharacterMovementComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting | Settings")
	float MinVaultingHeight = 30.f;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting | Settings")
	float MaxVaultingHeight = 200.f;
	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting | Settings")
	float MinWallDepth = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Vaulting | Settings")
	float MinSpeedToVault = .5f;
};
