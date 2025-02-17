// Fill out your copyright notice in the Description page of Project Settings.


#include "TornadoPawn.h"

#include "TornadoPhysicsObject.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
ATornadoPawn::ATornadoPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TornadoMesh = CreateDefaultSubobject<UStaticMeshComponent>("TornadoMesh");
	RootComponent = TornadoMesh;
	TornadoMesh->SetSimulatePhysics(false);
	TornadoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TornadoMesh->SetCollisionObjectType(ECC_Pawn);

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	//MovementComponent->MaxSpeed = MoveSpeed;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(TornadoMesh);
	SpringArm->TargetArmLength = 800.0f;
	SpringArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 5.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	TornadoCollision = CreateDefaultSubobject<USphereComponent>(TEXT("TornadoCollision"));
	TornadoCollision->SetupAttachment(RootComponent);
	TornadoCollision->SetSphereRadius(300.0f); // Adjust size
	TornadoCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TornadoCollision->SetCollisionObjectType(ECC_Pawn);
	TornadoCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	TornadoCollision->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	
}

// Called when the game starts or when spawned
void ATornadoPawn::BeginPlay()
{
	Super::BeginPlay();
	StartTime = GetWorld()->GetTimeSeconds();

	//BaseMoveSpeed = MovementComponent->MaxSpeed; // saves for recall after boosting
	
}

// Called every frame
void ATornadoPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UE_LOG(LogTemp, Warning, TEXT("Time Score: %f, Destruction Score: %f, Near Miss Bonus: %f"),
	TimeScore, DestructionScore, NearMissBonus);

	TimeScore = 10000.0f - (GetWorld()->GetTimeSeconds() - StartTime * 10.0f);

	TimeScore = FMath::Max(TimeScore, 0.0f);

	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (bIsBoosting && CurrentTime >= BoostEndTime)
	{
		MovementComponent->MaxSpeed /= BoostMultiplier;
		bIsBoosting = false;
		NextBoostTime = CurrentTime + BoostCooldown;
	}

	ApplyDrift(DeltaTime);
	AddMovementInput(CurrentVelocity.GetSafeNormal(), 1.0f);

	//FVector TornadoForward = GetActorForwardVector();
	FVector TornadoRight = GetActorRightVector();

	FVector TargetCameraOffset = TornadoRight * MoveDirection.Y * CameraLateralOffset;

	FVector NewCameraPos = FMath::VInterpTo(
		SpringArm->GetRelativeLocation(),
		FVector(0.0f, TargetCameraOffset.Y, SpringArm->GetRelativeLocation().Z),
		DeltaTime,
		CameraFollowSpeed
		);

	SpringArm->SetRelativeLocation(NewCameraPos);

	FRotator TargetRotation = FRotator(-45.0f, GetActorRotation().Yaw, 0.0f);
	FRotator SmoothedRotation = FMath::RInterpTo(
		SpringArm->GetRelativeRotation(),
		TargetRotation,
		DeltaTime,
		CameraFollowSpeed
		);
	SpringArm->SetRelativeRotation(SmoothedRotation);

	AffectNearbyObjects();
}

// Called to bind functionality to input
void ATornadoPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATornadoPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATornadoPawn::MoveRight);
	PlayerInputComponent->BindAction("Boost", IE_Pressed, this, &ATornadoPawn::StartBoost);
}

void ATornadoPawn::MoveForward(float Value)
{
	if (FMath::Abs(Value) > 0.01f) //detect meaningful input
	{
		bHasMoved = true;
	}
	MoveDirection.X = Value;
}

void ATornadoPawn::MoveRight(float Value)
{
	if (FMath::Abs(Value) > 0.01f) //detect meaningful input
	{
		bHasMoved = true;
	}
	MoveDirection.Y = Value;
}

void ATornadoPawn::ApplyDrift(float DeltaTime)
{

	// if player has not moved do not drift
	if (!bHasMoved)
	{
		CurrentVelocity = FVector::ZeroVector;
		return;
	}
	
	FVector TargetVelocity = MoveDirection * MovementComponent->Acceleration * DeltaTime;

	// Apply acceleration if input is given
	if (!MoveDirection.IsNearlyZero())
	{
		CurrentVelocity += TargetVelocity;

		// Clamp velocity to avoid exceeding max speed
		if (CurrentVelocity.Size() > MovementComponent->MaxSpeed)
		{
			CurrentVelocity = CurrentVelocity.GetSafeNormal() * MovementComponent->MaxSpeed;
		}
	}
	else
	{
		// Gradually slow down if no input (but never fully stop)
		float DecelerationFactor = 1.0f - (MovementComponent->Deceleration * DeltaTime * 0.001f);
		CurrentVelocity *= FMath::Clamp(DecelerationFactor, 0.85f, 1.0f);

		// Ensure some small amount of drift remains
		if (CurrentVelocity.Size() < 50.0f)
		{
			CurrentVelocity = FVector(10.0f, 10.0f, 0.0f);
		}
	}

	// Add Randomized Rotational Drift
	float RandomDriftAngle = FMath::RandRange(-5.0f, 5.0f); // Small random angle variation
	//float DriftRotationSpeed = 20.0f * DeltaTime; // How fast the drift rotates

	FRotator DriftRotation(0.0f, RandomDriftAngle, 0.0f);
	FVector DriftOffset = DriftRotation.RotateVector(CurrentVelocity.GetSafeNormal() * DriftFactor * 50.0f);

	CurrentVelocity += DriftOffset;
}

void ATornadoPawn::StartBoost()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();

	if (bCanBoost && CurrentTime >= NextBoostTime)
	{
		//MoveSpeed = BaseMoveSpeed * BoostMultiplier;
		MovementComponent->MaxSpeed *= BoostMultiplier;
		bIsBoosting = true;
		BoostEndTime = CurrentTime + BoostDuration;
	}
}

void ATornadoPawn::AffectNearbyObjects()
{
	FVector TornadoLocation = GetActorLocation();
	float TornadoStrength = 2000.0f;

	TArray<AActor*> OverlappingActors;
	TornadoCollision->GetOverlappingActors(OverlappingActors, ATornadoPhysicsObject::StaticClass());
	
	UE_LOG(LogTemp, Warning, TEXT("Found %d objects near tornado"), OverlappingActors.Num());

	for (AActor* Actor : OverlappingActors)
	{
		ATornadoPhysicsObject* PhysicsObject = Cast<ATornadoPhysicsObject>(Actor);
		if (PhysicsObject)
		{
			UE_LOG(LogTemp, Warning, TEXT("Applying force to %s"), *PhysicsObject->GetName());
			PhysicsObject->ApplyTornadoForce(TornadoLocation, TornadoStrength);
		}
		DetectNearMiss(PhysicsObject);
	}
}

void ATornadoPawn::DetectNearMiss(AActor* Object)
{
	float Distance = FVector::Dist(Object->GetActorLocation(), GetActorLocation());
    
	if (Distance < 200.0f) // Adjust for balance
	{
		NearMissBonus += 50.0f;
		UE_LOG(LogTemp, Warning, TEXT("Near miss with %s! +50 points"), *Object->GetName());
	}
}
