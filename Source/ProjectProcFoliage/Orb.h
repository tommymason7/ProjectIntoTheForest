// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InteractableParent.h"

#include "Components/TimelineComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "Orb.generated.h"

DECLARE_DYNAMIC_DELEGATE(FOrbGrabbed);

/**
 * 
 */
UCLASS()
class PROJECTPROCFOLIAGE_API AOrb : public AInteractableParent
{
	GENERATED_BODY()

	UPROPERTY()
	FTimeline floatingMovementTimeline;

	FOnTimelineFloat floatingTimelineCallback;

	UPROPERTY()
	UCurveFloat* timelineCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Particles, meta = (AllowPrivateAccess = "true"))
	UNiagaraSystem* NiagaraSystem;

	FVector originalLocation;

	float ZOffset = 0.0;

	UFUNCTION()
	void FloatingTimelineCallback(float val);

public:
	AOrb();

	UFUNCTION(BlueprintCallable)
	void SetTimelineCurve(UCurveFloat* newTimelineCurve);

	UFUNCTION(BlueprintCallable)
	void Start();

	UFUNCTION(BlueprintCallable)
	void SetOrbCollectionDelegate(const FOrbGrabbed& delegate);

	FOrbGrabbed orbGrabbedDelegate;

protected:
	// Called when the game starts or when spawned
	void BeginPlay() override;

	void ObjectOverlapping(APlayerCharacter* player) override;

	void Tick(float deltaTime) override;
};
