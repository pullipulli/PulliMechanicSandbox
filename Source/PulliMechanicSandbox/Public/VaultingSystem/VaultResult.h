// Andrea Pullia 2026

#pragma once

struct FVaultResult
{
	FVector FrontWallImpactPoint;
	FVector FrontWallNormal;
	FVector LedgePoint;
	FVector TargetLandingLocation;
	AActor* LandingActor;
	bool bShouldVault = false;
	bool bShouldMantle = false;
	bool bIsValid = false;
};