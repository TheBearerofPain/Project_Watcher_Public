// Copyright Epic Games, Inc. All Rights Reserved.

#include "Project_WatcherGameMode.h"
#include "Project_WatcherCharacter.h"
#include "UObject/ConstructorHelpers.h"

AProject_WatcherGameMode::AProject_WatcherGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
