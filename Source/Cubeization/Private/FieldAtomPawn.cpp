// Fill out your copyright notice in the Description page of Project Settings.


#include "FieldAtomPawn.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "FieldGlobalData.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Windows/AllowWindowsPlatformTypes.h"

// Sets default values
AFieldAtomPawn::AFieldAtomPawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	RootComponent = MeshComponent;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
}

// Called when the game starts or when spawned
void AFieldAtomPawn::BeginPlay()
{
	Super::BeginPlay();
	PrimitiveComponent = GetComponentByClass<UPrimitiveComponent>();
	PrimitiveComponent->SetSimulatePhysics(true);
	PrimitiveComponent->SetEnableGravity(false);

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

UPrimitiveComponent* AFieldAtomPawn::GetMovementBase() const
{
	return PrimitiveComponent;
}

// Called every frame
void AFieldAtomPawn::Tick(float DeltaTime)
{
	auto FieldDataSubsystem = GetWorld()->GetSubsystem<UFieldDataSubsystem>();

	const FVector Location = GetActorLocation();
	const float FieldHeight = FieldDataSubsystem->GetHeight(Location);
	const float Height = Location.Z - FieldHeight - AtomRadius;

	float RepulsiveForce = 0;
	// force apply to field
	{
		float ZeroPoint = FieldBalanceFactor / 1.5f;
		float a = FieldAttractingFactor;
		float b = a * ZeroPoint;
		float h = Height;
		float h_2 = h * h;
		float h_3 = h_2 * h;
		float FieldForce = h > ZeroPoint
			                   ? FieldForceMultiplier * (a / h_2 - b / h_3)
			                   : MaxFieldRepulsiveForce * (h / ZeroPoint - 1);
		if (h > ZeroPoint)
		{
			RepulsiveForce = -FieldForce;
		}
		FieldDataSubsystem->ApplyForce(Location, FieldForce, 5);
	}

	// force apply to self
	{
		float NextHeight = FMath::Lerp(Height, TargetHeight, 0.1f);
		float Acceleration = (NextHeight - Height) / DeltaTime + RepulsiveForce;
		//movement force
		FVector Force(0, 0, Acceleration);

		FVector TargetSpeed = ControlInputVector * MovementMaxSpeed;
		ControlInputVector = FVector::ZeroVector;
		FVector DeltaSpeed = TargetSpeed - PrimitiveComponent->GetPhysicsLinearVelocity();
		//movement
		Force += DeltaSpeed * MovementForceMultiplier;

		PrimitiveComponent->AddForce(Force, NAME_None, true);
	}

	FieldDataSubsystem->UpdateCenter(Location);
}

// Called to bind functionality to input
void AFieldAtomPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AFieldAtomPawn::Jump);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFieldAtomPawn::Move);

		// //Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFieldAtomPawn::Look);
	}
}

void AFieldAtomPawn::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AFieldAtomPawn::Jump(const FInputActionValue& Value)
{
	//add jump force
	PrimitiveComponent->AddImpulse(FVector(0, 0, JumpImpulse), NAME_None, true);
}

void AFieldAtomPawn::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}
