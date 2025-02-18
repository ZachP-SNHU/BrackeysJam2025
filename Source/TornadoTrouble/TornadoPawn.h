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

	// =================== MOVEMENT & INPUT ===================
	void MoveForward(float Value);
	void MoveRight(float Value);
	void ApplyDrift(float DeltaTime);
	void StartBoost();
	void AffectNearbyObjects();
	void DetectNearMiss(AActor* Object);

	// =================== COMPONENTS ===================
	// Collision
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Collision")
	class USphereComponent* TornadoCollision;
	
	// Tornado Mesh (Main Skeletal Mesh)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USkeletalMeshComponent* TornadoMesh;

	// REMOVE AFTER SKELETAL MESH IMPLEMENTATION
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UStaticMeshComponent* TestMesh;

	// Camera Setup
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UCameraComponent* Camera;

	// Movement Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UFloatingPawnMovement* MovementComponent;

	// =================== CAMERA SETTINGS ===================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Settings")
	float CameraFollowSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Settings")
	float CameraLateralOffset = 100.0f;
	
	// =================== MOVEMENT PROPERTIES ===================
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tornado Properties")
	float Friction = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tornado Properties")
	float DriftFactor = 0.2f;

	FVector MoveDirection;
	FVector CurrentVelocity;

	// =================== BOOST PROPERTIES ===================
	UPROPERTY(EditAnywhere, Category="Boost")
	float BoostMultiplier = 2.0f;
	
	UPROPERTY(EditAnywhere, Category="Boost")
	float BoostDuration = 2.0f;

	UPROPERTY(EditAnywhere, Category="Boost")
	float BoostCooldown = 3.0f;

	bool bIsBoosting = false;
	bool bCanBoost = true;
	float BoostEndTime = 0.0f;
	float NextBoostTime = 0.0f;

	// =================== JUMP & INPUT TRACKING ===================
	bool bIsJumping;
	bool bHasMoved = false; // Wait for player to make an input

	// =================== SCORING SYSTEM ===================
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Scoring")
	float DestructionScore = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Scoring")
	float TimeScore = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Scoring")
	float NearMissBonus = 0.0f;

	float StartTime;
};
