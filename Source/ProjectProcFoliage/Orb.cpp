// Fill out your copyright notice in the Description page of Project Settings.


#include "Orb.h"

AOrb::AOrb()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AOrb::BeginPlay()
{
	Super::BeginPlay();
}

void AOrb::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	floatingMovementTimeline.TickTimeline(deltaTime);
}

void AOrb::FloatingTimelineCallback(float val)
{
	SetActorLocation(FVector(originalLocation.X, originalLocation.Y, originalLocation.Z + val));
}

void AOrb::Start()
{
	originalLocation = GetActorLocation();

	// Start particle system
	UNiagaraFunctionLibrary::SpawnSystemAttached(NiagaraSystem, meshComponent, FName(), FVector(0.f), FRotator(0.f), EAttachLocation::SnapToTarget, true);

	// Setup bobbing motion
	floatingMovementTimeline.SetLooping(true);
	floatingMovementTimeline.SetTimelineLength(5.0f);
	floatingMovementTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);
	floatingMovementTimeline.SetPlaybackPosition(0.0f, false);

	floatingTimelineCallback.BindUFunction(this, FName("FloatingTimelineCallback"));
	floatingMovementTimeline.AddInterpFloat(timelineCurve, floatingTimelineCallback);
	floatingMovementTimeline.PlayFromStart();
}

void AOrb::SetTimelineCurve(UCurveFloat* newTimelineCurve)
{
	timelineCurve = newTimelineCurve;
}

void AOrb::SetOrbCollectionDelegate(const FOrbGrabbed& delegate)
{
	orbGrabbedDelegate = delegate;
}

void AOrb::ObjectOverlapping(APlayerCharacter* player)
{
	// Execute delegate set earlier
	orbGrabbedDelegate.ExecuteIfBound();

	GetWorld()->DestroyActor(this);
}