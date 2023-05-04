// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableParent.h"

#include "Components/TimelineComponent.h"
#include "Particles/ParticleSystemComponent.h"

#include "Orb.generated.h"

DECLARE_MULTICAST_DELEGATE(FOrbGrabbed)

/**
 * 
 */
UCLASS()
class PROJECTPROCFOLIAGE_API AOrb : public AInteractableParent
{
	GENERATED_BODY()

	UPROPERTY()
	UParticleSystemComponent* particleSystem = nullptr;

	UPROPERTY()
	UTimelineComponent* floatingMovementTimeline = nullptr;

	FOnTimelineFloat floatingTimelineCallback;

	UCurveFloat* timelineCurve;

	FVector originalLocation;

	UFUNCTION()
	void FloatingTimelineCallback(float val);

public:
	AOrb();

	void SetTimelineCurve(UCurveFloat* newTimelineCurve);

	void Start();

	FOrbGrabbed orbGrabbedDelegate;

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

	void ObjectOverlapping(APlayerCharacter* player) override;
};
