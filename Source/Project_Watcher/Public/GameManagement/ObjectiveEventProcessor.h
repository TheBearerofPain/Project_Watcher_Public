// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ObjectiveEventProcessor.generated.h"

USTRUCT(Blueprintable)
struct PROJECT_WATCHER_API FSubObjective
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubObjective")
	FString Title = TEXT("");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubObjective")
	FString Description = TEXT("");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubObjective")
	bool Completed = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubObjective")
	AActor * SubObjectiveMarker = nullptr;
};

UCLASS(Blueprintable)
class PROJECT_WATCHER_API UObjective : public UObject
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	int32 WID = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FString Title = TEXT("");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FString Description = TEXT("");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	int64 StartTime = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	int64 EndTime = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	bool Completed = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SubObjective")
	TArray<FSubObjective> SubObjectives;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	AActor * ObjectiveMarker = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	UObjective * SuccessObjective = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	UObjective * FailureObjective = nullptr;
	
public:
	
	void SetObjectiveData(const int32 WIDIn, const FString& TitleIn, const FString& DescriptionIn, const int64 StartTimeIn, const int64 EndTimeIn);
	
	void SetSubObjectiveData(const TArray<FSubObjective>& SubObjectivesIn);
	
	void SetObjectiveMarkerData(AActor * ObjectiveMarkerIn);

	void SetSuccessObjectiveData(UObjective * SuccessObjectiveIn);
	
	void SetFailureObjectiveData(UObjective * FailureObjectiveIn);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectiveDelegate, UObjective *, Objective);

/**
 * Used to deal with the various objectives & keep track of their states
 */
UCLASS()
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

	//Objective Timer Handle for when the next soonest objective expires
	FTimerHandle RunningObjectiveTimerHandle;

	//Objective Timer Handle for when the next soonest objective starts
	FTimerHandle PendingObjectiveTimerHandle;
	
	//Private//

public:

	//Objective Delegate fires when an objective's completed state has been evaluated as true
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Objective")
	FObjectiveDelegate ObjectiveCompleteDelegate;
	
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
	void AddObjective(UObjective * NewObjective);

	/**
	 * Removes an objective from the processor
	 * DOES NOT fire any associated events if it happens to be complete or expired
	 * @param WID The event World ID we are removing
	 */
	void RemoveObjective(const int32 WID);

	/**
	 * Looks through Running Objectives & updates it's completion state
	 * IF it becomes true, the ObjectiveCompleteDelegate will be fired & it will be removed
	 * from the RunningObjectives ProcessorList
	 * @param WID The WID of the objective we are updating
	 * @param Completed The Updated Completion state
	 */
	void UpdateObjectiveCompletionState(const int32 WID, const bool Completed);

	/**
	 * Looks through Running Objectives & updates it's completion state
	 * IF it becomes true, the ObjectiveCompleteDelegate will be fired & it will be removed
	 * from the RunningObjectives ProcessorList
	 * @param WID The WID of the objective we are updating
	 * @param SubObjectives The Updated completion state of the subobjectives
	 */
	void UpdateObjectiveCompletionState(const int32 WID, const TArray<FSubObjective>& SubObjectives);

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
	static bool EvaluateObjective(UObjective * Objective);
};
