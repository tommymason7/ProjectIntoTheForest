// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "DataTableStructs.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FSpawnRange
{
	GENERATED_USTRUCT_BODY()

	float min;
	float max;
};

USTRUCT(BlueprintType)
struct FLevelData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString levelName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UWorld> level;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int numOfXCells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int numOfYCells;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int numOfOrbsToSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float percentageOfOrbsNeeded;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* teleporterMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* orbMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* orbMovementCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName TeleporterActiveMaterialSlotName = FName("ArchDoorway");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* TeleporterActiveMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<ACharacter*, FSpawnRange> enemyToSpawnRange;
};

USTRUCT(BlueprintType)
struct FFoliageSpawnData : public FTableRowBase
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float min;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float max;
};