// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "Kismet/GameplayStatics.h"

#include "GameMode_Procedural.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPROCFOLIAGE_API AGameMode_Procedural : public AGameModeBase
{
	GENERATED_BODY()

	
	
public:
	virtual void BeginPlay() override;
};
