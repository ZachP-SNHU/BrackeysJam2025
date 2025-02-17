// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TornadoPawn.generated.h"

UCLASS()
class TORNADOTROUBLE_API ATornadoPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATornadoPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void ApplyDrift(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UStaticMeshComponent* TornadoMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UFloatingPawnMovement* MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Settings")
	float CameraFollowSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Settings")
	float CameraLateralOffset = 100.0f;
	
	FVector MoveDirection;
	FVector CurrentVelocity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tornado Properties")
	float MoveSpeed = 600.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tornado Properties")
	float Acceleration = 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tornado Properties")
	float MaxSpeed = 1200.0f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Tornado Properties")
	float Friction = 2.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tornado Properties")
	float DriftFactor = 0.2f;
	
};
