// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractableParent.h"

// Sets default values
AInteractableParent::AInteractableParent()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	meshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	collisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(meshComponent);
	collisionComponent->SetupAttachment(RootComponent);
	//meshComponent->SetupAttachment(RootComponent);

	// Setup Collision with player
	meshComponent->SetCollisionProfileName(TEXT("WorldStatic"), true);
	meshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	meshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	meshComponent->SetGenerateOverlapEvents(true);
	collisionComponent->SetCollisionProfileName(TEXT("Trigger"), true);
	collisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	collisionComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	collisionComponent->SetGenerateOverlapEvents(true);
	collisionComponent->SetHiddenInGame(false);
}

// Called when the game starts or when spawned
void AInteractableParent::BeginPlay()
{
	Super::BeginPlay();

	collisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AInteractableParent::OnOverlapBegin);
}

// Called every frame
void AInteractableParent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AInteractableParent::setMesh(UStaticMesh* newMesh)
{
	if (meshComponent)
	{
		meshComponent->SetStaticMesh(newMesh);

		//TODO Scale collision component properly
		collisionComponent->SetSphereRadius(128, true);
	}
}


void AInteractableParent::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                                     bool bFromSweep, const FHitResult& SweepResult)
{
	// If we are overlapping the Player then we should call ObjectOverlapping function that is overrideable
	APlayerCharacter* player = Cast<APlayerCharacter>(OtherActor);

	if (player)
	{
		ObjectOverlapping(player);
	}
}

void AInteractableParent::ObjectOverlapping(APlayerCharacter* player) 
{
}