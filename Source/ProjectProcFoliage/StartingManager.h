// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "StartingManager.generated.h"

UCLASS()
class PROJECTPROCFOLIAGE_API AStartingManager : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = UI, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> loadingScreenClass;
	
public:	
	// Sets default values for this actor's properties
	AStartingManager();

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void DeleteLoadingScreen();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
