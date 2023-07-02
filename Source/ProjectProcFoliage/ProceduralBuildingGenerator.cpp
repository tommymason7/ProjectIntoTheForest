// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralBuildingGenerator.h"

AProceduralBuildingGenerator::AProceduralBuildingGenerator()
{
	floorComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Floors"));
}

void AProceduralBuildingGenerator::BeginPlay()
{
	

	// Initial fill in Grid array
	for (int x = 0; x < numOfXCells; x++)
	{
		TArray<TSharedPtr<GridInfo>> inner;
		for (int y = 0; y < numOfYCells; y++)
		{
			TSharedPtr<GridInfo> gridCell = MakeShared<GridInfo>();
			
			gridCell->walls.Add(Direction::BOTTOM, MakeShared<SubGridInfo>());
			gridCell->walls.Add(Direction::TOP, MakeShared<SubGridInfo>());
			gridCell->walls.Add(Direction::RIGHT, MakeShared<SubGridInfo>());
			gridCell->walls.Add(Direction::LEFT, MakeShared<SubGridInfo>());

			inner.Add(gridCell);
		}
		grid.Add(inner);
	}

	Generate();
}

void AProceduralBuildingGenerator::Generate()
{
	//int x;
	//int y;

	// Generating the rooms/hallways

	// ASSUMPTION: There is only one entry/exit for every building
	// 
	// Need to pick an initial point for entry
	bool xIsMinOrMax = FMath::RandBool();
	int doorX;
	int doorY;
	FVector doorOffset;
	Direction dir;
	float rotation = 0;

	if (xIsMinOrMax)
	{
		doorX = FMath::RandBool() ? 0 : numOfXCells - 1;
		doorY = FMath::RandHelper(numOfYCells);

		if (doorX == 0)
		{
			// Bottom facing
			doorOffset.X = 0;
			doorOffset.Y = 0;
			doorOffset.Z = 0;
			dir = Direction::BOTTOM;
		}
		else
		{
			// Top Facing
			doorOffset.X = 400;
			doorOffset.Y = 0;
			doorOffset.Z = 0;
			dir = Direction::TOP;
		}

		rotation = 90;
	}
	else
	{
		doorX = FMath::RandHelper(numOfXCells);
		doorY = FMath::RandBool() ? 0 : numOfYCells - 1;

		if (doorY == 0)
		{
			// Left Facing
			doorOffset.X = 0;
			doorOffset.Y = 0;
			doorOffset.Z = 0;
			dir = Direction::LEFT;
		}
		else
		{
			// Right Facing
			doorOffset.X = 0;
			doorOffset.Y = 400;
			doorOffset.Z = 0;
			dir = Direction::RIGHT;
		}
	}

	// Set what side of the grid the door is on
	auto doorIndex = grid[doorX][doorY]->walls.Add(dir, MakeShared<SubGridInfo>());
	doorIndex->offset = doorOffset;
	doorIndex->zRotation = rotation;
	doorIndex->type.RemoveAll([](MeshType opt) {
		// Removes all entries that meet this condition
		return opt != MeshType::DOORWAY;
	});

	// Need to determine the walls for the rest of this grid cell and to repeat process for the rest of the grid
	// All other outer walls should be set at this point
	for (int x = 0; x < numOfXCells; x++)
	{
		for (int y = 0; y < numOfYCells; y++)
		{
			if (x == doorX && y == doorY)
			{
				continue;
			}

			// Bottom non corners
			if (x != 0 && x != numOfXCells - 1 && y == 0)
			{
				grid[x][0]->walls[Direction::BOTTOM]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[x][0]->walls[Direction::BOTTOM]->offset.X = 0;
				grid[x][0]->walls[Direction::BOTTOM]->zRotation = 90;

				UE_LOG(LogTemp, Warning, TEXT("BOTTOM NON CORNER x: %d y: %d"), x, y);
			}
			// Top non corners
			else if (x != 0 && x != numOfXCells - 1 && y == numOfYCells - 1)
			{
				grid[x][numOfYCells - 1]->walls[Direction::TOP]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[x][numOfYCells - 1]->walls[Direction::TOP]->offset.X = 400;
				grid[x][numOfYCells - 1]->walls[Direction::TOP]->zRotation = 90;

				UE_LOG(LogTemp, Warning, TEXT("TOP NON CORNER x: %d y: %d"), x, y);
			}
			// Left non corners
			else if (y != 0 && y != numOfYCells - 1 && x == 0)
			{
				grid[0][y]->walls[Direction::LEFT]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[0][y]->walls[Direction::LEFT]->offset.Y = 0;
				grid[0][y]->walls[Direction::LEFT]->zRotation = 0;

				UE_LOG(LogTemp, Warning, TEXT("LEFT NON CORNER x: %d y: %d"), x, y);
			}
			// Right non corners
			else if (y != 0 && y != numOfYCells - 1 && x == numOfXCells - 1)
			{
				grid[numOfXCells - 1][y]->walls[Direction::RIGHT]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[numOfXCells - 1][y]->walls[Direction::RIGHT]->offset.Y = 400;
				grid[numOfXCells - 1][y]->walls[Direction::RIGHT]->zRotation = 0;

				UE_LOG(LogTemp, Warning, TEXT("RIGHT NON CORNER x: %d y: %d"), x, y);
			}
			// Bottom Left Corner
			else if (x == 0 && y == 0)
			{
				grid[x][y]->walls[Direction::LEFT]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[x][y]->walls[Direction::LEFT]->offset.Y = 0;
				grid[x][y]->walls[Direction::LEFT]->zRotation = 0;

				grid[x][y]->walls[Direction::BOTTOM]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[x][y]->walls[Direction::BOTTOM]->offset.X = 0;
				grid[x][y]->walls[Direction::BOTTOM]->zRotation = 90;

				UE_LOG(LogTemp, Warning, TEXT("BOTTOM LEFT CORNER x: %d y: %d"), x, y);
			}
			// Bottom Right Corner
			else if (x == numOfXCells - 1 && y == 0)
			{
				grid[x][y]->walls[Direction::RIGHT]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[x][y]->walls[Direction::RIGHT]->offset.Y = 400;
				grid[x][y]->walls[Direction::RIGHT]->zRotation = 0;


				grid[x][y]->walls[Direction::BOTTOM]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[x][y]->walls[Direction::BOTTOM]->offset.X = 0;
				grid[x][y]->walls[Direction::BOTTOM]->zRotation = 90;

				UE_LOG(LogTemp, Warning, TEXT("BOTTOM RIGHT CORNER x: %d y: %d"), x, y);
			}
			// Top Left Corner
			else if (x == 0 && y == numOfYCells - 1)
			{
				grid[x][y]->walls[Direction::LEFT]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[x][y]->walls[Direction::LEFT]->offset.Y = 0;
				grid[x][y]->walls[Direction::LEFT]->zRotation = 0;

				grid[x][y]->walls[Direction::TOP]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[x][y]->walls[Direction::TOP]->offset.X = 400;
				grid[x][y]->walls[Direction::TOP]->zRotation = 90;

				UE_LOG(LogTemp, Warning, TEXT("TOP LEFT CORNER x: %d y: %d"), x, y);
			}
			// Top Right Corner
			else if (x == numOfXCells - 1 && y == numOfYCells - 1)
			{
				grid[x][y]->walls[Direction::RIGHT]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[x][y]->walls[Direction::RIGHT]->offset.Y = 400;
				grid[x][y]->walls[Direction::RIGHT]->zRotation = 0;

				grid[x][y]->walls[Direction::TOP]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[x][y]->walls[Direction::TOP]->offset.X = 400;
				grid[x][y]->walls[Direction::TOP]->zRotation = 90;

				UE_LOG(LogTemp, Warning, TEXT("TOP RIGHT CORNER x: %d y: %d"), x, y);
			}
		}
	}

	// Add the points around the initial point to determine what gets generated next
	if (doorX - 1 >= 0)
	{
		_gridGenerationQueue.Add(TPair<int, int>(doorX - 1, doorY));
	}

	if (doorX + 1 < numOfXCells)
	{
		_gridGenerationQueue.Add(TPair<int, int>(doorX + 1, doorY));
	}

	if (doorY - 1 >= 0)
	{
		_gridGenerationQueue.Add(TPair<int, int>(doorX, doorY - 1));
	}

	if (doorY + 1 < numOfYCells)
	{
		_gridGenerationQueue.Add(TPair<int, int>(doorX, doorY + 1));
	}

	GenerateBuilding();

	SpawnBuilding();
}

// Only called after the entrance to the building is set
void AProceduralBuildingGenerator::GenerateBuilding()
{
	if (!_gridGenerationQueue.IsEmpty())
	{
		// Look in queue for next point to start spawn from
		TPair<int, int> xyPair = *_gridGenerationQueue.begin();
		TSharedPtr<GridInfo> gridInfo = grid[xyPair.Key][xyPair.Value];

		// Get valid options and choose one for each direction
		
		// Need check on x and y to verify the mesh can be set there for each if
		if (gridInfo->walls[Direction::BOTTOM]->type.Num() > 1)
		{
			int meshIndex = FMath::RandHelper(gridInfo->walls[Direction::BOTTOM]->type.Num());
			
			FillInGrid(xyPair.Key, xyPair.Value, gridInfo->walls[Direction::BOTTOM]->type[meshIndex], Direction::BOTTOM);
		}

		if (gridInfo->walls[Direction::LEFT]->type.Num() > 1)
		{
			int meshIndex = FMath::RandHelper(gridInfo->walls[Direction::LEFT]->type.Num());

			FillInGrid(xyPair.Key, xyPair.Value, gridInfo->walls[Direction::LEFT]->type[meshIndex], Direction::LEFT);
		}

		if (gridInfo->walls[Direction::RIGHT]->type.Num() > 1)
		{
			int meshIndex = FMath::RandHelper(gridInfo->walls[Direction::RIGHT]->type.Num());

			FillInGrid(xyPair.Key, xyPair.Value, gridInfo->walls[Direction::RIGHT]->type[meshIndex], Direction::RIGHT);
		}

		if (gridInfo->walls[Direction::TOP]->type.Num() > 1)
		{
			int meshIndex = FMath::RandHelper(gridInfo->walls[Direction::TOP]->type.Num());

			FillInGrid(xyPair.Key, xyPair.Value, gridInfo->walls[Direction::TOP]->type[meshIndex], Direction::TOP);
		}

		_gridGenerationQueue.Remove(xyPair);
	}
	else
	{
		// Queue is empty but we have unfilled cells

		// Find unfilled cell
		TTuple<int, int> xyPair;
		bool result = GetNextUnfilledCell(xyPair);

		// If we looped through all options then we don't want to execute this
		if (result)
		{
			// Get random option from types left and a valid direction
			TSharedPtr<GridInfo> gridInfo = grid[xyPair.Key][xyPair.Value];

			// Get valid options and choose one for each direction
			if (gridInfo->walls[Direction::BOTTOM]->type.Num() > 1)
			{
				int meshIndex = FMath::RandHelper(gridInfo->walls[Direction::BOTTOM]->type.Num());

				FillInGrid(xyPair.Key, xyPair.Value, gridInfo->walls[Direction::BOTTOM]->type[meshIndex], Direction::BOTTOM);
			}

			if (gridInfo->walls[Direction::LEFT]->type.Num() > 1)
			{
				int meshIndex = FMath::RandHelper(gridInfo->walls[Direction::LEFT]->type.Num());

				FillInGrid(xyPair.Key, xyPair.Value, gridInfo->walls[Direction::LEFT]->type[meshIndex], Direction::LEFT);
			}

			if (gridInfo->walls[Direction::RIGHT]->type.Num() > 1)
			{
				int meshIndex = FMath::RandHelper(gridInfo->walls[Direction::RIGHT]->type.Num());

				FillInGrid(xyPair.Key, xyPair.Value, gridInfo->walls[Direction::RIGHT]->type[meshIndex], Direction::RIGHT);
			}

			if (gridInfo->walls[Direction::TOP]->type.Num() > 1)
			{
				int meshIndex = FMath::RandHelper(gridInfo->walls[Direction::TOP]->type.Num());

				FillInGrid(xyPair.Key, xyPair.Value, gridInfo->walls[Direction::TOP]->type[meshIndex], Direction::TOP);
			}
		}
	}

	// Check that all locations has a single item set
	if (IsGridFilled())
	{
		return;
	}
	else
	{
		return GenerateBuilding();
	}
}

void AProceduralBuildingGenerator::FillInGrid(int x, int y, MeshType meshType, Direction dir)
{
	// Remove all other mesh types
	grid[x][y]->walls[dir]->type.RemoveAll([meshType](MeshType opt) {
		return meshType != opt;
	});

	switch (dir)
	{
	  case Direction::BOTTOM:
		  grid[x][y]->walls[dir]->offset.X = 0;
		  grid[x][y]->walls[dir]->zRotation = 90;
	    break;
	  case Direction::TOP:
		  grid[x][y]->walls[dir]->offset.X = 400;
		  grid[x][y]->walls[dir]->zRotation = 90;
		break;
	  case Direction::LEFT:
		  grid[x][y]->walls[dir]->offset.Y = 0;
		  grid[x][y]->walls[dir]->zRotation = 0;
		break;
	  case Direction::RIGHT:
		  grid[x][y]->walls[dir]->offset.Y = 400;
		  grid[x][y]->walls[dir]->zRotation = 0;
		break;
	  default:
		break;

	}

	// If there are restrictions for options of MeshType based on previously selected types this would be where we update it

	// Queue up options around
	if (x - 1 >= 0 &&
	    (grid[x - 1][y]->walls[Direction::BOTTOM]->type.Num() > 1 ||
		 grid[x - 1][y]->walls[Direction::TOP]->type.Num() > 1 || 
		 grid[x - 1][y]->walls[Direction::RIGHT]->type.Num() > 1 ||
		 grid[x - 1][y]->walls[Direction::LEFT]->type.Num() > 1))
	{
		_gridGenerationQueue.Add(TPair<int, int>(x - 1, y));
	}

	if (x + 1 < numOfXCells &&
		(grid[x + 1][y]->walls[Direction::BOTTOM]->type.Num() > 1 ||
		 grid[x + 1][y]->walls[Direction::TOP]->type.Num() > 1 ||
		 grid[x + 1][y]->walls[Direction::RIGHT]->type.Num() > 1 ||
		 grid[x + 1][y]->walls[Direction::LEFT]->type.Num() > 1))
	{
		_gridGenerationQueue.Add(TPair<int, int>(x + 1, y));
	}

	if (y - 1 >= 0 &&
		(grid[x][y - 1]->walls[Direction::BOTTOM]->type.Num() > 1 ||
		 grid[x][y - 1]->walls[Direction::TOP]->type.Num() > 1 ||
		 grid[x][y - 1]->walls[Direction::RIGHT]->type.Num() > 1 ||
		 grid[x][y - 1]->walls[Direction::LEFT]->type.Num() > 1))
	{
		_gridGenerationQueue.Add(TPair<int, int>(x, y - 1));
	}

	if (y + 1 < numOfYCells &&
		(grid[x][y + 1]->walls[Direction::BOTTOM]->type.Num() > 1 ||
		 grid[x][y + 1]->walls[Direction::TOP]->type.Num() > 1 ||
		 grid[x][y + 1]->walls[Direction::RIGHT]->type.Num() > 1 ||
		 grid[x][y + 1]->walls[Direction::LEFT]->type.Num() > 1))
	{
		_gridGenerationQueue.Add(TPair<int, int>(x, y + 1));
	}
}

void AProceduralBuildingGenerator::SpawnBuilding()
{
	for (int x = 0; x < numOfXCells; x++)
	{
		for (int y = 0; y < numOfYCells; y++)
		{
			// TODO: 400 is the assumed size of the mesh, we could just sample the mesh here to get the size.
			//       The mesh would have top be set as to what is selected further above.
			FTransform trans(FVector(x*400, y*400, GetActorLocation().Z));

			// Spawn the floor
			floorComponent->AddInstance(trans);

			auto bottomInfo = grid[x][y]->walls[Direction::BOTTOM];
			auto topInfo = grid[x][y]->walls[Direction::TOP];
			auto leftInfo = grid[x][y]->walls[Direction::LEFT];
			auto rightInfo = grid[x][y]->walls[Direction::RIGHT];

			SpawnWallDoor(x, y, Direction::BOTTOM, bottomInfo);
			SpawnWallDoor(x, y, Direction::TOP, topInfo);
			SpawnWallDoor(x, y, Direction::LEFT, leftInfo);
			SpawnWallDoor(x, y, Direction::RIGHT, rightInfo);
		}
	}
}

void AProceduralBuildingGenerator::SpawnWallDoor(int x, int y, Direction dir, TSharedPtr<SubGridInfo> info)
{
	// Spawn Walls and Doors
	// Need to be able to set the walls and doors in BP
	// Just use a table or set variables up

	// Verify that there is only one option otherwise spawn nothing
	if (info->type.Num() == 1)
	{
		FVector spawnLocation = floorComponent->GetComponentLocation() + FVector(x * 400, y * 400, GetActorLocation().Z) + info->offset;
		switch (info->type[0])
		{
			case MeshType::DOORWAY:
			{
				AStaticMeshActor* spawned = GetWorld()->SpawnActor<AStaticMeshActor>(spawnLocation, FRotator(0, info->zRotation, 0));
				if (spawned)
				{
					spawned->GetStaticMeshComponent()->SetStaticMesh(doorOption);
				}

				break;
			}
			case MeshType::WALL:
			{
				AStaticMeshActor* spawned = GetWorld()->SpawnActor<AStaticMeshActor>(spawnLocation, FRotator(0, info->zRotation, 0));
				if (spawned)
				{
					spawned->GetStaticMeshComponent()->SetStaticMesh(wallOption);
				}
				break;
			}
			case MeshType::NONE:
			{
				break;
			}
		}
	}
}

bool AProceduralBuildingGenerator::GetNextUnfilledCell(TTuple<int, int>& xyPair)
{
	for (int x = 0; x < numOfXCells; x++)
	{
		for (int y = 0; y < numOfYCells; y++)
		{
			if (grid[x][y]->walls[Direction::BOTTOM]->type.Num() > 1 ||
				grid[x][y]->walls[Direction::TOP]->type.Num() > 1 ||
				grid[x][y]->walls[Direction::LEFT]->type.Num() > 1 ||
				grid[x][y]->walls[Direction::RIGHT]->type.Num() > 1)
			{
				xyPair = TTuple<int, int>(x, y);
				return true;
			}
		}
	}

	return false;
}


bool AProceduralBuildingGenerator::IsGridFilled()
{
	for (int x = 0; x < grid.Num(); x++)
	{
		for (int y = 0; y < grid[x].Num(); y++)
		{
			//			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("-------- grid x: %d grid y: %d"), x, y));
			if (grid[x][y]->walls[Direction::BOTTOM]->type.Num() > 1)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Grid isn't full found option at x: %d y: %d"), x, y));
				return false;
			}

			if (grid[x][y]->walls[Direction::TOP]->type.Num() > 1)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Grid isn't full found option at x: %d y: %d"), x, y));
				return false;
			}

			if (grid[x][y]->walls[Direction::RIGHT]->type.Num() > 1)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Grid isn't full found option at x: %d y: %d"), x, y));
				return false;
			}

			if (grid[x][y]->walls[Direction::LEFT]->type.Num() > 1)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Grid isn't full found option at x: %d y: %d"), x, y));
				return false;
			}
		}
	}

	return true;
}