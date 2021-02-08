// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project_AAGameMode.h"
#include "Project_AACharacter.h"
#include "UObject/ConstructorHelpers.h"

AProject_AAGameMode::AProject_AAGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
