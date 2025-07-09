// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ObjectiveState.h"
#include "ObjectiveStrictSubObjectiveState.generated.h"

/**
 * List of SubObjectives that ALL need to be completed
 * in order to evaluate the objective as completed
 */
UCLASS(BlueprintType)
class PROJECT_WATCHER_API UObjectiveStrictSubObjectiveState : public UObjectiveState
{
	GENERATED_BODY()
private:
	TArray<FSubObjective> SubObjectiveList;
public:
	
	/**
	 * Returns the SubObjectiveList
	 * @return the SubObjectiveList
	 */
	TArray<FSubObjective> GetSubObjectiveList();

	/**
	 * Allows for the setting of the SubObjectiveList
	 * @param SubObjectiveListIn The new list data we are updating with
	 */
	UFUNCTION(BlueprintCallable, Category = "SubObjectiveList")
	void SetStrictSubObjectiveList(const TArray<FSubObjective>& SubObjectiveListIn);

	virtual EObjectiveState Evaluate() override;
};
