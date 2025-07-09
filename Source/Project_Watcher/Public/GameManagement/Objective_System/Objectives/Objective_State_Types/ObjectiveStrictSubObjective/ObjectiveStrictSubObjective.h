// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveState.h"
#include "ObjectiveStrictSubObjective.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UObjectiveStrictSubObjective : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class PROJECT_WATCHER_API IObjectiveStrictSubObjective
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TArray<FSubObjective> GetSubObjectiveState();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetSubObjectiveState(const TArray<FSubObjective>& SubObjectiveStateIn);
};
