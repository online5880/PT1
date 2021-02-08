// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ManiAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_AA_API UManiAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:

	UManiAnimInstance();

	virtual void NativeInitializeAnimation() override; // 애니메이션 초기화

	UFUNCTION(BlueprintCallable, Category = AnimationProperties)
	void UpdateAnimationProperties();

	UPROPERTY(EditAnywhere,BlueprintReadOnly,Category="Movement")
	float MovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	bool bIsAir;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	class AProject_AACharacter* Main;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	class APawn* Pawn;
	
};
