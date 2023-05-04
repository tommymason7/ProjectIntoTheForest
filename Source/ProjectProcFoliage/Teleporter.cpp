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
}