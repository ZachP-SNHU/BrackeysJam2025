// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TornadoPhysicsObject.generated.h"

UCLASS()
class TORNADOTROUBLE_API ATornadoPhysicsObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ATornadoPhysicsObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Physics Properties")
	class UStaticMeshComponent* ObjectMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics Properties")
	bool bCausesGrowth = true;

	bool HasBeenCounted() const
	{
		return bHasbeenCounted;
	}

	void SetHasBeenCounted() { bHasbeenCounted = true; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics Properties")
	float LiftForce = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics Properties")
	float DragFactor = 0.95f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics Properties")
	float ObjectMass = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Physics Properties")
	bool bIsDestructible = false;

	void ApplyTornadoForce(FVector TornadoLocation,float TornadoStrength);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scoring")
	float DamagePenalty = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Scoring")
	float OutOfBoundsHeight = -1000.0f;


private:
	bool bHasbeenCounted;
};
