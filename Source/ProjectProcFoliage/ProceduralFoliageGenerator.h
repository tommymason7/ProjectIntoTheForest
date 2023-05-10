// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Foliage/Public/InstancedFoliageActor.h"
#include "Components/BoxComponent.h"

#include "FoliageInstance.h"
#include "GameMode_Procedural.h"
#include "Orb.h"
#include "ParentArchitecture.h"
#include "StartingManager.h"
#include "Teleporter.h"

#include "ProceduralFoliageGenerator.generated.h"

UENUM()
enum GridOption
{
	None,
	Architecture,
	Foliage
};

USTRUCT(BlueprintType)
struct FMinMax
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float min = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float max = 0.0f;
};


UCLASS()
class PROJECTPROCFOLIAGE_API AProceduralFoliageGenerator : public AActor
{
	GENERATED_BODY()

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
	FName TeleporterActiveMaterialSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* TeleporterActiveMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	TSoftObjectPtr<UWorld> LevelToLoad;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	AStartingManager* starterManager;

	// This allows the user to control whats allowed to spawn via types and chance
	// The pair refers to the Minimum as the Key and the Maximum as the value. Ex: (0.1, 0.4)
	// The pair minimum value is 0 and the maximum value 1
	// Each entry should have their own range to allow just grabbing a random float from 0 to 1 and determine 
	//  if we have successfully gotten the correct value for said type
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	TMap<TEnumAsByte<GridOption>, FMinMax> typeToMinMax;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	TMap<UStaticMesh*, FMinMax> architectureSpawnChance;

	// Terrain Grid
	TArray<TArray<TSharedPtr<GridInfo>>> grid;

	float xSeperationAmnt;
	float ySeperationAmnt;
	FVector boxExtent;

	TArray<TPair<int, int>> _gridGenerationQueue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Procedural, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class ACharacter> enemyClass;

	// Track if Enemy was spawned
	UPROPERTY()
	ACharacter* enemy = nullptr;


	// Track different meshes spawning
	UPROPERTY()
	TMap<UStaticMesh*, AFoliageInstance*> _meshToActor;

	UPROPERTY()
	ATeleporter* teleporter = nullptr;
	int numOfOrbsCollectedThisLevel = 0;
	int orbSpawnedCounter = 0;

	
public:	
	// Sets default values for this actor's properties
	AProceduralFoliageGenerator();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void PlayerTeleporting();

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

	//TODO Could template this function with getArchitectureFromChance
	GridOption getOptionFromChance(float randNum);

	UStaticMesh* getArchitectureFromChance(float randNum);
};
