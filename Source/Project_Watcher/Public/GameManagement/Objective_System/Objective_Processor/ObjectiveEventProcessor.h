// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameManagement/Objective_System/Objectives/Objective.h"
#include "ObjectiveEventProcessor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectiveDelegate, UObjective *, Objective);

/**
 * Used to deal with the various objectives & keep track of their states
 */
UCLASS(Blueprintable)
class PROJECT_WATCHER_API UObjectiveEventProcessor : public UObject
{
	GENERATED_BODY()
private:
	//Private//
	
	//Running Objective List
	UPROPERTY()
	TArray<UObjective*> RunningObjectives;

	//Pending Objective List
	UPROPERTY()
	TArray<UObjective*> PendingObjectives;

	//No Timer Objective List
	UPROPERTY()
	TArray<UObjective*> NoTimerObjectives;

	//Objective Timer Handle for when the next soonest objective expires
	FTimerHandle RunningObjectiveTimerHandle;

	//Objective Timer Handle for when the next soonest objective starts
	FTimerHandle PendingObjectiveTimerHandle;
	
	//Private//

public:

	//Objective Delegate fires when an objective's completed state has been evaluated as completed
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Objective")
	FObjectiveDelegate ObjectiveCompleteDelegate;

	//Objective Delegate fires when an objective's completed state has been evaluated as failed
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Objective")
	FObjectiveDelegate ObjectiveFailedDelegate;
	
	//Objective Delegate fires when an objective's timer has expired
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Objective")
	FObjectiveDelegate ObjectiveExpiredDelegate;

	//Objective Delegate fires when an objective's timer has started
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Objective")
	FObjectiveDelegate ObjectiveStartedDelegate;

	/**
	 * Adds a new objective to the processor
	 * @param NewObjective The Objective we are adding
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void AddObjective(UObjective * NewObjective);

	/**
	 * Removes an objective from the processor
	 * DOES NOT fire any associated events if it happens to be complete or expired
	 * @param WID The event World ID we are removing
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void RemoveObjective(const int32 WID);

	/**
	 * Updates the Objective with the matching WID, with the given state, We ONLY search running objectives to update
	 * @param WID The World ID we are looking for
	 * @param ObjectiveStateIn The State we are updating with
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void UpdateObjectiveState(const int32 WID, UObjectiveState * ObjectiveStateIn);

private:
	
	/**
	 * Adds an Objective to the RunningObjective List
	 * @param NewObjective The Objective we are adding
	 */
	void AddObjectiveRunning(UObjective * NewObjective);

	/**
	 * Adds an Objective to the PendingObjective List
	 * @param NewObjective The Objective we are adding
	 */
	void AddObjectivePending(UObjective * NewObjective);

	/**
	 * Adds an Objective to the NoTimerObjective List
	 * @param NewObjective The Objective we are adding
	 */
	void AddObjectiveNoTimer(UObjective * NewObjective);
	
	/**
	 * Sets the timer until the next objective IF any
	 * otherwise it'll just clear the timer
	 */
	void UpdateRunningTimerUntilNextObjective();

	/**
	 * Sets the timer until the next pending objective is ready to
	 * be added to the running list. IF no other pending objectives exist
	 * then the timer will be cleared
	 */
	void UpdatePendingTimerUntilNextObjective();

	/**
	 * Gets called when the ObjectiveTimer finishes
	 * This will grab the [0] indexed objective and pass it along the
	 * TimerExpiredDelegate
	 */
	UFUNCTION()
	void ObjectiveRunningTimerComplete();

	/**
	 * Gets called when the PendingObjectiveTimer finishes
	 * This will grab the [0] indexed objective and pass it to the
	 * RunningObjective List
	 */
	UFUNCTION()
	void ObjectivePendingTimerComplete();

	/**
	 * Used to CallObjectiveCompleteDelegate
	 * @param Objective The Objective we are broadcasting as completed
	 */
	void CallObjectiveCompleteDelegate(UObjective * Objective) const;

	/**
	 * Used to CallObjectiveFailedDelegate
	 * @param Objective The Objective we are broadcasting as failed
	 */
	void CallObjectiveFailedDelegate(UObjective * Objective) const;

	/**
	 * Used to CallObjectiveTimerExpiredDelegate
	 * @param Objective The Objective we are broadcasting as expired
	 */
	void CallObjectiveTimerExpiredDelegate(UObjective * Objective) const;

	/**
	 * Used to CallObjectiveStartedDelegate
	 * @param Objective The Objective we are broadcasting as started
	 */
	void CallObjectiveTimerStartedDelegate(UObjective * Objective) const;

	/**
	 * Evaluates an objectives completion state
	 * @param Objective The objective we are evaluating
	 * @return True if the objective is complete, False if the objective is NOT complete
	 */
	static EObjectiveState EvaluateObjective(const UObjective * Objective);
};