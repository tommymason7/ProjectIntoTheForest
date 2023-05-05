// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Foliage/Public/InstancedFoliageActor.h"
#include "Components/BoxComponent.h"

#include "FoliageInstance.h"
#include "Orb.h"
#include "ParentArchitecture.h"
#include "Teleporter.h"

#include "ProceduralFoliageGenerator.generated.h"

UCLASS()
class PROJECTPROCFOLIAGE_API AProceduralFoliageGenerator : public AActor
{
	GENERATED_BODY()

	enum GridOption
	{
		None,
		Architecture,
		Foliage
	};

	struct GridInfo
	{
		TArray<GridOption> possibilities = { GridOption::None, GridOption::Architecture, GridOption::Foliage };
		UStaticMesh* selectedMesh = nullptr;
		bool rootLocation = false;
		bool initialPick = false;
	};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* _volume = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	int numOfXCells = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	int numOfYCells = 10;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	int numOfOrbsToSpawn = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	float percentageOfOrbsNeeded = 0.5;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	TArray<UStaticMesh*> foliageOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	TArray<UStaticMesh*> architectureOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* teleporterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* orbMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	UCurveFloat* orbMovementCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AOrb> orbClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	FName TeleporterActiveMaterialSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* TeleporterActiveMaterial;

	// Terrain Grid
	TArray<TArray<TSharedPtr<GridInfo>>> grid;

	float xSeperationAmnt;
	float ySeperationAmnt;
	FVector boxExtent;

	TArray<TPair<int, int>> _gridGenerationQueue;


	// Track different meshes spawning
	TMap<UStaticMesh*, AFoliageInstance*> _meshToActor;

	ATeleporter* teleporter = nullptr;
	int numOfOrbsCollectedThisLevel = 0;
	int orbSpawnedCounter = 0;

	
public:	
	// Sets default values for this actor's properties
	AProceduralFoliageGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void GenerateTerrainGrid(bool initialPick);

	void FillInGrid(int x, int y, GridOption option, bool initialPick);

	bool IsGridFilled();

	void SpawnGrid();

	bool GetNextUnfilledCell(TTuple<int, int>& xyPair);

	UFUNCTION()
	void ForwardOrbCollected();

};
