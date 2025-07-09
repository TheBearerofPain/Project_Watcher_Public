// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "ObjectiveLooseSubObjective.h"
#include "UObject/Object.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveState.h"
#include "ObjectiveLooseSubObjectiveState.generated.h"

/**
 * List of SubObjectives that only SOME of them need to be completed
 * in order to evaluate the objective as completed
 */
UCLASS(BlueprintType)
class PROJECT_WATCHER_API UObjectiveLooseSubObjectiveState : public UObjectiveState, public IObjectiveLooseSubObjective
{
	GENERATED_BODY()
private:
	TArray<FSubObjective> SubObjectiveList;
	int32 CompletionThreshold = 1;
public:

	virtual void SetPending() override;

	virtual void SetInProgress() override;
	
	virtual TArray<FSubObjective> GetSubObjectiveState_Implementation() override;

	virtual void SetSubObjectiveState_Implementation(const TArray<FSubObjective>& SubObjectiveStateIn, const int32 CompletionThresholdIn) override;

	virtual EObjectiveState Evaluate() override;
};
