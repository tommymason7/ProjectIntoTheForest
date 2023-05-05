// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"

#include "ParentArchitecture.h"

#include "Teleporter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPROCFOLIAGE_API ATeleporter : public AParentArchitecture
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Box = nullptr;

	int numCollected = 0;
	int numSpawned = -1;
	float percentageNeeded = -1;
	FName materialSlotToChange;

	UMaterialInterface* activeMat;


public:
	ATeleporter();

	void Activate();

	UFUNCTION()
	void OrbCollected();

	void setNumSpawned(int spawnedAmnt);
	void setPercentageNeeded(float perc);
	void setActiveMaterial(UMaterialInterface* newMaterial);
	void setActiverMaterialSlotName(FName slotName);
};
