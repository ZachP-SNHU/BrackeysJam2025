// Fill out your copyright notice in the Description page of Project Settings.


#include "LevelExit.h"
#include "Components/BoxComponent.h"

// Sets default values
ALevelExit::ALevelExit()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>("SkeletalMesh");
	RootComponent = SkeletalMesh;
	RootComponent = SkeletalMesh;
	SkeletalMesh->SetSimulatePhysics(false);
	SkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);


	TempMesh = CreateDefaultSubobject<UStaticMeshComponent>("TempMesh");
	TempMesh->SetupAttachment(RootComponent);

	ExitCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("ExitCollision"));
	ExitCollision->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ALevelExit::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ALevelExit::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

