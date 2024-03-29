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

	ULevel* currentLevel = GetWorld()->GetCurrentLevel();
	TArray<FName> rowNames = levelData->GetRowNames();

	// Loop through all rows until the correct level is found
	for (FName rowName : rowNames)
	{
		FLevelData* tmpData = levelData->FindRow<FLevelData>(rowName, "");
		// TODO: Find a way to compare current level and a row in the data table, if we can use just names, it's a weak comparison but it'd work
		// TODO: Make sure this works in a packages build
		FString levelName;
		FString throwaway;
		currentLevel->GetPathName().Split(FString(":"), &levelName, &throwaway);
		
		if (tmpData->level->GetPathName() == levelName)
		{
			// Initialize variables to avoid replacing of variables
			numOfXCells = tmpData->numOfXCells;
			numOfYCells = tmpData->numOfYCells;
			numOfOrbsToSpawn = tmpData->numOfOrbsToSpawn;
			percentageOfOrbsNeeded = tmpData->percentageOfOrbsNeeded;
			teleporterMesh = tmpData->teleporterMesh;
			orbMesh = tmpData->orbMesh;
			orbMovementCurve = tmpData->orbMovementCurve;
			TeleporterActiveMaterialSlotName = tmpData->TeleporterActiveMaterialSlotName;
			TeleporterActiveMaterial = tmpData->TeleporterActiveMaterial;

			break;
		}
	}

	// TODO: Probably should have a condition to verify that we polled the right data to prevent odd crashes

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

	if (_volume)
	{
		boxExtent = _volume->GetLocalBounds().BoxExtent;

		FVector extent = boxExtent;
		extent *= 2;

		xSeperationAmnt = extent.X / numOfXCells;
		ySeperationAmnt = extent.Y / numOfYCells;
	}

	AGameModeBase* mode = UGameplayStatics::GetGameMode(GetWorld());
	AGameMode_Procedural* castedMode = Cast<AGameMode_Procedural>(mode);

	if (castedMode)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game Mode Valid"));
		castedMode->buildingsGeneratedDelegate.AddDynamic(this, &AProceduralFoliageGenerator::setupForGeneration);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Game Mode NOT Valid"));
	}
	


	// TODO: Move to a on reception of building finished generating event
	//GenerateTerrainGrid(true);

	//SpawnGrid();
	
}

// Called every frame
void AProceduralFoliageGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProceduralFoliageGenerator::setupForGeneration()
{
	AActor* foundGenerator = UGameplayStatics::GetActorOfClass(GetWorld(), AProceduralBuildingGenerator::StaticClass());
	buildingGenerator = Cast<AProceduralBuildingGenerator>(foundGenerator);

	if (buildingGenerator)
	{
		UE_LOG(LogTemp, Warning, TEXT("Building generator Found!"));

		// Get max X, max Y, min X, min Y to fill in grid here
		float minX = buildingGenerator->getBuildingMinX();
		float maxX = buildingGenerator->getBuildingMaxX();
		float minY = buildingGenerator->getBuildingMinY();
		float maxY = buildingGenerator->getBuildingMaxY();
		FVector buildingOrigin = buildingGenerator->getOrigin();

		// Fill in grid 

		//FHitResult t;
		//DrawDebugLine(GetWorld(), FVector(buildingOrigin.X, buildingOrigin.Y, 10000), FVector(buildingOrigin.X, buildingOrigin.Y, -100), t.bBlockingHit ? FColor::Cyan : FColor::Black, true, -1, 0, 50.0);

		//FHitResult min;
		//DrawDebugLine(GetWorld(), FVector(minX, minY, 10000), FVector(minX, minY, -100), min.bBlockingHit ? FColor::Cyan : FColor::Yellow, true, -1, 0, 50.0);

		//FHitResult max;
		//DrawDebugLine(GetWorld(), FVector(maxX, maxY, 10000), FVector(maxX, maxY, -100), max.bBlockingHit ? FColor::Cyan : FColor::Magenta, true, -1, 0, 50.0);

		// Translate Building Coordinates to be in reference to _volumes origin ( ActorLocation - boxExtent.X && ActorLocation - boxExtent.Y)
		float volumeOriginX = GetActorLocation().X - boxExtent.X;
		float volumeOriginY = GetActorLocation().Y - boxExtent.Y;

		int startingX = FMath::Min<int>((buildingOrigin.X - volumeOriginX) / xSeperationAmnt, numOfXCells - 1);
		int startingY = FMath::Min<int>((buildingOrigin.Y - volumeOriginY) / ySeperationAmnt, numOfYCells - 1);
		int endingX = FMath::Min<int>((maxX - volumeOriginX) / xSeperationAmnt, numOfXCells - 1);
		int endingY = FMath::Min<int>((maxY - volumeOriginY) / ySeperationAmnt, numOfYCells - 1);

		for (int x = startingX; x <= endingX; x++)
		{
			for (int y = startingY; y <= endingY; y++)
			{
				auto origin = GetActorLocation();
				float startX = origin.X - boxExtent.X + (x * xSeperationAmnt);
				float startY = origin.Y - boxExtent.Y + (y * ySeperationAmnt);

				FVector startLoc(startX, startY, origin.Z + boxExtent.Z);
				FVector endLoc = startLoc - FVector(0, 0, 100000);

				//FHitResult hit;
				//DrawDebugLine(GetWorld(), FVector(startLoc.X, startLoc.Y, 10000), endLoc, hit.bBlockingHit ? FColor::Green : FColor::Silver, true, -1, 0, 50.0);

				// Remove all non selected options
				if (grid.Num() > 0 && grid[x].Num() > 0)
				{
					grid[x][y]->possibilities.RemoveAll([this](GridOption opt) {
						// Removes all entries that meet this condition
						return opt != GridOption::Architecture;
						});
				}
			}
		}

		GenerateTerrainGrid(true);

		SpawnGrid();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Building generator NOT found!"));
	}
}

void AProceduralFoliageGenerator::GenerateTerrainGrid(bool initialPick)
{
	int x;
	int y;

	// Pick an initial point to start with
	if (initialPick)
	{
		bool needsLoc = true;
		while (needsLoc)
		{
			// Random index into the array
			x = FMath::RandRange(0, numOfXCells - 1);
			y = FMath::RandRange(0, numOfYCells - 1);

			//Ensure that this is not a location for the building
			// Ensure the grid isn't filled. Shouldn't ever be but this ensure we won't get in an infinite loop in theory
			if (grid[x][y]->possibilities.Num() > 1 || IsGridFilled())
			{
				needsLoc = false;
			}
		}

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

		if (grid[x][y]->possibilities.Num() > 1)
		{
			// Get random number between 0 and 1
			float chance = FMath::RandRange(0.0, 1.0);
			
			// Get the option associated with the generated chance
			GridOption option = getOptionFromChance(chance);
			int index = grid[x][y]->possibilities.Find(option);

			// Make sure that we have the proper option set
			if (index == INDEX_NONE)
			{
				option = GridOption::None;
			}

			FillInGrid(x, y, option, initialPick);
		}

		// Remove the pair for the queue
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
			// Get random number between 0 and 1
			float chance = FMath::RandRange(0.0, 1.0);

			// Get the option associated with the generated chance
			GridOption option = getOptionFromChance(chance);
			int index = grid[xyPair.Key][xyPair.Value]->possibilities.Find(option);

			// Make sure that we have the proper option set
			if (index == INDEX_NONE)
			{
				option = GridOption::None;
			}

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

	// Based on the selected option get a random item from the appropriate array
	switch (*grid[x][y]->possibilities.begin())
	{
		case GridOption::None:
			break;
		case GridOption::Architecture:
		{
			float chance = FMath::RandRange(0.0, 1.0);
			UStaticMesh* chosenMesh = getArchitectureFromChance(chance);
			if (chosenMesh)
			{
				grid[x][y]->selectedMesh = chosenMesh;
			}
			// What happens when we don't select a mesh?
			// We can just not spawn anything even tho we have it blocked out as architecture
			// We can change this to become a none as well

			break;
		}
		case GridOption::Foliage:
		{
			// If none match don't spawn anything
			float randNum = FMath::RandRange(0.0, 1.0);

			TArray<FName> rowNames = foliageData->GetRowNames();

			for (FName rowName : rowNames)
			{
				FFoliageSpawnData* rowData = foliageData->FindRow<FFoliageSpawnData>(rowName, "");
				if (randNum > rowData->min && randNum < rowData->max)
				{
					// This is the row we are using
					grid[x][y]->selectedMesh = rowData->mesh;
				}
			}

			break;
		}
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

		// TODO If origin isn't in center of mesh this breaks
		// Do we extend into both previous and next x tiles (Cardinal directions)
		if (extent.X > xSeperationAmnt)
		{
			// Determine how many times do we go over the tile amount
			int tileAmnt = extent.X / xSeperationAmnt;
			int i = 1;

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

	//UE_LOG(LogTemp, Warning, TEXT("volume extent x: %d, y: %d"), boxExtent.X, boxExtent.Y);
	//UE_LOG(LogTemp, Warning, TEXT("origin x: %d, y: %d"), origin.X, origin.Y);

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

			// Forcing middle spawns for testing
			/*spawnX = origin.X - boxExtent.X + ((x + 0.5) * xSeperationAmnt);
			spawnY = origin.Y - boxExtent.Y + ((y + 0.5) * ySeperationAmnt);*/

			// Spawn Floating Text to help debug grid
			ATextRenderActor* text = GetWorld()->SpawnActor<ATextRenderActor>(FVector(spawnX, spawnY, origin.Z + 10), FRotator(90, 90, 90));
			if (text)
			{
				auto rend = text->GetTextRender();
				rend->SetText(FText::FromString(FString::Printf(TEXT("X: %d Y: %d"), x, y)));
			}

			// Line Trace to Landscape
			FHitResult hit;
			FVector startLoc(spawnX, spawnY, origin.Z + boxExtent.Z);
			FVector endLoc(spawnX, spawnY, (origin.Z + boxExtent.Z) - 100000);

			//UE_LOG(LogTemp, Warning, TEXT("Grid cell x: %d cell y: %d type: %s x: %f y: %f"), x, y, *UEnum::GetValueAsString(*grid[x][y]->possibilities.begin()), spawnX, spawnY);

			//UE_LOG(LogTemp, Warning, TEXT("spawn grid x: %d grid y: %d spawn x: %d, spawn y: %d"), x, y, spawnX, spawnY);
			GetWorld()->LineTraceSingleByChannel(hit, startLoc, endLoc, ECollisionChannel::ECC_WorldStatic);
			//DrawDebugLine(GetWorld(), startLoc, endLoc, hit.bBlockingHit ? FColor::Blue : FColor::Red, true, -1, 0, 3.0);

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
							teleporter = GetWorld()->SpawnActor<ATeleporter>(hit.Location, FRotator(0, FMath::RandRange(0.0, 359.0), 0));
							teleporter->setMesh(teleporterMesh);
							teleporter->setPercentageNeeded(percentageOfOrbsNeeded);
							teleporter->setActiveMaterial(TeleporterActiveMaterial);
							teleporter->setActiverMaterialSlotName(TeleporterActiveMaterialSlotName);
							teleporter->teleportDelegate.BindUFunction(this, "PlayerTeleporting");
						}
						else if(grid[x][y]->selectedMesh)
						{
							AParentArchitecture* architecture = GetWorld()->SpawnActor<AParentArchitecture>(hit.Location, FRotator(0, FMath::RandRange(0.0, 359.0), 0));
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
						    foliage = GetWorld()->SpawnActor<AFoliageInstance>(hit.Location, FRotator(0, FMath::RandRange(0.0, 359.0), 0));
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
							AOrb* orb = GetWorld()->SpawnActor<AOrb>(orbClass, hit.Location, FRotator());
							if (orb)
							{
								// Teleporter is first item spawned so we can bind teleporter to orb function
								//orb->orbGrabbedDelegate.AddUFunction(this, "ForwardOrbCollected");
								orb->setMesh(orbMesh);
								orb->SetTimelineCurve(orbMovementCurve);
								orb->Start();

								orbSpawnedCounter++;
							}
						}
						else if(!enemy)
						{
							// Need to adjust the Z for spawning of the Enemy
							enemy = GetWorld()->SpawnActor<ACharacter>(enemyClass, FVector(hit.Location.X, hit.Location.Y, 500), FRotator());
						}
						break;
					}
				}
			}
		}
	}

	teleporter->setNumSpawned(orbSpawnedCounter);

	// Couldn't get the Widget to be valid during a call to deleteloadingscreen so scrapping this
	/*AGameMode_Procedural* gm = Cast<AGameMode_Procedural>(GetWorld()->GetAuthGameMode());
	if (gm)
	{
		gm->DeleteLoadingScreen();
	}*/

	if (starterManager)
	{
		starterManager->DeleteLoadingScreen();
	}
}

void AProceduralFoliageGenerator::ForwardOrbCollected()
{
	UE_LOG(LogTemp, Warning, TEXT("Orb Collected"));
	teleporter->OrbCollected();
}

GridOption AProceduralFoliageGenerator::getOptionFromChance(float randNum)
{
	for (auto it : typeToMinMax)
	{
		if (randNum > it.Value.min && randNum < it.Value.max)
		{
			return it.Key;
		}
	}
	return GridOption::None;
}

UStaticMesh* AProceduralFoliageGenerator::getArchitectureFromChance(float randNum)
{
	for (auto it : architectureSpawnChance)
	{
		if (randNum > it.Value.min && randNum < it.Value.max)
		{
			return it.Key;
		}
	}

	return nullptr;
}