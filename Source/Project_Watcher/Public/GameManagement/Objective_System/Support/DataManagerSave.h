// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameManagement/Objective_System/Data_Manager/DataManager.h"
#include "GameManagement/Objective_System/Objectives/Objective.h"
#include "DataManagerSave.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_WATCHER_API UDataManagerSave : public USaveGame
{
	GENERATED_BODY()
	
public:

	//Save//

	static constexpr int32 UserIndex = 0;
	
	static const inline FString SaveSlot = TEXT("DataManagerState");

	//Save//

	//State//

	/* SaveState of DataManager */
	UPROPERTY(VisibleAnywhere, SaveGame)
	FDataManagerSaveState SaveState;
	
	//State//
	
public:
	
	UDataManagerSave();
};
