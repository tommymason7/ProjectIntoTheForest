// Fill out your copyright notice in the Description page of Project Settings.


#include "ProceduralFoliageGenerator.h"

// Sets default values
AProceduralFoliageGenerator::AProceduralFoliageGenerator()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	_volume = CreateDefaultSubobject<UBoxComponent>(TEXT("Volume"));
	_volume->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AProceduralFoliageGenerator::BeginPlay()
{
	Super::BeginPlay();

	// Initial fill in Grid array
	for (int x = 0; x < numOfXCells; x++)
	{
		TArray<TSharedPtr<GridInfo>> inner;
		for (int y = 0; y < numOfYCells; y++)
		{
			inner.Add(MakeShared<GridInfo>());
		}
		grid.Add(inner);
	}

	// Change to Volume when moved to an actor
	if (_volume)
	{
		boxExtent = _volume->GetLocalBounds().BoxExtent;

		FVector extent = boxExtent;
		extent *= 2;

		xSeperationAmnt = extent.X / numOfXCells;
		ySeperationAmnt = extent.Y / numOfYCells;

		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Initial pick seperation amnt x: %f seperation amnt y: %f"), xSeperationAmnt, ySeperationAmnt));
	}

	GenerateTerrainGrid(true);

	SpawnGrid();
	
}

// Called every frame
void AProceduralFoliageGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralFoliageGenerator::GenerateTerrainGrid(bool initialPick)
{
	int x;
	int y;

	// Pick an initial point to start with
	if (initialPick)
	{
		// Random index into the array
		x = FMath::RandRange(0, numOfXCells - 1);
		y = FMath::RandRange(0, numOfYCells - 1);

		if (GEngine)
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Initial pick x: %d y: %d"), x, y));

		// Spawning teleporter
		GridOption option = GridOption::Architecture;

		FillInGrid(x, y, option, initialPick);
	}
	// Initial Pick has already occured
	else if (!_gridGenerationQueue.IsEmpty())
	{
		// Look in queue for next point to start spawn from
		TPair<int, int> xyPair = *_gridGenerationQueue.begin();

		// Get valid options and choose one
		x = xyPair.Key;
		y = xyPair.Value;

		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Grid Generation queue x: %d y: %d possible num: %d"), x, y, grid[x][y]->possibilities.Num() - 1));

		if (grid[x][y]->possibilities.Num() > 1)
		{

			GridOption option = grid[x][y]->possibilities[FMath::RandRange(0, grid[x][y]->possibilities.Num() - 1)];

			//GridOption option = GridOption::Foliage;

			FillInGrid(x, y, option, initialPick);
		}

		// Remove the pair for the queue
		_gridGenerationQueue.Remove(xyPair);

		// Going to need to check for selected mesh extent interfering with already set grid pieces
		// If it isn't then just set as none
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
			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Queue is empty x: %d y: %d"), xyPair.Key, xyPair.Value));
			GridOption option = grid[xyPair.Key][xyPair.Value]->possibilities[FMath::RandRange(0, grid[xyPair.Key][xyPair.Value]->possibilities.Num() - 1)];
			FillInGrid(xyPair.Key, xyPair.Value, option, initialPick);
		}
	}

	// Check that all locations has a single item set
	if (IsGridFilled())
	{
		return;
	}
	else
	{
		return GenerateTerrainGrid(false);
	}
}

void AProceduralFoliageGenerator::FillInGrid(int x, int y, GridOption option, bool initialPick)
{
	// Allows us to determine where to spawn the teleporter
	grid[x][y]->initialPick = initialPick;

	// Remove all non selected options
	grid[x][y]->possibilities.RemoveAll([option](GridOption opt) {
		// Removes all entries that meet this condition
		return opt != option;
	});

	//UE_LOG(LogTemp, Warning, TEXT("Fill in grid possibility x: %d y: %d num: %d"), x, y, grid[x][y]->possibilities.Num());

	// Based on the selected option get a random item from the appropriate array
	switch (*grid[x][y]->possibilities.begin())
	{
	case GridOption::None:
		break;
	case GridOption::Architecture:
		grid[x][y]->selectedMesh = architectureOptions[FMath::RandRange(0, architectureOptions.Num() - 1)];
		break;
	case GridOption::Foliage:
		grid[x][y]->selectedMesh = foliageOptions[FMath::RandRange(0, foliageOptions.Num() - 1)];
		break;
	}

	grid[x][y]->rootLocation = true;

	// Fill in neighboring cells that will be effected by the size of the mesh

	// Get Extent of Mesh and double it
	FVector origin;
	FVector extent;

	if (grid[x][y]->selectedMesh)
	{
		FBoxSphereBounds bounds = grid[x][y]->selectedMesh->GetBounds();
		extent = bounds.BoxExtent;

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Extent X: %f X seperation amnt: %f"), extent.X, xSeperationAmnt));

		// TODO If origin isn't in center of mesh this breaks
		// Do we extend into both previous and next x tiles (Cardinal directions)
		if (extent.X > xSeperationAmnt)
		{
			// Determine how many times do we go over the tile amount
			int tileAmnt = extent.X / xSeperationAmnt;
			int i = 1;

			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("TileAmnt: %d"), tileAmnt));
			for (; i <= tileAmnt; i++)
			{
				// Check if x - i is less than 0
				if (x - i >= 0)
				{
					// Valid tile
					grid[x - i][y]->possibilities.RemoveAll([option](GridOption opt) {
						// Removes all entries that meet this condition
						return opt != option;
						});
					grid[x - i][y]->selectedMesh = grid[x][y]->selectedMesh;
				}

				if (x + i < grid.Num())
				{
					grid[x + i][y]->possibilities.RemoveAll([option](GridOption opt) {
						// Removes all entries that meet this condition
						return opt != option;
						});
					grid[x + i][y]->selectedMesh = grid[x][y]->selectedMesh;
				}
			}

			// TODO: For each case consider removing the option of another piece of architecture spawning to a builidng
			//       AKA a building can't spawn right next to buidling

			// i is in a position where it not part of the grid for the currentrly selected mesh
			// Check if x - i and x + i are valid locations
			if (x - i >= 0)
			{
				// Valid position add it to the queue

				// Can't just remove from x - i possibilities because foliage and none can be next to each other
				switch (*grid[x][y]->possibilities.begin())
				{
				case GridOption::None:
					break;
				case GridOption::Architecture:
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("over tile x - i removing architecture")));
					grid[x - i][y]->possibilities.Remove(GridOption::Architecture);
					break;
				case GridOption::Foliage:
					break;
				}

				_gridGenerationQueue.Add(TPair<int, int>(x - i, y));
			}

			if (x + i < grid.Num())
			{
				switch (*grid[x][y]->possibilities.begin())
				{
				case GridOption::None:
					break;
				case GridOption::Architecture:
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("over tile x + i removing architecture")));
					grid[x + i][y]->possibilities.Remove(GridOption::Architecture);
					break;
				case GridOption::Foliage:
					break;
				}

				_gridGenerationQueue.Add(TPair<int, int>(x + i, y));
			}
		}
		// The mesh doesn't extend past the current square
		else
		{
			// Check if x - 1 and x + 1 are valid locations
			if (x - 1 >= 0)
			{
				// Valid position add it to the queue
				// TODO: Not sure using this function works well because this is a FILO Queue

				switch (*grid[x][y]->possibilities.begin())
				{
				case GridOption::None:
					break;
				case GridOption::Architecture:
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("x - 1 removing architecture")));
					grid[x - 1][y]->possibilities.Remove(GridOption::Architecture);
					break;
				case GridOption::Foliage:
					break;
				}

				_gridGenerationQueue.Add(TPair<int, int>(x - 1, y));
			}

			if (x + 1 < grid.Num())
			{
				switch (*grid[x][y]->possibilities.begin())
				{
				case GridOption::None:
					break;
				case GridOption::Architecture:
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("x + 1 removing architecture")));
					grid[x + 1][y]->possibilities.Remove(GridOption::Architecture);
					break;
				case GridOption::Foliage:
					break;
				}

				_gridGenerationQueue.Add(TPair<int, int>(x + 1, y));
			}
		}

		//(Cardinal directions)
		if (extent.Y > ySeperationAmnt)
		{
			int tileAmnt = extent.Y / ySeperationAmnt;

			int i = 1;
			for (; i <= tileAmnt; i++)
			{
				// Check if y - i is less than 0
				if (y - i >= 0)
				{
					// Valid tile
					grid[x][y - i]->possibilities.RemoveAll([option](GridOption opt) {
						// Removes all entries that meet this condition
						return opt != option;
						});
					grid[x][y - i]->selectedMesh = grid[x][y]->selectedMesh;
				}

				if (y + i < grid[x].Num())
				{
					grid[x][y + i]->possibilities.RemoveAll([option](GridOption opt) {
						// Removes all entries that meet this condition
						return opt != option;
						});
					grid[x][y + i]->selectedMesh = grid[x][y]->selectedMesh;
				}
			}

			// i is in a position where it not part of the grid for the currentrly selected mesh
			// Check if y - i and y + i are valid locations
			if (y - i >= 0)
			{
				// Valid position add it to the queue
				// TODO: Not sure using this function works well because this is a FILO Queue

				switch (*grid[x][y]->possibilities.begin())
				{
				case GridOption::None:
					break;
				case GridOption::Architecture:
					grid[x][y - i]->possibilities.Remove(GridOption::Architecture);
					break;
				case GridOption::Foliage:
					break;
				}

				_gridGenerationQueue.Add(TPair<int, int>(x, y - i));
			}

			if (y + i < grid[x].Num())
			{
				switch (*grid[x][y]->possibilities.begin())
				{
				case GridOption::None:
					break;
				case GridOption::Architecture:
					grid[x][y + i]->possibilities.Remove(GridOption::Architecture);
					break;
				case GridOption::Foliage:
					break;
				}

				_gridGenerationQueue.Add(TPair<int, int>(x, y + i));
			}
		}
		// The mesh doesn't extend past the current square
		else
		{
			// i is in a position where it not part of the grid for the currentrly selected mesh
			// Check if y - i and y + i are valid locations
			if (y - 1 >= 0)
			{
				// Valid position add it to the queue

				switch (*grid[x][y]->possibilities.begin())
				{
				case GridOption::None:
					break;
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("y - 1 removing architecture")));
				case GridOption::Architecture:
					grid[x][y - 1]->possibilities.Remove(GridOption::Architecture);
					break;
				case GridOption::Foliage:
					break;
				}

				_gridGenerationQueue.Add(TPair<int, int>(x, y - 1));
			}

			if (y + 1 < grid[x].Num())
			{
				switch (*grid[x][y]->possibilities.begin())
				{
				case GridOption::None:
					break;
				case GridOption::Architecture:
					GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("y + 1 removing architecture")));
					grid[x][y + 1]->possibilities.Remove(GridOption::Architecture);
					break;
				case GridOption::Foliage:
					break;
				}

				_gridGenerationQueue.Add(TPair<int, int>(x, y + 1));
			}
		}

		// Need to consider the x - 1 y - 1, x - 1 y + 1, x + 1 y - 1, x + 1 y + 1
		if (extent.X > xSeperationAmnt && extent.Y > ySeperationAmnt)
		{
			int xMax = extent.X / xSeperationAmnt;
			int yMax = extent.Y / ySeperationAmnt;

			if (x - xMax - 1 >= 0 && y - yMax - 1 >= 0)
			{
				grid[x - xMax - 1][y - yMax - 1]->possibilities.RemoveAll([option](GridOption opt) {
					// Removes all entries that meet this condition
					return opt != option;
					});
				grid[x - xMax - 1][y - yMax - 1]->selectedMesh = grid[x][y]->selectedMesh;
			}

			if (x - xMax - 1 >= 0 && y + yMax + 1 < grid[x - xMax - 1].Num())
			{
				grid[x - xMax - 1][y + yMax + 1]->possibilities.RemoveAll([option](GridOption opt) {
					// Removes all entries that meet this condition
					return opt != option;
					});
				grid[x - xMax - 1][y + yMax + 1]->selectedMesh = grid[x][y]->selectedMesh;
			}

			if (x + xMax + 1 < grid.Num() && y - yMax - 1 >= 0)
			{
				grid[x + xMax + 1][y - yMax - 1]->possibilities.RemoveAll([option](GridOption opt) {
					// Removes all entries that meet this condition
					return opt != option;
					});
				grid[x + xMax + 1][y - yMax - 1]->selectedMesh = grid[x][y]->selectedMesh;
			}

			if (x + xMax + 1 < grid.Num() && y + yMax + 1 < grid[x + xMax + 1].Num())
			{
				grid[x + xMax + 1][y + yMax + 1]->possibilities.RemoveAll([option](GridOption opt) {
					// Removes all entries that meet this condition
					return opt != option;
					});
				grid[x + xMax + 1][y + yMax + 1]->selectedMesh = grid[x][y]->selectedMesh;
			}
		}
	}
}

bool AProceduralFoliageGenerator::IsGridFilled()
{
	for (int x = 0; x < grid.Num(); x++)
	{
		for (int y = 0; y < grid[x].Num(); y++)
		{
//			GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("-------- grid x: %d grid y: %d"), x, y));
			if (grid[x][y]->possibilities.Num() > 1)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Grid isn't full found option at x: %d y: %d"), x, y));
				return false;
			}
		}
	}

	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Grid is full")));
	return true;
}

bool AProceduralFoliageGenerator::GetNextUnfilledCell(TTuple<int, int>& xyPair)
{
	for (int x = 0; x < numOfXCells; x++)
	{
		for (int y = 0; y < numOfYCells; y++)
		{
			if (grid[x][y]->possibilities.Num() > 1)
			{
				xyPair = TTuple<int, int>(x, y);
				return true;
			}
		}
	}

	return false;
}

void AProceduralFoliageGenerator::SpawnGrid()
{
	// Get extent of landscape or whatever volume or actor is being used to determine the grid
	FVector origin = GetActorLocation();

	UE_LOG(LogTemp, Warning, TEXT("volume extent x: %d, y: %d"), boxExtent.X, boxExtent.Y);
	UE_LOG(LogTemp, Warning, TEXT("origin x: %d, y: %d"), origin.X, origin.Y);

	for (int x = 0; x < numOfXCells; x++)
	{
		for (int y = 0; y < numOfYCells; y++)
		{
			// Spawn items

			// Min value per square
			float minX = origin.X - boxExtent.X + (x * xSeperationAmnt);
			float maxX = origin.X - boxExtent.X + ((x + 1) * xSeperationAmnt);
			float minY = origin.Y - boxExtent.Y + (y * ySeperationAmnt);
			float maxY = origin.Y - boxExtent.Y + ((y + 1) * ySeperationAmnt);

			float spawnX = FMath::RandRange(minX, maxX);
			float spawnY = FMath::RandRange(minY, maxY);

			//spawnX = origin.X - boxExtent.X + ((x + 0.5) * xSeperationAmnt);
			//spawnY = origin.Y - boxExtent.Y + ((y + 0.5) * ySeperationAmnt);

			// Line Trace to Landscape
			FHitResult hit;
			FVector startLoc(spawnX, spawnY, origin.Z + boxExtent.Z);
			FVector endLoc(spawnX, spawnY, (origin.Z + boxExtent.Z) - 100000);

			UE_LOG(LogTemp, Warning, TEXT("spawn grid x: %d grid y: %d spawn x: %d, spawn y: %d"), x, y, spawnX, spawnY);
			GetWorld()->LineTraceSingleByChannel(hit, startLoc, endLoc, ECollisionChannel::ECC_WorldStatic);
			DrawDebugLine(GetWorld(), startLoc, endLoc, hit.bBlockingHit ? FColor::Blue : FColor::Red, true, -1, 0, 3.0);

			if (grid[x][y]->rootLocation)
			{
				if (hit.bBlockingHit)
				{
					switch (*grid[x][y]->possibilities.begin())
					{
					case GridOption::Architecture:
					{
						if (grid[x][y]->initialPick)
						{
							// Spawn Teleporter
							teleporter = GetWorld()->SpawnActor<ATeleporter>(hit.Location, FRotator());
							teleporter->setMesh(teleporterMesh);
						}
						else
						{
							AParentArchitecture* architecture = GetWorld()->SpawnActor<AParentArchitecture>(hit.Location, FRotator());
							if (architecture && grid[x][y]->selectedMesh)
							{
								FBoxSphereBounds bounds = grid[x][y]->selectedMesh->GetBounds();
								FVector extent = bounds.BoxExtent;
								
								architecture->setMesh(grid[x][y]->selectedMesh);
							}
						}
						break;
					}
					case GridOption::Foliage:
					{
						AFoliageInstance* foliage = _meshToActor.FindRef(grid[x][y]->selectedMesh);
						if (grid[x][y]->selectedMesh && foliage)
						{
							//_foliage->AddMesh(grid[x][y]->selectedMesh);
							foliage->AddInstance(FTransform(hit.Location), true);
						}
						else if (grid[x][y]->selectedMesh && !foliage)
						{
						    foliage = GetWorld()->SpawnActor<AFoliageInstance>(hit.Location, FRotator());
							foliage->SetMesh(grid[x][y]->selectedMesh);
							foliage->AddInstance(FTransform(hit.Location), true);

							_meshToActor.Add(grid[x][y]->selectedMesh, foliage);
						}
						break;
					}
					case GridOption::None:
						if (orbSpawnedCounter < numOfOrbsToSpawn)
						{
							// Spawn orb
							AOrb* orb = GetWorld()->SpawnActor<AOrb>(hit.Location, FRotator());
							if (orb)
							{
								orb->orbGrabbedDelegate.AddUObject(teleporter, &ATeleporter::OrbCollected);
								orb->setMesh(orbMesh);
								orb->SetTimelineCurve(orbMovementCurve);
								orb->Start();

								orbSpawnedCounter++;
							}
						}
						break;
					}
				}
			}
		}
	}
}