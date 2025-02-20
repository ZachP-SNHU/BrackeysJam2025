// Fill out your copyright notice in the Description page of Project Settings.

#include "TornadoPawn.h"
#include "TornadoPhysicsObject.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

// THIS is the constructor vvvvv

ATornadoPawn::ATornadoPawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // Collision Sphere for Detecting Physics Objects
    TornadoCollision = CreateDefaultSubobject<USphereComponent>(TEXT("TornadoCollision"));
    RootComponent = TornadoCollision;
    TornadoCollision->SetSphereRadius(150.0f);
    TornadoCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    TornadoCollision->SetCollisionObjectType(ECC_Pawn);
    TornadoCollision->SetCollisionResponseToAllChannels(ECR_Block);  // Block everything by default
    TornadoCollision->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);  // Overlap physics objects
    TornadoCollision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
    
    // skele mesh
    TornadoMesh = CreateDefaultSubobject<USkeletalMeshComponent>("TornadoMesh");
    TornadoMesh->SetupAttachment(TornadoCollision);
    TornadoMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // REMOVE AFTER SKELETAL MESH IMPLEMENTATION
    TestMesh = CreateDefaultSubobject<UStaticMeshComponent>("TestMesh");
    TestMesh->SetupAttachment(TornadoCollision);

    // Movement
    MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));

    // Camera Setup
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(TornadoMesh);
    SpringArm->TargetArmLength = 800.0f;
    SpringArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed = 5.0f;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);
    
}

void ATornadoPawn::BeginPlay()
{
    Super::BeginPlay();
    StartTime = GetWorld()->GetTimeSeconds();

    // Set initial targets to current values
    TargetSize = CurrentSize;
    TargetMaxSpeed = MovementComponent->MaxSpeed;
    TargetAcceleration = MovementComponent->Acceleration;
    TargetDrift = DriftFactor;
    TargetCollisionRadius = TornadoCollision->GetUnscaledSphereRadius();
    
}

void ATornadoPawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    FVector NewScale = FMath::VInterpTo(GetActorScale3D(), FVector(TargetSize), DeltaTime, 3.0f);
    SetActorScale3D(NewScale);
    CurrentSize = NewScale.X;

    // Smoothly interpolate speed & acceleration
    MovementComponent->MaxSpeed = FMath::FInterpTo(MovementComponent->MaxSpeed, TargetMaxSpeed, DeltaTime, 3.0f);
    MovementComponent->Acceleration = FMath::FInterpTo(MovementComponent->Acceleration, TargetAcceleration, DeltaTime, 3.0f);

    // Smoothly interpolate drift factor
    DriftFactor = FMath::FInterpTo(DriftFactor, TargetDrift, DeltaTime, 3.0f);

    //  Smoothly adjust collision radius
    float NewRadius = FMath::FInterpTo(TornadoCollision->GetScaledSphereRadius(), TargetCollisionRadius, DeltaTime, 3.0f);
    TornadoCollision->SetSphereRadius(NewRadius);

    // Debug Log for Score Tracking
    UE_LOG(LogTemp, Warning, TEXT("Time Score: %f, Destruction Score: %f, Near Miss Bonus: %f"),
        TimeScore, DestructionScore, NearMissBonus);

    // Time Score Calculation
    TimeScore = 10000.0f - ((GetWorld()->GetTimeSeconds() - StartTime) * 10.0f);
    TimeScore = FMath::Max(TimeScore, 0.0f);

    // Boost Logic
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (bIsBoosting && CurrentTime >= BoostEndTime)
    {
        MovementComponent->MaxSpeed /= BoostMultiplier;
        bIsBoosting = false;
        NextBoostTime = CurrentTime + BoostCooldown;
    }

    ApplyDrift(DeltaTime);
    AddMovementInput(CurrentVelocity.GetSafeNormal(), 1.0f);

    FHitResult HitResult;
    AddActorWorldOffset(CurrentVelocity * DeltaTime, true, &HitResult);

    if (HitResult.IsValidBlockingHit())
    {
        UE_LOG(LogTemp, Warning, TEXT("Blocked by: %s"), *HitResult.GetActor()->GetName());
        CurrentVelocity = FVector::ZeroVector;  // Stop on collision
    }
    DrawDebugSphere(GetWorld(), GetActorLocation(), TornadoCollision->GetScaledSphereRadius(), 32, FColor::Red, false, -1.0f, 0, 2.0f);

    // Camera Adjustment
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

// Input Handling

void ATornadoPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis("MoveForward", this, &ATornadoPawn::MoveForward);
    PlayerInputComponent->BindAxis("MoveRight", this, &ATornadoPawn::MoveRight);
    PlayerInputComponent->BindAction("Boost", IE_Pressed, this, &ATornadoPawn::StartBoost);
}

void ATornadoPawn::MoveForward(float Value)
{
    if (FMath::Abs(Value) > 0.01f) 
        bHasMoved = true;

    MoveDirection.X = Value;
}

void ATornadoPawn::MoveRight(float Value)
{
    if (FMath::Abs(Value) > 0.01f) 
        bHasMoved = true;

    MoveDirection.Y = Value;
}

// Movement System

void ATornadoPawn::ApplyDrift(float DeltaTime)
{
    // If player has not moved, do not drift
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
    float RandomDriftAngle = FMath::RandRange(-5.0f, 5.0f);
    FRotator DriftRotation(0.0f, RandomDriftAngle, 0.0f);
    FVector DriftOffset = DriftRotation.RotateVector(CurrentVelocity.GetSafeNormal() * DriftFactor * 50.0f);

    CurrentVelocity += DriftOffset;
}

// boost system

void ATornadoPawn::StartBoost()
{
    float CurrentTime = GetWorld()->GetTimeSeconds();

    if (bCanBoost && CurrentTime >= NextBoostTime)
    {
        MovementComponent->MaxSpeed *= BoostMultiplier;
        bIsBoosting = true;
        BoostEndTime = CurrentTime + BoostDuration;
    }
}

// physics interactions

void ATornadoPawn::AffectNearbyObjects()
{
    FVector TornadoLocation = GetActorLocation();
    //TornadoStrength *= CurrentSize; //scale strength w/ size

    TArray<AActor*> OverlappingActors;
    TornadoCollision->GetOverlappingActors(OverlappingActors, ATornadoPhysicsObject::StaticClass());
    
    UE_LOG(LogTemp, Warning, TEXT("Found %d objects near tornado"), OverlappingActors.Num());

    for (AActor* Actor : OverlappingActors)
    {
        ATornadoPhysicsObject* PhysicsObject = Cast<ATornadoPhysicsObject>(Actor);
        if (PhysicsObject)
        {
            PhysicsObject->ApplyTornadoForce(TornadoLocation, TornadoStrength);
            if (PhysicsObject->bCausesGrowth &&  !PhysicsObject->HasBeenCounted())
            {
                ObjectsHit++;
                PhysicsObject->SetHasBeenCounted();
                
                if (ObjectsHit >= GrowthThreshold)
                {
                    GrowTornado();
                    ObjectsHit = 0; // reset counter
                }
            }
            
            UE_LOG(LogTemp, Warning, TEXT("Applying force to %s"), *PhysicsObject->GetName());
        }
        
        DetectNearMiss(PhysicsObject);
        
    }
}

// Growth System

FTimerHandle GrowthTimerHandle;
bool bCanGrow = true;

void ATornadoPawn::GrowTornado()
{
    if (bCanGrow && CurrentSize < MaxSize)
    {
        bCanGrow = false;

        // Update target values for smooth interpolation
        TargetSize = CurrentSize * GrowthMultiplier;
        TargetMaxSpeed = MovementComponent->MaxSpeed * 0.9f;  // Reduce speed by 10%
        TargetAcceleration = MovementComponent->Acceleration * 0.85f; // Reduce acceleration by 15%
        TargetDrift = DriftFactor * 1.1f; // Increase drift by 10%
        TargetCollisionRadius = TornadoCollision->GetUnscaledSphereRadius() * GrowthMultiplier;

        // 0.7-second delay before allowing next growth
        GetWorld()->GetTimerManager().SetTimer(GrowthTimerHandle, [this]()
        {
            bCanGrow = true;
        }, 0.7f, false);

        UE_LOG(LogTemp, Warning, TEXT("Growth triggered: TargetSize=%f, TargetMaxSpeed=%f, TargetDrift=%f"), 
            TargetSize, TargetMaxSpeed, TargetDrift);
    }
}


// Scoring System

void ATornadoPawn::DetectNearMiss(AActor* Object)
{
    float Distance = FVector::Dist(Object->GetActorLocation(), GetActorLocation());

    if (Distance < NearMissDistance) 
    {
        NearMissBonus += 50.0f;
        UE_LOG(LogTemp, Warning, TEXT("Near miss with %s! +50 points"), *Object->GetName());
    }
}



// Blueprints controls

void ATornadoPawn::SetGrowthSettings(float NewMultiplier, float NewThreshold, float NewMaxSize)
{
    GrowthMultiplier = NewMultiplier;
    GrowthThreshold = FMath::Max(1.0f, NewThreshold); // Prevents division by zero
    MaxSize = NewMaxSize;

    UE_LOG(LogTemp, Warning, TEXT("Growth Settings Updated: Multiplier = %f, Threshold = %f, Max Size = %f"), 
        GrowthMultiplier, static_cast<float>(GrowthThreshold), MaxSize);
}

void ATornadoPawn::SetBoostSettings(float NewMultiplier, float NewDuration, float NewCooldown)
{
    BoostMultiplier = NewMultiplier;
    BoostDuration = NewDuration;
    BoostCooldown = NewCooldown;
    UE_LOG(LogTemp, Warning, TEXT("Boost Settings Updated: Multiplier = %f, Duration = %f, Cooldown = %f"), BoostMultiplier, BoostDuration, BoostCooldown);
}

void ATornadoPawn::SetTornadoStrength(float NewStrength, float NewCollisionRadius, float NewNearMissDistance)
{
    TornadoStrength = NewStrength;
    TornadoCollision->SetSphereRadius(NewCollisionRadius);

    if (NewNearMissDistance < 0)
    {
        NearMissDistance = NewCollisionRadius + 100;
    }
    else
    {
        NearMissDistance = NewNearMissDistance;
    }
    UE_LOG(LogTemp, Warning, TEXT("Tornado Strength Updated: Strength = %f, Collision Radius = %f, Near Miss Distance = %f"),
       TornadoStrength, NewCollisionRadius, NearMissDistance);
}

void ATornadoPawn::SetTornadoMovement(float NewMaxSpeed, float NewAcceleration, float NewDeceleration,
    float NewTurningBoost, float NewFriction, float NewDriftFactor)
{
    MovementComponent->MaxSpeed = NewMaxSpeed;
    MovementComponent->Acceleration = NewAcceleration;
    MovementComponent->Deceleration = NewDeceleration;
    MovementComponent->TurningBoost = NewTurningBoost;
    Friction = NewFriction;
    DriftFactor = NewDriftFactor;
    
    UE_LOG(LogTemp, Warning, TEXT("Tornado Movement Updated: MaxSpeed = %f, Acceleration = %f, Deceleration = %f, TurningBoost = %f, Friction = %f, DriftFactor = %f"),
       NewMaxSpeed, NewAcceleration, NewDeceleration, NewTurningBoost, NewFriction, NewDriftFactor);
}
