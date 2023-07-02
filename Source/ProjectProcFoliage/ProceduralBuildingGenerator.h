// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

#include "ProceduralBuildingGenerator.generated.h"


UENUM()
enum Direction
{
	TOP,
	BOTTOM,
	LEFT,
	RIGHT
};

UENUM()
enum MeshType
{
	WALL,
	DOORWAY,
	NONE
};

/**
 * 
 */
UCLASS()
class PROJECTPROCFOLIAGE_API AProceduralBuildingGenerator : public AActor
{
	GENERATED_BODY()

	struct SubGridInfo
	{
		TArray<MeshType> type = {MeshType::WALL, MeshType::DOORWAY, MeshType::NONE};
		UStaticMesh* mesh = nullptr;
		FVector offset; // Offset from pivot of floor. Offset gets rotated when it needs to be
		float zRotation = 0.0;
	};

	struct GridInfo
	{
		// Need to store at least 3 options for a basic room
		// TODO need to assoicate the 4 wall locations with a wall option
		//  Aka. Left right top down need to know if there is a wall there or not. This is going to be determined by how big the room is
		TMap<Direction, TSharedPtr<SubGridInfo>> walls;

		// I don't think this is really should just be a repeat of whats in walls
		//TArray<SubGridInfo> doorWays;
		
		// Info for subgrid of grid
	};

public:
	AProceduralBuildingGenerator();

	void Generate();
	void Spawn();

protected:

	virtual void BeginPlay() override;

private:

	void GenerateBuilding();
	bool GetNextUnfilledCell(TTuple<int, int>& xyPair);
	void FillInGrid(int x, int y, MeshType meshType, Direction dir);
	bool IsGridFilled();
	void SpawnBuilding();
	void SpawnWallDoor(int x, int y, Direction dir, TSharedPtr<SubGridInfo> info);

	// Each cell is 400x400.
	// Keep in mind where we are calculating from
	// We could actually have the size of the cells vary based on the mesh selected but more details would have to be known at generation
	TArray<TArray<TSharedPtr<GridInfo>>> grid;

	TArray<TPair<int, int>> _gridGenerationQueue;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Cells, meta = (AllowPrivateAccess = "true"))
	int numOfXCells;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Cells , meta = (AllowPrivateAccess = "true"))
	int numOfYCells;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UInstancedStaticMeshComponent* floorComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ArchitectureOptions, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* wallOption = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ArchitectureOptions, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* doorOption = nullptr;

	int xCellsSubgrid;
	int yCellsSubgrid;

};
