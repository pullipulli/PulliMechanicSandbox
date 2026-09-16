// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PulliMechanicSandboxGameMode.generated.h"

/**
 *  Simple GameMode for a third person game
 */
UCLASS(abstract)
class APulliMechanicSandboxGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	
	/** Constructor */
	APulliMechanicSandboxGameMode();
};



