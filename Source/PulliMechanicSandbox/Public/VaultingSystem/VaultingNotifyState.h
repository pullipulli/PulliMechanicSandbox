// Andrea Pullia 2026

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "VaultingNotifyState.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVaultAnimationEvent);

UCLASS()
class PULLIMECHANICSANDBOX_API UVaultingNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	FVaultAnimationEvent VaultAnimationStart;
	FVaultAnimationEvent VaultAnimationStop;
	FVaultAnimationEvent VaultAnimationTick;
	
protected:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};
