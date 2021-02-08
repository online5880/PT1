// Fill out your copyright notice in the Description page of Project Settings.


#include "ManiAnimInstance.h"
#include "Project_AACharacter.h"
#include "GameFramework/PawnMovementComponent.h"

UManiAnimInstance::UManiAnimInstance()
{
	MovementSpeed = 0.f;
	bIsAir = false;
}

void UManiAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			Main = Cast<AProject_AACharacter>(Pawn);
		}
	}
}

void UManiAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
	}

	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity();
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size();

		bIsAir = Pawn->GetMovementComponent()->IsFalling();

		if (Main == nullptr)
		{
			Main = Cast<AProject_AACharacter>(Pawn);
		}
	}
}
