// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/KismetMathLibrary.h"
#include "Landscape.h"

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

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBuildingGenerationFinished);

/**
 * 
 */
UCLASS()
class PROJECTPROCFOLIAGE_API AProceduralBuildingGenerator : public AActor
{
	GENERATED_BODY()

	struct RoomInfo
	{
		int xMin;
		int xMax;
		int yMin;
		int yMax;

		TSet<TPair<int, int>> gridCellsInRoom;

		// Door ways
		//TSet<TTuple<int, int>> doorwayCoordinates;
		TMap<TTuple<int, int>, TSharedPtr<RoomInfo>> roomConnections;

		// Track Story elements that are spawned in to prevent duplicate spawns
	};

	struct SubGridInfo
	{
		TArray<MeshType> type = {MeshType::WALL, MeshType::DOORWAY, MeshType::NONE};
		UStaticMesh* mesh = nullptr;
		FVector offset; // Offset from pivot of floor. Offset gets rotated when it needs to be
		float zRotation = 0.0;
	};

	struct GridInfo
	{
		// Walls per grid info
		TMap<Direction, TSharedPtr<SubGridInfo>> walls;
		
		// Room that grid is apart of
		TSharedPtr<RoomInfo> room;
	};

public:
	AProceduralBuildingGenerator();

	void Generate();
	void Spawn();

	UFUNCTION(BlueprintCallable)
	void setNumOfXCells(int xCells);

	UFUNCTION(BlueprintCallable)
	void setNumOfYCells(int yCells);

	UFUNCTION(BlueprintCallable)
    void initialize();

	UFUNCTION(BlueprintCallable)
	void setWallOption(UStaticMesh* option);

	UFUNCTION(BlueprintCallable)
	void setDoorOption(UStaticMesh* option);

	UFUNCTION(BlueprintCallable)
	void setFloorOption(UStaticMesh* option);

	float getBuildingMinX();
	float getBuildingMaxX();
	float getBuildingMinY();
	float getBuildingMaxY();

	UPROPERTY(BlueprintAssignable)
	FBuildingGenerationFinished generationFinishedCallback;

	bool isGenerationFinished();

	FVector getOrigin();

protected:

	virtual void BeginPlay() override;

private:

	void GenerateBuilding();
	bool GetNextUnfilledCell(TTuple<int, int>& xyPair);
	void FillInGrid(int x, int y, MeshType meshType, Direction dir);
	bool IsGridFilled();
	void SpawnBuilding();
	void SpawnWallDoor(int x, int y, Direction dir, TSharedPtr<SubGridInfo> info);

	void GenerateRoomInfo();
	void RoomDetection(bool initialPick);
	void RoomInformation(int x, int y, TSharedPtr<RoomInfo> room);
	bool GetNextCellUnassignedToRoom(TPair<int, int> &pair);


	// Each cell is 400x400.
	// Keep in mind where we are calculating from
	// We could actually have the size of the cells vary based on the mesh selected but more details would have to be known at generation
	TArray<TArray<TSharedPtr<GridInfo>>> grid;

	TArray<TSharedPtr<RoomInfo>> rooms;

	TArray<TPair<int, int>> _gridGenerationQueue;

	TSet<TPair<int, int>> _cellsAssignedToARoom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Cells, meta = (AllowPrivateAccess = "true"))
	int numOfXCells = 2;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Cells , meta = (AllowPrivateAccess = "true"))
	int numOfYCells = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UInstancedStaticMeshComponent* floorComponent = nullptr;

	UInstancedStaticMeshComponent* wallComponent = nullptr;
	UInstancedStaticMeshComponent* doorComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ArchitectureOptions, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* wallOption = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ArchitectureOptions, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* doorOption = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ArchitectureOptions, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* floorOption = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = ArchitectureOptions, meta = (AllowPrivateAccess = "true"))
	UStaticMesh* exclusionObject = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* _volume = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = LineTrace, meta = (AllowPrivateAccess = "true"))
	TSet<AActor*> _actorsToIgnoreForLineTrace;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = LineTrace, meta = (AllowPrivateAccess = "true"))
	float originZOffset = 10;

	int xCellsSubgrid;
	int yCellsSubgrid;

	bool generationFinished = false;

	FVector origin;
};
