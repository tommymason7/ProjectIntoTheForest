// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "Components/InstancedStaticMeshComponent.h"

#include "FoliageInstance.generated.h"

UCLASS()
class PROJECTPROCFOLIAGE_API AFoliageInstance : public AActor
{
	GENERATED_BODY()

	UInstancedStaticMeshComponent* _mesh = nullptr;
	
public:	
	// Sets default values for this actor's properties
	AFoliageInstance();

	void AddInstance(FTransform location, bool worldSpace);
	void SetMesh(UStaticMesh* mesh);


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
