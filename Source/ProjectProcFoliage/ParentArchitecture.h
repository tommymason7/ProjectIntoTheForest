// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ParentArchitecture.generated.h"

UCLASS()
class PROJECTPROCFOLIAGE_API AParentArchitecture : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = StaticMesh, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* mesh = nullptr;

public:	
	// Sets default values for this actor's properties
	AParentArchitecture();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void setMesh(UStaticMesh* newMesh);

// Variables
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = StaticMesh, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* meshComponent = nullptr;
};
