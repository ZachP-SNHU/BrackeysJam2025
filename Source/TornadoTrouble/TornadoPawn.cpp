// Fill out your copyright notice in the Description page of Project Settings.


#include "TornadoPawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
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
	MovementComponent->MaxSpeed = MoveSpeed;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(TornadoMesh);
	SpringArm->TargetArmLength = 800.0f;
	SpringArm->SetRelativeRotation(FRotator(-45.0f, 0.0f, 0.0f));
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 5.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

// Called when the game starts or when spawned
void ATornadoPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATornadoPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ApplyDrift(DeltaTime);
	
	AddMovementInput(CurrentVelocity.GetSafeNormal(), 1.0f);

	FVector TornadoForward = GetActorForwardVector();
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
}

// Called to bind functionality to input
void ATornadoPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATornadoPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATornadoPawn::MoveRight);
}

void ATornadoPawn::MoveForward(float Value)
{
	MoveDirection.X = Value;
}

void ATornadoPawn::MoveRight(float Value)
{
	MoveDirection.Y = Value;
}

void ATornadoPawn::ApplyDrift(float DeltaTime)
{
	FVector TargetVelocity = MoveDirection * Acceleration * DeltaTime;

	CurrentVelocity += TargetVelocity;

	if (CurrentVelocity.Size() > MaxSpeed)
	{
		CurrentVelocity = CurrentVelocity.GetSafeNormal() * MaxSpeed;
	}

	CurrentVelocity *= (1.0f - (Friction * DeltaTime));

	FVector DriftOffset = FVector(-MoveDirection.Y, MoveDirection.X, 0.0f) * DriftFactor;
	CurrentVelocity += DriftOffset;
}

