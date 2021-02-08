// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Project_AACharacter.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCharacterMovement()->MaxWalkSpeed = 350.f;

	MaxHealth = 200.f;
	Health = 20.f;

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
	}
	if (!AnimInstance || !EnemyAnim) return;


	const char* Attacklist[] = { "Attack_1","Attack_2","Attack_3" };

	if (!(AnimInstance->Montage_IsPlaying(EnemyAnim)))
	{
		AnimInstance->Montage_Play(EnemyAnim);
	}
	else if (AnimInstance->Montage_IsPlaying(EnemyAnim))
	{
		AnimInstance->Montage_IsPlaying(EnemyAnim);
	}
}

