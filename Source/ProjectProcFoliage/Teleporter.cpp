// Fill out your copyright notice in the Description page of Project Settings.


#include "Teleporter.h"

ATeleporter::ATeleporter()
{
	// Create subobject for mesh and attach to root
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	Box->SetupAttachment(meshComponent);
}

void ATeleporter::Activate()
{

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