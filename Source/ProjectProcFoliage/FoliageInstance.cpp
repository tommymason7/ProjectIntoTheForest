// Fill out your copyright notice in the Description page of Project Settings.


#include "FoliageInstance.h"

// Sets default values
AFoliageInstance::AFoliageInstance()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_mesh = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Mesh"));
	_mesh->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AFoliageInstance::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFoliageInstance::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFoliageInstance::AddInstance(FTransform transform, bool worldSpace)
{
	_mesh->AddInstance(transform, worldSpace);
}

void AFoliageInstance::SetMesh(UStaticMesh* mesh)
{
	_mesh->SetStaticMesh(mesh);
}

