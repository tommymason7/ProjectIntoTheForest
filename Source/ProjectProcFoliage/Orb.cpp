// Fill out your copyright notice in the Description page of Project Settings.


#include "Orb.h"

AOrb::AOrb()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Particle System
	particleSystem = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Particle System"));
	particleSystem->SetupAttachment(meshComponent);
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

void AOrb::ObjectOverlapping(APlayerCharacter* player)
{
	// Execute delegate set earlier
	orbGrabbedDelegate.Broadcast();

	GetWorld()->DestroyActor(this);
}