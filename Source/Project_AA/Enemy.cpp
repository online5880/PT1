// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Project_AACharacter.h"
#include "AIController.h"
#include "Engine/Classes/Components/SphereComponent.h"
#include "AITypes.h"
#include "AI/Navigation/NavigationTypes.h"
#include "Math/UnrealMathUtility.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Actor.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Project_AACharacter.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroShphere"));
	AgroSphere->SetupAttachment(RootComponent);
	AgroSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(RootComponent);
	CombatSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CombatSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	CombatCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CombatCapsule"));
	CombatCapsule->SetupAttachment(GetMesh(),FName("weapon_r"));
	CombatCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCharacterMovement()->MaxWalkSpeed = 250.f;

	MaxHealth = 200.f;
	Health = 20.f;

	Damage = 20.f;

	bAttacking = false;
	bOverappingCombatSphere = false;
	bTarget = false;

	EnemyState = EEnemyState::EES_Idle;

	AttackMinTime = 1.f;
	AttackMaxTime = 2.f;

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSpherenOnOverlapEnd);

	CombatCapsule->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatCollisionOnOverlapBegin);
	CombatCapsule->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatCollisionOnOverlapEnd);
	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	if (bTarget&&!bAttacking)
	{
		MoveToTarget(Main);
		if (bAttacking)
		{
			AIController->StopMovement();
		}
	}
	
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

float AEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	Health -= damage;

	if (Health <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Die"));
		
		Die();

		return damage;
	}
	return damage;
}

void AEnemy::Die()
{
	SetEnemyStatus(EEnemyState::EES_Death);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(EnemyAnim, 1.f);
		AnimInstance->Montage_JumpToSection(FName("Death"), EnemyAnim);
		SetActorEnableCollision(false);
	}
}


void AEnemy::MoveToTarget(class AProject_AACharacter* Target)
{
	SetEnemyStatus(EEnemyState::EES_MoveToTarget);

	if (AIController && bTarget)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(10.0f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);

	////	/** °æ·Î º¸¿©ÁÜ
	//	auto PathPonints = NavPath->GetPathPoints();
	//	for (auto Point : PathPonints)
	//	{
	//		FVector Location = Point.Location;

	//		UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 8, FLinearColor::Red, 10.f, 1.5f);
	//	}
	
	}
}

void AEnemy::CollisionOn()
{
	CombatCapsule->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::CollisionOff()
{
	CombatCapsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		Main = Cast<AProject_AACharacter>(OtherActor);
		if (Main)
		{
			bTarget = true;
			if (bAttacking == false)
			{
				MoveToTarget(Main);
			}
		}
	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

	if (AIController)
	{
		
		AIController->StopMovement();
		SetEnemyStatus(EEnemyState::EES_Idle);
		bTarget = false;
	}
}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		Main = Cast<AProject_AACharacter>(OtherActor);
		if (Main)
		{
			bOverappingCombatSphere = true;
			if (bTarget&&bOverappingCombatSphere)
			{
				Attack();
				float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
				GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime, true);
				//MoveToTarget(Main);
				UE_LOG(LogTemp, Warning, TEXT("CombatSphereOnOverlapBegin"));
			}
			
		}
	}
}

void AEnemy::CombatSpherenOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		Main = Cast<AProject_AACharacter>(OtherActor);
		if (Main)
		{
			AIController->StopMovement();
			bOverappingCombatSphere = false;
			if (bTarget)
			{
				MoveToTarget(Main);
				GetWorldTimerManager().ClearTimer(AttackTimer);
				SetEnemyStatus(EEnemyState::EES_MoveToTarget);
				UE_LOG(LogTemp, Warning, TEXT("CombatSpherenOnOverlapEnd"));
			}
		}
	}
}

void AEnemy::CombatCollisionOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		Main = Cast<AProject_AACharacter>(OtherActor);
		if (Main)
		{
			UGameplayStatics::ApplyDamage(Main, Damage, NULL, GetOwner(), NULL);
			//MoveToTarget(Main);
			UE_LOG(LogTemp, Warning, TEXT("Hit"));
		}
	}
}

void AEnemy::CombatCollisionOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}

void AEnemy::Attack()
{
	SetEnemyStatus(EEnemyState::EES_Attack);
	AIController->StopMovement();
	//AIController->StopMovement();
	if (!bAttacking && (!Main->GetMesh()->bNoSkeletonUpdate))
	{
		bAttacking = true;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance)
		{
			AnimInstance->Montage_Play(EnemyAnim, 1.f);
			AnimInstance->Montage_JumpToSection(FName("Attack_1"), EnemyAnim);
		
		}
	}
}

void AEnemy::EndAttack()
{
	bAttacking = false;
	if (bOverappingCombatSphere)
	{
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime,true);
	}
}
