// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Project_AACharacter.generated.h"

UCLASS(config=Game)
class AProject_AACharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
public:
	AProject_AACharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemy* Enemy;

	UFUNCTION(BlueprintCallable)
	void EnemyHealth(float CurrentHealth, float EnemyMaxHealt);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<class UHitCameraShake> ShakeClass;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);

	void MoveRight(float Value);

	void TurnAtRate(float Rate);

	void LookUpAtRate(float Rate);

	/**************************** 스탯 *********************************/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Stamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float Damage;
	/**************************** 달리기 / 점프 *************************/
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,Category="Movement")
	bool bSprint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float RunSpeed;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	float SprintSpeed;

	UFUNCTION()
	void StartSprint();

	UFUNCTION()
	void StopSprint();

	UFUNCTION()
	void StartJump();
	/****************************** 방어 **********************************/
	
	void Block();

	void BlockOff();

	/****************************** 구르기 ********************************/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bRoll;

	UFUNCTION()
	void StartRoll();

	UFUNCTION()
	void StopRoll();
	/****************************** 공격 모션 / 포션 ***********************/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bAttacking;

	bool bIsAttackButtonAttack;
	int ComboCount;
	bool bLMBDwon;

	void Attack();

	UFUNCTION(BlueprintCallable)
	void EndAttack();

	UFUNCTION(BlueprintCallable)
	void AttackInputChecking();

	void LMBDown();

	void LMBUp();

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Combat")
	class UAnimMontage* CombatMontage;

	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	bool bDeath;

	bool getDeath() { return bDeath; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Items")
	uint8 Potion;

	UFUNCTION(BlueprintCallable)
	void DrinkPotion();
	/****************************** 공격 효과 ********************************/ 
	UPROPERTY(VisibleAnywhere,BlueprintReadWrite,Category="Combat")
	class UCapsuleComponent* CombatCapsule;

	UFUNCTION(BlueprintCallable)
	void CollisionOn();

	UFUNCTION(BlueprintCallable)
	void CollisionOff();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	class USoundCue* HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Sound")
	USoundCue* SwingSound;

	/******************************** Overlap ********************************/
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface

public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual void Tick(float DeltaTime) override;
};

