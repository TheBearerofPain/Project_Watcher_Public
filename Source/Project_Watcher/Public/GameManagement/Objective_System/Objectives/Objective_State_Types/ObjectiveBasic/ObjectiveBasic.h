// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveState.h"
#include "ObjectiveBasic.generated.h"

// This class does not need to be modified.
UINTERFACE(Blueprintable)
class UObjectiveBasic : public UInterface
{
	GENERATED_BODY()
};

/**
 * Get & Set for ObjectiveBasicState
 */
class PROJECT_WATCHER_API IObjectiveBasic
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	EObjectiveState GetObjectiveState();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetObjectiveState(const EObjectiveState ObjectiveStateIn);
};
