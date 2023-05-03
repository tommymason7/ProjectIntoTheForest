// Fill out your copyright notice in the Description page of Project Settings.


#include "ParentArchitecture.h"

// Sets default values
AParentArchitecture::AParentArchitecture()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create subobject for mesh and attach to root
	meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	meshComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AParentArchitecture::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AParentArchitecture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AParentArchitecture::setMesh(UStaticMesh* newMesh)
{
	meshComponent->SetStaticMesh(newMesh);
}