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
#include "Enemy.h"
#include "Sound/SoundCue.h"
#include "HitCameraShake.h"
#include "Camera/PlayerCameraManager.h"

//////////////////////////////////////////////////////////////////////////
// AProject_AACharacter

AProject_AACharacter::AProject_AACharacter()
{
	PrimaryActorTick.bCanEverTick = true;

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
	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	RunSpeed = 350.f;
	SprintSpeed = 600.f;

	Health = 30.f;
	MaxHealth = 100.f;
	Stamina = 50.f;
	MaxStamina = 200.f;
	Damage = 20.f;
	Potion = 8;

	bSprint = false;
	bRoll = false;
	bLMBDwon = false;
	bAttacking = false;
	bIsAttackButtonAttack = false;
	bDeath = false;
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


void AProject_AACharacter::EnemyHealth(float CurrentHealth,float EnemyMaxHealth)
{
	(CurrentHealth / EnemyMaxHealth);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AProject_AACharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AProject_AACharacter::StartJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AProject_AACharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AProject_AACharacter::StopSprint);

	PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &AProject_AACharacter::StartRoll);
	PlayerInputComponent->BindAction("Roll", IE_Released, this, &AProject_AACharacter::StopRoll);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AProject_AACharacter::LMBDown);
	PlayerInputComponent->BindAction("Attack", IE_Released, this, &AProject_AACharacter::LMBUp);

	PlayerInputComponent->BindAction("Potion", IE_Pressed, this, &AProject_AACharacter::DrinkPotion);

	PlayerInputComponent->BindAction("Block", IE_Pressed, this, &AProject_AACharacter::Block);
	PlayerInputComponent->BindAction("Block", IE_Released, this, &AProject_AACharacter::BlockOff);

	PlayerInputComponent->BindAxis("MoveForward", this, &AProject_AACharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AProject_AACharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AProject_AACharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AProject_AACharacter::LookUpAtRate);

}

void AProject_AACharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
	if (Stamina >= MaxStamina)
	{
		Stamina = MaxStamina;
	}

	if (bSprint)
	{
		Stamina -= DeltaTime * 20;
	}
	else
	{
		Stamina += DeltaTime * 20;
		if (Stamina >= MaxStamina)
		{
			Stamina = MaxStamina;
		}

	}
}

void AProject_AACharacter::BeginPlay()
{
	Super::BeginPlay();

	CombatCapsule->OnComponentBeginOverlap.AddDynamic(this, &AProject_AACharacter::OnOverlapBegin);
	CombatCapsule->OnComponentEndOverlap.AddDynamic(this, &AProject_AACharacter::OnOverlapEnd);
}

void AProject_AACharacter::DrinkPotion()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
		
		if (Health >= 100)
		{
			Health = MaxHealth;
		}
		if (Potion > 0)
		{
			Potion--;
			AnimInstance->Montage_Play(CombatMontage, 1.f);
			AnimInstance->Montage_JumpToSection(FName("Potion"), CombatMontage);
			Health += 10;
		}
		else 
		{
			Potion = 0;
		}
	}
}

void AProject_AACharacter::CollisionOn()
{
	CombatCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AProject_AACharacter::CollisionOff()
{
	CombatCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

float AProject_AACharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

		Health -= damage;

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance)
		{
			AnimInstance->Montage_Play(CombatMontage, 1.f);
			AnimInstance->Montage_JumpToSection(FName("React"), CombatMontage);
		}

		UE_LOG(LogTemp, Warning, TEXT("React"));

	if (Health <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Die"));
		Die();

		return damage;
	}
	return damage;
}

void AProject_AACharacter::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
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
	if (OtherActor)
	{
		Enemy = Cast<AEnemy>(OtherActor);
		if (Enemy)
		{
			UE_LOG(LogTemp, Warning, TEXT("No Main"));
			//CombatCapsule->SetCollisionResponseToAllChannels(ECR_Overlap);

			UGameplayStatics::ApplyDamage(Enemy, Damage, NULL, GetOwner(), NULL);
			//StartCameraShake(ShakeClass,1.0f,ECameraShakePlaySpace::CameraLocal,FRotator(0.f));

			//Camera->StartCameraShake(ShakeClass, 1.0f, ECameraShakePlaySpace::CameraLocal, FRotator(0.f));
			GetWorld()->GetFirstPlayerController()->PlayerCameraManager->PlayCameraShake(ShakeClass, 1.0f, ECameraShakePlaySpace::CameraLocal, FRotator(0.f));
		}

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
		GetCharacterMovement()->MaxWalkSpeed = 550.f;
	}
}

void AProject_AACharacter::StopSprint()
{
	bSprint = false;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
}

void AProject_AACharacter::StartJump()
{
	if (!bDeath && !bAttacking)
	{
		ACharacter::Jump();
	}
}

void AProject_AACharacter::Block()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Block"), CombatMontage);
	}
}

void AProject_AACharacter::BlockOff()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Idle"), CombatMontage);
	}
}

void AProject_AACharacter::StartRoll()
{
	bRoll = true;

	FVector Location1 = GetActorLocation();
	FVector Location2 = GetActorForwardVector();
	SetActorLocation(Location1 + Location2 * 200.f, true);
	
	Stamina -= 15.f;
}

void AProject_AACharacter::StopRoll()
{
	bRoll = false;
}



void AProject_AACharacter::Attack()
{
	bLMBDwon = true;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	
	if (!AnimInstance || !CombatMontage) return;

	bAttacking = true;

	const char* Attacklist[] = { "Attack_1","Attack_2","Attack_3" };

	int32 Section = FMath::RandRange(0, 2);

	//if (!(AnimInstance->Montage_IsPlaying(CombatMontage)))
	//{
	//	AnimInstance->Montage_Play(CombatMontage);
	//	//AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
	//}
	//else if (AnimInstance->Montage_IsPlaying(CombatMontage))
	//{
	//	AnimInstance->Montage_Play(CombatMontage);
	//	AnimInstance->Montage_JumpToSection(FName(Attacklist[Section]), CombatMontage);
	//}

	AnimInstance->Montage_Play(CombatMontage,1.2f);
	AnimInstance->Montage_JumpToSection(FName(Attacklist[Section]), CombatMontage);
}

void AProject_AACharacter::EndAttack()
{
	bAttacking = false;
	UE_LOG(LogTemp, Warning, TEXT("bAttacking = false"));
}

void AProject_AACharacter::AttackInputChecking()
{
	if (ComboCount >= 2)
	{
		ComboCount = 0;
	}

	if (bIsAttackButtonAttack == true && !bAttacking)
	{
		ComboCount += 1;
		bIsAttackButtonAttack = false;
		Attack();
	
		UE_LOG(LogTemp, Warning, TEXT("%d"), ComboCount);
	}
}

void AProject_AACharacter::LMBDown()
{
	bLMBDwon = true;

	if (bAttacking == false && (!bPressedJump))
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


void AProject_AACharacter::Die()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
		bDeath = true;
	}
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
	if ((Controller != nullptr) && (Value != 0.0f)&& (!bDeath) && (!bAttacking))
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
	if ( (Controller != nullptr) && (Value != 0.0f)&&(!bDeath)&& (!bAttacking) )
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
