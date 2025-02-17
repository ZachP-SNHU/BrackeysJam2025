// Fill out your copyright notice in the Description page of Project Settings.


#include "TornadoPhysicsObject.h"

#include "TornadoPawn.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

// Sets default values
ATornadoPhysicsObject::ATornadoPhysicsObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ObjectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectMesh"));
	RootComponent = ObjectMesh;

	ObjectMesh->SetSimulatePhysics(true);
	ObjectMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ObjectMesh->SetCollisionObjectType(ECC_PhysicsBody);
	ObjectMesh->SetEnableGravity(true);
	ObjectMesh->SetMassOverrideInKg(NAME_None, ObjectMass);
	
}

// Called when the game starts or when spawned
void ATornadoPhysicsObject::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATornadoPhysicsObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetActorLocation().Z < OutOfBoundsHeight)
	{
		ATornadoPawn* Tornado = Cast<ATornadoPawn>(GetWorld()->GetFirstPlayerController()->GetPawn());
		if (Tornado)
		{
			Tornado->DestructionScore += 50.0f;
			UE_LOG(LogTemp, Warning, TEXT("%s fell off the map! -50 points"), *GetName())
		}
		Destroy();
	}

}

void ATornadoPhysicsObject::ApplyTornadoForce(FVector TornadoLocation, float TornadoStrength)
{
	if (!ObjectMesh) return;

	FVector ForceDirection = GetActorLocation() - TornadoLocation;
	ForceDirection.Normalize();

	FVector AppliedForce = ForceDirection * TornadoStrength * LiftForce;
	AppliedForce.Z += TornadoStrength * 200.0f;

	ObjectMesh->AddForce(AppliedForce * ObjectMesh->GetMass());

	ObjectMesh->SetPhysicsLinearVelocity(ObjectMesh->GetPhysicsLinearVelocity() * DragFactor);
	
}


