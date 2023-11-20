// Fill out your copyright notice in the Description page of Project Settings.


#include "FieldAtomCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "..\DynamicTerrain\TerrainDataSubsystem.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Windows/AllowWindowsPlatformTypes.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Ability/CubeGameplayAbility.h"
#include "Components/CapsuleComponent.h"
#include "Controller/CubeizationPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AFieldAtomCharacter::AFieldAtomCharacter()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("MeshComponent");
	MeshComponent->SetupAttachment(RootComponent);
	
	// GetCapsuleComponent()->Deactivate();

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement]
	GetCharacterMovement()->MovementState.bCanFly = true;
	GetCharacterMovement()->MovementMode = EMovementMode::MOVE_Flying;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate
	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->GravityScale = 0;

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

	// ability
	AbilitySystem = CreateDefaultSubobject<UAbilitySystemComponent>("AbilitySystem");
	AbilitySystem->SetIsReplicated(true);

	AttributeSet = CreateDefaultSubobject<UAttributeSet>("AttributeSet");
}

// Called when the game starts or when spawned
void AFieldAtomCharacter::BeginPlay()
{
	Super::BeginPlay();
	PrimitiveComponent = GetComponentByClass<UPrimitiveComponent>();
	// PrimitiveComponent->SetSimulatePhysics(true);
	// PrimitiveComponent->SetEnableGravity(false);
	PrimitiveComponent->SetCollisionObjectType(ECC_PhysicsBody);

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	for (TSubclassOf<UGameplayAbility>& StartupAbility : Abilities)
	{
		GetAbilitySystemComponent()->GiveAbility(FGameplayAbilitySpec(StartupAbility));
	}
}

UAbilitySystemComponent* AFieldAtomCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystem;
}

void AFieldAtomCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystem)
	{
		AbilitySystem->InitAbilityActorInfo(this, this);
	}

	SetOwner(NewController);
}

// Called every frame
void AFieldAtomCharacter::Tick(float DeltaTime)
{
	FieldInteractionTick(DeltaTime);
	DirectionalMovementTick(DeltaTime);
}

// Called to bind functionality to input
void AFieldAtomCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AFieldAtomCharacter::Jump);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AFieldAtomCharacter::Move);

		// //Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AFieldAtomCharacter::Look);
	}

	AbilitySystem->BindToInputComponent(PlayerInputComponent);
	// AbilitySystem->BindAbilityActivationToInputComponent(PlayerInputComponent,
	// 	FGameplayAbilityInputBinds(
	// 		"ConfirmInput",
	// 		"CancelInput",
	// 		 "EAbilityInputID"));
}

void AFieldAtomCharacter::FieldInteractionTick(float DeltaTime)
{
	auto FieldDataSubsystem = GetWorld()->GetSubsystem<UTerrainDataSubsystem>();

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

	//force apply to self
	{
		float NextHeight = FMath::Lerp(Height, TargetHeight, 0.2f);
		float Acceleration = (NextHeight - Height) / DeltaTime + RepulsiveForce * 0.5f;
		//movement force
		FVector Force(0, 0, Acceleration);
	
		// FVector TargetSpeed = ControlInputVector * MovementMaxSpeed;
		// ControlInputVector = FVector::ZeroVector;
		// FVector DeltaSpeed = TargetSpeed - PrimitiveComponent->GetPhysicsLinearVelocity();
		// //movement
		// Force += DeltaSpeed * MovementForceMultiplier;

		// AddMovementInput(Force);

		
	
		// PrimitiveComponent->AddForce(Force, NAME_None, true);
	}

	FieldDataSubsystem->UpdateCenter(Location);
}

void AFieldAtomCharacter::DirectionalMovementTick(float DeltaTime)
{
	auto PC = Cast<ACubeizationPlayerController>(GetController());
	if(PC == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Controller is not a CubeizationPlayerController"));
		return;
	}
	auto TargetPosition = PC->GetPointingPosition();
	
	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetPosition);
	FRotator CurrentRotation = GetActorRotation();
	FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaTime, 10);

	// DrawDebugLine(GetWorld(), GetActorLocation(), TargetPosition, FColor::Red, false, 0.1f, 0, 5.f);
	// DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 100, FColor::Green, false, 0.1f, 0, 5.f);
	// //line of target rotation
	// DrawDebugLine(GetWorld(), GetActorLocation(), GetActorLocation() + NewRotation.Vector() * 100, FColor::Blue, false, 0.1f, 0, 5.f);
	//
	SetActorRotation(NewRotation);
}

void AFieldAtomCharacter::Move(const FInputActionValue& Value)
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

// void AFieldAtomPawn::Jump(const FInputActionValue& Value)
// {
// 	//add jump force
// 	PrimitiveComponent->AddImpulse(FVector(0, 0, JumpImpulse), NAME_None, true);
// }

void AFieldAtomCharacter::Look(const FInputActionValue& Value)
{
	// // input is a Vector2D
	// FVector2D LookAxisVector = Value.Get<FVector2D>();
	//
	// if (Controller != nullptr)
	// {
	// 	// add yaw and pitch input to controller
	// 	AddControllerYawInput(LookAxisVector.X);
	// 	AddControllerPitchInput(LookAxisVector.Y);
	// }
}
