// Fill out your copyright notice in the Description page of Project Settings.


#include "Teleporter.h"

ATeleporter::ATeleporter()
{
	// Create subobject for mesh and attach to root
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	Box->SetupAttachment(meshComponent);
}

void ATeleporter::BeginPlay()
{
	Super::BeginPlay();

	Box->OnComponentBeginOverlap.AddDynamic(this, &ATeleporter::OnOverlapBegin);

	// Turn off collision until we have met the orb condition
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATeleporter::OrbCollected()
{
	// Increase tracking of orbs
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Orb Collected")));
	numCollected++;

	// If the player has collected enough orbs, change the material
	if (numCollected / numSpawned >= percentageNeeded)
	{
		int32 matIndex = meshComponent->GetStaticMesh()->GetMaterialIndexFromImportedMaterialSlotName(materialSlotToChange);
		meshComponent->GetStaticMesh()->SetMaterial(matIndex, activeMat);

		// Turn on collision
		Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ATeleporter::setNumSpawned(int spawnedAmnt)
{
	numSpawned = spawnedAmnt;
}

void ATeleporter::setPercentageNeeded(float perc)
{
	percentageNeeded = perc;
}

void ATeleporter::setActiveMaterial(UMaterialInterface* newMaterial)
{
	activeMat = newMaterial;
}

void ATeleporter::setActiverMaterialSlotName(FName slotName)
{
	materialSlotToChange = slotName;
}

void ATeleporter::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                            bool bFromSweep, const FHitResult& SweepResult)
{
	// Only way for this to be happening is if the player got enough orbs, so don't need to check the orb collection
	// Call Delegate to Generator
	if (OtherActor->IsA(APlayerCharacter::StaticClass()))
	{
		teleportDelegate.ExecuteIfBound();
	}
}

void ATeleporter::setMesh(UStaticMesh* newMesh)
{
	Super::setMesh(newMesh);

	Box->SetBoxExtent(meshComponent->GetStaticMesh()->GetBounds().BoxExtent + FVector(50, 0, 50), true);
}