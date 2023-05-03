// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"

#include "ParentArchitecture.h"

#include "Teleporter.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTPROCFOLIAGE_API ATeleporter : public AParentArchitecture
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	UBoxComponent* Box = nullptr;

public:
	ATeleporter();

	void Activate();
};
