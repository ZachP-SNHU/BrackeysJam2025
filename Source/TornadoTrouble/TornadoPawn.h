// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Pawn.h"
#include "TornadoPhysicsObject.h"
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

	// Movement + input
	void MoveForward(float Value);
	void MoveRight(float Value);
	void ApplyDrift(float DeltaTime);
	void StartBoost();
	void AffectNearbyObjects();
	void GrowTornado();

	// input detection
	bool bIsJumping;
	bool bHasMoved = false; // Wait for player to make an input

	// Components
	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	class USoundBase* BoostSound;
	
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Settings")
	float CameraFollowSpeed = 5.0f;
	
	// Movement Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	class UFloatingPawnMovement* MovementComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera Settings")
	float CameraLateralOffset = 100.0f;
	
	// movement properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tornado Properties")
	float Friction = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tornado Properties")
	float DriftFactor = 0.2f;

	FVector MoveDirection;
	FVector CurrentVelocity;

	// Strength
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tornado Properties")
	float TornadoStrength = 850.0f;

	// boost properties
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boost")
	float BoostMultiplier = 2.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boost")
	float BoostDuration = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Boost")
	float BoostCooldown = 3.0f;

	bool bIsBoosting = false;
	bool bCanBoost = true;
	float BoostEndTime = 0.0f;
	float NextBoostTime = 0.0f;
	
	// Growth System

	float TargetSize = 1.0f;
	float TargetMaxSpeed;
	float TargetAcceleration;
	float TargetDrift;
	float TargetCollisionRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Growth")
	int GrowthThreshold = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Growth")
	float GrowthMultiplier = 1.05f; // 5% growth

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Growth")
	float MaxSize = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Growth")
	int ObjectsHit = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Growth")
	float CurrentSize = 1.0f;

	// Blueprint callable functions

	UFUNCTION(BlueprintCallable, Category="Growth")
	void SetGrowthSettings(float NewMultiplier, float NewThreshold, float NewMaxSize);

	UFUNCTION(BlueprintCallable, Category="Boost")
	void SetBoostSettings(float NewMultiplier, float NewDuration, float NewCooldown);

	UFUNCTION(BlueprintCallable, Category="Tornado Strength")
	void SetTornadoStrength(float NewStrength, float NewCollisionRadius);

	UFUNCTION(BlueprintCallable, Category="Tornado Movement")
	void SetTornadoMovement(float NewMaxSpeed, float NewAcceleration, float NewDeceleration, float NewTurningBoost, float NewFriction, float NewDriftFactor);

	// Timer
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	float LevelTime = 120.0f;  // Default 2 minutes (editable per level)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoring")
	bool bLevelFailed = false;

	UFUNCTION(BlueprintCallable, Category = "Scoring")
	FString GetFormattedTime() const;

	// Scoring Variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scoring")
	float LevelScore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	float NearMissBonusValue = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	float CollisionPenaltyBase = -50.0f;

	void AddNearMissBonus();
	void AddCollisionPenalty(float PenaltyAmount);
	float CalculateDynamicPenalty(ATornadoPhysicsObject* Object);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring|NearMiss")
	float BaseNearMissDistance = 200.0f;
	//final score
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	float CalculateFinalScore(float TimeMultiplier = 10.0f) const;
	
	UFUNCTION(BlueprintImplementableEvent, Category="Scoring")
	void TriggerFailState();

	// Final near miss distance after scaling
	float CurrentNearMissDistance;

	

	// score system
	UFUNCTION(BlueprintCallable, Category = "Scoring")
	void DetectNearMiss(AActor* Object);
	
	// Near Miss Tracking
	UPROPERTY()
	TSet<AActor*> NearMissedObjects;

	
	
};

inline float ATornadoPawn::CalculateFinalScore(float TimeMultiplier) const
{
	float TimeBonus = LevelTime * TimeMultiplier;
	float RawFinalScore = TimeBonus + LevelScore;
	int32 FinalScore = FMath::FloorToInt(RawFinalScore);
	


	return static_cast<float>(FinalScore);
}
