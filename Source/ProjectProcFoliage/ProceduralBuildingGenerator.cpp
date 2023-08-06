// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralBuildingGenerator.h"

AProceduralBuildingGenerator::AProceduralBuildingGenerator()
{
	floorComponent = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Floors"));

	_volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	_volume->SetupAttachment(RootComponent);
}

void AProceduralBuildingGenerator::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add("Building");
}

void AProceduralBuildingGenerator::initialize()
{
	// Create floor component
	floorComponent = NewObject<UInstancedStaticMeshComponent>(this);
	floorComponent->RegisterComponent();

	floorComponent->SetStaticMesh(floorOption);

	AddInstanceComponent(floorComponent);

	// Create Door component
	doorComponent = NewObject<UInstancedStaticMeshComponent>(this);
	doorComponent->RegisterComponent();
	doorComponent->SetStaticMesh(doorOption);

	AddInstanceComponent(doorComponent);

	// Create Wall component
	wallComponent = NewObject<UInstancedStaticMeshComponent>(this);
	wallComponent->RegisterComponent();
	wallComponent->SetStaticMesh(wallOption);

	AddInstanceComponent(wallComponent);

	// Create Ceiling component
	ceilingComponent = NewObject<UInstancedStaticMeshComponent>(this);
	ceilingComponent->RegisterComponent();
	ceilingComponent->SetStaticMesh(ceilingOption);

	AddInstanceComponent(ceilingComponent);

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
	UE_LOG(LogTemp, Warning, TEXT("Generate about to be called in initalize. numofXCells: %d numofYCells: %d"), numOfXCells, numOfYCells);

	// Pick random spot in volume to get origin location
	if (_volume)
	{
		auto o = _volume->Bounds.Origin;
		auto boxExtent = _volume->Bounds.BoxExtent;
		FVector randomLoc = UKismetMathLibrary::RandomPointInBoundingBox(o, boxExtent);
		FVector startingPoint(randomLoc.X, randomLoc.Y, o.Z + boxExtent.Z);
		FVector endingPoint = startingPoint - FVector(0, 0, o.Z + boxExtent.Z);

		// Line trace to Landscape
		FHitResult result;
		FCollisionObjectQueryParams params(ECollisionChannel::ECC_WorldStatic);
		FCollisionQueryParams colParams;
		colParams.AddIgnoredActors(_actorsToIgnoreForLineTrace.Array());
		GetWorld()->LineTraceSingleByObjectType(result, startingPoint, endingPoint, params, colParams);
		//DrawDebugLine(GetWorld(), startingPoint, endingPoint, FColor::Cyan, true);

		if (result.bBlockingHit)
		{
			// Verify that we hit land
			AActor* actor = result.GetActor();
			ALandscape* land = Cast<ALandscape>(actor);

			if (land)
			{
				origin = result.Location + FVector(0, 0, originZOffset);
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Setting origin z: %f"), origin.Z));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("HITTING SOMETHING ELSE OTHER THEN LAND: %s"), *actor->GetName()));
			}
		}

	}

	Generate();
}

void AProceduralBuildingGenerator::Generate()
{
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

	UE_LOG(LogTemp, Warning, TEXT("Outside Door Cell x: %d y: %d with direction: %s" ), doorX, doorY, *UEnum::GetDisplayValueAsText(dir).ToString());


	// Set what side of the grid the door is on
	grid[doorX][doorY]->walls[dir]->offset = doorOffset;
	grid[doorX][doorY]->walls[dir]->zRotation = rotation;
	grid[doorX][doorY]->walls[dir]->type.RemoveAll([](MeshType opt) {
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
				// Detect if it's a corner slot

				// TODO: Breaks for 1x1 buildings

				// Bottom Left Corner
				if (x == 0 && y == 0)
				{
					if (dir == Direction::LEFT)
					{
						grid[x][y]->walls[Direction::BOTTOM]->type.RemoveAll([](MeshType opt) {
							return opt != MeshType::WALL;
						});
						grid[x][y]->walls[Direction::BOTTOM]->offset.X = 0;
						grid[x][y]->walls[Direction::BOTTOM]->zRotation = 90;
					}
					else if (dir == Direction::BOTTOM)
					{
						grid[x][y]->walls[Direction::LEFT]->type.RemoveAll([](MeshType opt) {
							return opt != MeshType::WALL;
							});
						grid[x][y]->walls[Direction::LEFT]->offset.Y = 0;
						grid[x][y]->walls[Direction::LEFT]->zRotation = 0;
					}
				}
				// Top Left Corner
				else if (x == numOfXCells - 1 && y == 0)
				{
					if (dir == Direction::LEFT)
					{
						grid[x][y]->walls[Direction::TOP]->type.RemoveAll([](MeshType opt) {
							return opt != MeshType::WALL;
						});
						grid[x][y]->walls[Direction::TOP]->offset.X = 400;
						grid[x][y]->walls[Direction::TOP]->zRotation = 90;
					}
					else if (dir == Direction::TOP)
					{
						grid[x][y]->walls[Direction::LEFT]->type.RemoveAll([](MeshType opt) {
							return opt != MeshType::WALL;
							});
						grid[x][y]->walls[Direction::LEFT]->offset.Y = 0;
						grid[x][y]->walls[Direction::LEFT]->zRotation = 0;
					}

				}
				// Bottom Right Corner
				else if (x == 0 && y == numOfYCells - 1)
				{
					if (dir == Direction::RIGHT)
					{
						grid[x][y]->walls[Direction::BOTTOM]->type.RemoveAll([](MeshType opt) {
							return opt != MeshType::WALL;
						});
						grid[x][y]->walls[Direction::BOTTOM]->offset.X = 0;
						grid[x][y]->walls[Direction::BOTTOM]->zRotation = 90;
					}
					else if(dir == Direction::BOTTOM)
					{
						grid[x][y]->walls[Direction::RIGHT]->type.RemoveAll([](MeshType opt) {
							return opt != MeshType::WALL;
						});
						grid[x][y]->walls[Direction::RIGHT]->offset.Y = 400;
						grid[x][y]->walls[Direction::RIGHT]->zRotation = 0;
					}
				}
				// Top Right Corner
				else if (x == numOfXCells - 1 && y == numOfYCells - 1)
				{
					if (dir == Direction::RIGHT)
					{
						grid[x][y]->walls[Direction::TOP]->type.RemoveAll([](MeshType opt) {
							return opt != MeshType::WALL;
						});
						grid[x][y]->walls[Direction::TOP]->offset.X = 400;
						grid[x][y]->walls[Direction::TOP]->zRotation = 90;
					}
					else if (dir == Direction::TOP)
					{
						grid[x][y]->walls[Direction::RIGHT]->type.RemoveAll([](MeshType opt) {
							return opt != MeshType::WALL;
						});
						grid[x][y]->walls[Direction::RIGHT]->offset.Y = 400;
						grid[x][y]->walls[Direction::RIGHT]->zRotation = 0;
					}
				}
				continue;
			}

			// Left non corners
			if (x != 0 && x != numOfXCells - 1 && y == 0)
			{
				grid[x][0]->walls[Direction::LEFT]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[x][0]->walls[Direction::LEFT]->offset.Y = 0;
				grid[x][0]->walls[Direction::LEFT]->zRotation = 0;

				UE_LOG(LogTemp, Warning, TEXT("LEFT NON CORNER x: %d y: %d"), x, y);
			}
			// Right non corners
			else if (x != 0 && x != numOfXCells - 1 && y == numOfYCells - 1)
			{
				grid[x][numOfYCells - 1]->walls[Direction::RIGHT]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[x][numOfYCells - 1]->walls[Direction::RIGHT]->offset.Y = 400;
				grid[x][numOfYCells - 1]->walls[Direction::RIGHT]->zRotation = 0;

				UE_LOG(LogTemp, Warning, TEXT("RIGHT NON CORNER x: %d y: %d"), x, y);
			}
			// Bottom non corners
			else if (y != 0 && y != numOfYCells - 1 && x == 0)
			{
				grid[0][y]->walls[Direction::BOTTOM]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[0][y]->walls[Direction::BOTTOM]->offset.X = 0;
				grid[0][y]->walls[Direction::BOTTOM]->zRotation = 90;

				UE_LOG(LogTemp, Warning, TEXT("BOTTOM NON CORNER x: %d y: %d"), x, y);
			}
			// Top non corners
			else if (y != 0 && y != numOfYCells - 1 && x == numOfXCells - 1)
			{
				grid[numOfXCells - 1][y]->walls[Direction::TOP]->type.RemoveAll([](MeshType opt) {
					return opt != MeshType::WALL;
				});

				grid[numOfXCells - 1][y]->walls[Direction::TOP]->offset.X = 400;
				grid[numOfXCells - 1][y]->walls[Direction::TOP]->zRotation = 90;

				UE_LOG(LogTemp, Warning, TEXT("TOP NON CORNER x: %d y: %d"), x, y);
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
			// Top Left Corner
			else if (x == numOfXCells - 1 && y == 0)
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
			// Bottom Right Corner
			else if (x == 0 && y == numOfYCells - 1)
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

	// TODO: Recursive problem
	GenerateRoomInfo();

	// SpawnItems();


	/////////////////////////////////////////////////////////////////////////////////
	// Attempt to spawn exclusion zone around building for PCG
	// Spawn Exclusion Zone around Building?
	/////////////////////////////////////////////////////////////////////////////////
	//auto actorLocation = GetActorLocation();
	//UE_LOG(LogTemp, Warning, TEXT("ActorLocation x: %f y: %f"), actorLocation.X, actorLocation.Y);
	//auto exclusionZone = GetWorld()->SpawnActor<AStaticMeshActor>(FVector(actorLocation.X + (numOfXCells/2) * 400, actorLocation.Y + (numOfYCells/2) * 400, actorLocation.Z), FRotator(0, 0, 0));
	//if (exclusionZone)
	//{
	//	exclusionZone->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	//	exclusionZone->GetStaticMeshComponent()->SetStaticMesh(exclusionObject);
	//	exclusionZone->Tags.Add("Exclusion");

	//	exclusionZone->SetActorScale3D(FVector(numOfYCells, numOfXCells, 1));
	//}
	/////////////////////////////////////////////////////////////////////////////////

	generationFinishedCallback.Broadcast();

	generationFinished = true;

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
			FTransform trans(origin + FVector(x*400, y*400, 0));
			FTransform ceilingTrans(origin + FVector(x * 400, y * 400, 400));

			UE_LOG(LogTemp, Warning, TEXT("Spawn Building transform z: %f origin z: %f"), trans.GetLocation().Z, origin.Z);

			// Spawn the floor
			floorComponent->AddInstance(trans);

			// Spawn the ceiling
			ceilingComponent->AddInstance(ceilingTrans);

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
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("origin z: %f"), origin.Z));
		FVector spawnLocation = origin + FVector(x * 400, y * 400, 0) + info->offset;
		switch (info->type[0])
		{
			case MeshType::DOORWAY:
			{
				doorComponent->AddInstance(FTransform(FRotator(0, info->zRotation, 0), spawnLocation));
				break;
			}
			case MeshType::WALL:
			{
				wallComponent->AddInstance(FTransform(FRotator(0, info->zRotation, 0), spawnLocation));
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

void AProceduralBuildingGenerator::setNumOfXCells(int xCells)
{
	numOfXCells = xCells;
}

void AProceduralBuildingGenerator::setNumOfYCells(int yCells)
{
	numOfYCells = yCells;
}

void AProceduralBuildingGenerator::setWallOption(UStaticMesh* option)
{
	wallOption = option;
}

void AProceduralBuildingGenerator::setDoorOption(UStaticMesh* option)
{
	doorOption = option;
}

void AProceduralBuildingGenerator::setFloorOption(UStaticMesh* option)
{
	floorOption = option;
}

float AProceduralBuildingGenerator::getBuildingMinX()
{
	return origin.X;
}

float AProceduralBuildingGenerator::getBuildingMaxX()
{
	// Getting the location of the original floor component. Adding the number of X Cells (represented by rows) * size of the floor component. Then adding the offset of X to shift the value to the edge of the floor
	return getBuildingMinX() + ((numOfXCells - 1) * 400) + grid[numOfXCells - 1][0]->walls[Direction::TOP]->offset.X;
}

float AProceduralBuildingGenerator::getBuildingMinY()
{
	return origin.Y;
}

float AProceduralBuildingGenerator::getBuildingMaxY()
{
	return getBuildingMinY() + ((numOfYCells - 1) * 400) + grid[0][numOfYCells - 1]->walls[Direction::RIGHT]->offset.Y;
}

bool AProceduralBuildingGenerator::isGenerationFinished()
{
	return generationFinished;
}

FVector AProceduralBuildingGenerator::getOrigin()
{
	return origin;
}

void AProceduralBuildingGenerator::GenerateRoomInfo()
{
	// Should use something like Wave Function Collapse to gather all the information for the room
	RoomDetection(true);
}

void AProceduralBuildingGenerator::RoomDetection(bool initialPick)
{
	if (initialPick)
	{
		// Get initial position to build out rooms from
		int initialX = FMath::RandHelper(numOfXCells);
		int initialY = FMath::RandHelper(numOfYCells);

		// Call function to handle the cell being part of the room
		RoomInformation(initialX, initialY, nullptr);
	}
	else
	{
		// Get next un assigned cell
		TPair<int, int> xyPair;
		if (GetNextCellUnassignedToRoom(xyPair))
		{
			RoomInformation(xyPair.Key, xyPair.Value, nullptr);
		}
	}

	TPair<int, int> dummy;
	// Detect if there are any cells not assigned a room
	if (GetNextCellUnassignedToRoom(dummy))
	{
		return RoomDetection(false);
	}
	else
	{
		return;
	}
}

void AProceduralBuildingGenerator::RoomInformation(int x, int y, TSharedPtr<RoomInfo> room)
{
	// How do we know if it's being added to part of an existing room or it's a new one?

	if (!room)
	{
		room = MakeShared<RoomInfo>();
	}

	room->gridCellsInRoom.Add(TTuple<int, int>(x, y));

	_cellsAssignedToARoom.Add(TTuple<int, int>(x, y));

	_roomAssignmentQueue.Remove(TTuple<int, int, TSharedPtr<RoomInfo>>(x, y, room));

	// Detect if room is complete or not
	// Should have num of unique x many Left and Right Walls/Doors
	// Should have num of unique y many Top and Bottom Walls
	TSet<int> xs;
	TSet<int> ys;
	for (auto pair : room->gridCellsInRoom)
	{
		xs.Add(pair.Key);
		ys.Add(pair.Value);
	}

	int leftCount = 0;
	int rightCount = 0;
	int topCount = 0;
	int bottomCount = 0;
	for (auto pair : room->gridCellsInRoom)
	{
		// Add for Left Wall or Door
		if (grid[pair.Key][pair.Value]->walls[Direction::LEFT]->type[0] == MeshType::DOORWAY ||
			grid[pair.Key][pair.Value]->walls[Direction::LEFT]->type[0] == MeshType::WALL)
		{
			leftCount++;
		}

		if (grid[pair.Key][pair.Value]->walls[Direction::RIGHT]->type[0] == MeshType::DOORWAY ||
			grid[pair.Key][pair.Value]->walls[Direction::RIGHT]->type[0] == MeshType::WALL)
		{
			rightCount++;
		}

		if (grid[pair.Key][pair.Value]->walls[Direction::TOP]->type[0] == MeshType::DOORWAY ||
			grid[pair.Key][pair.Value]->walls[Direction::TOP]->type[0] == MeshType::WALL)
		{
			topCount++;
		}

		if (grid[pair.Key][pair.Value]->walls[Direction::BOTTOM]->type[0] == MeshType::DOORWAY ||
			grid[pair.Key][pair.Value]->walls[Direction::BOTTOM]->type[0] == MeshType::WALL)
		{
			bottomCount++;
		}

		// Check that counts match xs and ys
		if (leftCount == xs.Num() && rightCount == xs.Num() &&
			topCount == ys.Num() && bottomCount == ys.Num())
		{
			// Completed Room
			return;
		}
		else
		{
			// Uncompleted Room

			// Recursive call until room is completed

			// How to detect what coordinates don't have the expected wall/door?
			//  Why don't we use the for loop above and check the x and y and for the directions associated with the x and y

		}


		// Something To Ponder:
		// If we start at a corner we could recursively call until we hit each other corner

		if (!_cellsAssignedToARoom.Contains(TPair<int, int>(pair.Key, pair.Value)))
		{
			// Corners
			if (pair.Key == 0 && pair.Value == 0)
			{
				// Top and Right
				if (grid[pair.Key][pair.Value]->walls[Direction::TOP]->type[0] == MeshType::NONE)
				{
					_roomAssignmentQueue.Add(TTuple<int, int, TSharedPtr<RoomInfo>>(pair.Key + 1, pair.Value, room));
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::RIGHT]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key, pair.Value + 1, room);
				}
			}
			else if (pair.Key == 0 && pair.Value == numOfYCells - 1)
			{
				// Top and Left
				if (grid[pair.Key][pair.Value]->walls[Direction::TOP]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key + 1, pair.Value, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::LEFT]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key, pair.Value - 1, room);
				}
			}
			else if (pair.Key == numOfXCells - 1 && pair.Value == 0)
			{
				// Bottom and Right
				if (grid[pair.Key][pair.Value]->walls[Direction::BOTTOM]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key - 1, pair.Value, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::RIGHT]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key, pair.Value + 1, room);
				}
			}
			else if (pair.Key == numOfXCells - 1 && pair.Value == numOfYCells - 1)
			{
				// Bottom and Left
				if (grid[pair.Key][pair.Value]->walls[Direction::BOTTOM]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key - 1, pair.Value, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::LEFT]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key, pair.Value - 1, room);
				}
			}
			// Min X and Middle Y
			else if (pair.Key == 0)
			{
				// Left, Top and Right
				if (grid[pair.Key][pair.Value]->walls[Direction::LEFT]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key, pair.Value - 1, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::TOP]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key + 1, pair.Value, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::RIGHT]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key, pair.Value + 1, room);
				}
			}
			// Max X and Middle Y
			else if (pair.Key == numOfXCells - 1)
			{
				// Left, Bottom, and Right
				if (grid[pair.Key][pair.Value]->walls[Direction::LEFT]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key, pair.Value - 1, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::BOTTOM]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key - 1, pair.Value, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::RIGHT]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key, pair.Value + 1, room);
				}
			}
			// Middle X and Min Y
			else if (pair.Value == 0)
			{
				// Top, Right, and Bottom

				if (grid[pair.Key][pair.Value]->walls[Direction::TOP]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key + 1, pair.Value, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::RIGHT]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key, pair.Value + 1, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::BOTTOM]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key - 1, pair.Value, room);
				}
			}
			// Middle X and Max Y
			else if (pair.Value == numOfYCells - 1)
			{
				// Top, Left, and Bottom

				if (grid[pair.Key][pair.Value]->walls[Direction::TOP]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key + 1, pair.Value, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::LEFT]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key, pair.Value - 1, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::BOTTOM]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key - 1, pair.Value, room);
				}
			}
			// All other cells
			else
			{
				// Top, Left, Right, and Bottom

				if (grid[pair.Key][pair.Value]->walls[Direction::TOP]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key + 1, pair.Value, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::LEFT]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key, pair.Value - 1, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::RIGHT]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key, pair.Value + 1, room);
				}

				if (grid[pair.Key][pair.Value]->walls[Direction::BOTTOM]->type[0] == MeshType::NONE)
				{
					return RoomInformation(pair.Key - 1, pair.Value, room);
				}

			}
		}
	}

	// Do we have any items queued?
}

 bool AProceduralBuildingGenerator::GetNextCellUnassignedToRoom(TPair<int, int>& pair)
{
	for (int x = 0; x < numOfXCells; x++)
	{
		for (int y = 0; y < numOfYCells; y++)
		{
			if (!_cellsAssignedToARoom.Contains(TPair<int, int>(x, y)))
			{
				pair = TPair<int, int>(x, y);
				return true;
			}

		}
	}

	return false;
}