// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ObjectiveState.h"
#include "ObjectiveLooseSubObjectiveState.generated.h"

/**
 * List of SubObjectives that only SOME of them need to be completed
 * in order to evaluate the objective as completed
 */
UCLASS(BlueprintType)
class PROJECT_WATCHER_API UObjectiveLooseSubObjectiveState : public UObjectiveState
{
	GENERATED_BODY()
private:
	TArray<FSubObjective> SubObjectiveList;
	int32 CompletionThreshold = 1;
public:
	
	/**
	 * Returns the SubObjectiveList
	 * @return the SubObjectiveList
	 */
	TArray<FSubObjective> GetSubObjectiveList();

	/**
	 * Allows for the setting of the SubObjectiveList
	 * @param SubObjectiveListIn The new list data we are updating with
	 * @param CompletionThresholdIn The Threshold for evaluating as complete
	 */
	UFUNCTION(BlueprintCallable, Category = "SubObjectiveList")
	void SetLooseSubObjectiveList(const TArray<FSubObjective>& SubObjectiveListIn, const int32 CompletionThresholdIn);

	virtual EObjectiveState Evaluate() override;
};
