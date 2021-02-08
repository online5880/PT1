// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project_AACharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "ManiAnimInstance.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"

//////////////////////////////////////////////////////////////////////////
// AProject_AACharacter

AProject_AACharacter::AProject_AACharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);


	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 400.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->MaxWalkSpeed = 350.f;

	RunSpeed = 350.f;
	SprintSpeed = 600.f;

	bSprint = false;
	bRoll = false;
	bLMBDwon = false;
	bAttacking = false;
	bIsAttackButtonAttack = false;
	ComboCount = 0;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	FName WeaponSocket(TEXT("Sword_joint"));
	CombatCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CombatSphere"));
	CombatCapsule->SetupAttachment(GetMesh(), WeaponSocket);
	CombatCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//CombatCapsule->SetCollisionResponseToAllChannels(ECR_Overlap);
	//CombatCapsule->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	
}


//////////////////////////////////////////////////////////////////////////
// Input

void AProject_AACharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AProject_AACharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AProject_AACharacter::StopSprint);

	PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &AProject_AACharacter::StartRoll);
	PlayerInputComponent->BindAction("Roll", IE_Released, this, &AProject_AACharacter::StopRoll);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AProject_AACharacter::LMBDown);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &AProject_AACharacter::LMBUp);

	PlayerInputComponent->BindAxis("MoveForward", this, &AProject_AACharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProject_AACharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AProject_AACharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProject_AACharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AProject_AACharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AProject_AACharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AProject_AACharacter::OnResetVR);
}

void AProject_AACharacter::BeginPlay()
{
	Super::BeginPlay();

	CombatCapsule->OnComponentBeginOverlap.AddDynamic(this, &AProject_AACharacter::OnOverlapBegin);
	CombatCapsule->OnComponentEndOverlap.AddDynamic(this, &AProject_AACharacter::OnOverlapEnd);
}

void AProject_AACharacter::CollisionOn()
{
	CombatCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AProject_AACharacter::CollisionOff()
{
	CombatCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProject_AACharacter::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AProject_AACharacter* Main = Cast<AProject_AACharacter>(OtherActor);
	if (OtherActor == Main || OtherActor == GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("Main"));

		return;
		/*CombatCapsule->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Ignore);
		CombatCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);*/
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No Main"));
//		CombatCapsule->SetCollisionResponseToAllChannels(ECR_Overlap);

		UGameplayStatics::ApplyDamage(OtherActor, 20, NULL, GetOwner(), NULL);

	}
}

void AProject_AACharacter::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{


}

void AProject_AACharacter::StartSprint()
{
	bSprint = true;
	if (bSprint)
	{
		GetCharacterMovement()->MaxWalkSpeed = 600.f;
	}
}

void AProject_AACharacter::StopSprint()
{
	bSprint = false;
	GetCharacterMovement()->MaxWalkSpeed = 350.f;
}

void AProject_AACharacter::StartRoll()
{
	bRoll = true;
	if (bRoll)
	{

	}
}

void AProject_AACharacter::StopRoll()
{
	bRoll = false;
}

void AProject_AACharacter::Attack()
{
	bLMBDwon = true;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
	}
	if (!AnimInstance || !CombatMontage) return;

	bAttacking = true;

	const char* Attacklist[] = { "Attack_1","Attack_2","Attack_3" };

	if (!(AnimInstance->Montage_IsPlaying(CombatMontage)))
	{
		AnimInstance->Montage_Play(CombatMontage);
	}
	else if (AnimInstance->Montage_IsPlaying(CombatMontage))
	{
		AnimInstance->Montage_IsPlaying(CombatMontage);
		AnimInstance->Montage_JumpToSection(FName(Attacklist[ComboCount]), CombatMontage);
	}
}

void AProject_AACharacter::EndAttack()
{
	bAttacking = false;
}

void AProject_AACharacter::AttackInputChecking()
{
	if (ComboCount >= 2)
	{
		ComboCount = 0;
	}

	if (bIsAttackButtonAttack == true)
	{
		ComboCount += 1;
		bIsAttackButtonAttack = false;
		Attack();
	}
}

void AProject_AACharacter::LMBDown()
{
	bLMBDwon = true;

	if (bAttacking == false)
	{
		Attack();
	}
	else if(bAttacking == true)
	{
		bIsAttackButtonAttack = true;
	}
}

void AProject_AACharacter::LMBUp()
{
	bLMBDwon = false;
}


void AProject_AACharacter::OnResetVR()
{
	// If Project_AA is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in Project_AA.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AProject_AACharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AProject_AACharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AProject_AACharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AProject_AACharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AProject_AACharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AProject_AACharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}
