// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveState.h"
#include "ObjectiveStrictSubObjective.h"
#include "ObjectiveStrictSubObjectiveState.generated.h"

/**
 * List of SubObjectives that ALL need to be completed
 * in order to evaluate the objective as completed
 */
UCLASS(BlueprintType)
class PROJECT_WATCHER_API UObjectiveStrictSubObjectiveState : public UObjectiveState , public IObjectiveStrictSubObjective
{
	GENERATED_BODY()
private:
	TArray<FSubObjective> SubObjectiveList;
public:

	virtual UObjectiveState * GetCopy() override;

	virtual void UpdateFromCopy(UObjectiveState* ObjectiveStateIn) override;
	
	virtual void SetPending() override;
	
	virtual void SetInProgress() override;
	
	virtual TArray<FSubObjective> GetSubObjectiveState_Implementation() override;

	virtual void SetSubObjectiveState_Implementation(const TArray<FSubObjective>& SubObjectiveStateIn) override;

	virtual EObjectiveState Evaluate() override;
};
