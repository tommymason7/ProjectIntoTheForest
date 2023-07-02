// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Blueprint/UserWidget.h"

#include "ParentArchitecture.h"
#include "PlayerCharacter.h"

#include "Teleporter.generated.h"

DECLARE_DYNAMIC_DELEGATE(FPlayerOverlapped);

/**
 * 
 */
UCLASS()
class PROJECTPROCFOLIAGE_API ATeleporter : public AParentArchitecture
{
	GENERATED_BODY()

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Box = nullptr;

	int numCollected = 0;
	int numSpawned = -1;
	float percentageNeeded = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Materials, meta = (AllowPrivateAccess = "true"))
	FName materialSlotToChange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Materials, meta = (AllowPrivateAccess = "true"))
	UMaterialInterface* activeMat;


public:
	ATeleporter();

	UFUNCTION(BlueprintCallable)
	void OrbCollected();

	UFUNCTION(BlueprintCallable)
	void setNumSpawned(int spawnedAmnt);

	UFUNCTION(BlueprintCallable)
	void setPercentageNeeded(float perc);

	void setActiveMaterial(UMaterialInterface* newMaterial);
	void setActiverMaterialSlotName(FName slotName);

	virtual void setMesh(UStaticMesh* newMesh) override;

	UFUNCTION(BlueprintCallable)
	void SetTeleportDelegate(const FPlayerOverlapped& delegate);

	FPlayerOverlapped teleportDelegate;

protected:
	void BeginPlay() override;
};
