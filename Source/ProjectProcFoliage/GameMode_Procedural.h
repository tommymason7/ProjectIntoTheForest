// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"

#include "Kismet/GameplayStatics.h"

#include "GameMode_Procedural.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAllBuildingsGenerated);

/**
 * 
 */
UCLASS()
class PROJECTPROCFOLIAGE_API AGameMode_Procedural : public AGameModeBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> loadingScreen;
	
public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent)
	void CreateLoadingScreen();

	UFUNCTION(BlueprintImplementableEvent)
	void DeleteLoadingScreen();

    UPROPERTY(BlueprintCallable)
	FAllBuildingsGenerated buildingsGeneratedDelegate;

};
