// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameManagement/Objective_System/Objectives/Objective.h"
#include "ObjectiveSupport.generated.h"

class UObjectiveStrictSubObjectiveState;
class UObjectiveLooseSubObjectiveState;
class UObjectiveBasicState;

/**
 * A group of support functions for creating and managing objectives
 */
UCLASS()
class PROJECT_WATCHER_API UObjectiveSupport : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * Sets the SuccessObjective in the Given Objective
	 * @param Objective The Objective we are setting the SuccessObjective in
	 * @param SuccessObjective The SuccessObjective
	 * @return The Objective with the newly set SuccessObjective
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjective * SetSuccessObjective(UObjective * Objective, UObjective * SuccessObjective);

	/**
	 * Sets the FailureObjective in the Given Objective
	 * @param Objective The Objective we are setting the FailureObjective in
	 * @param FailureObjective The FailureObjective
	 * @return The Objective with the newly set Failure Objective
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjective * SetFailureObjective(UObjective * Objective, UObjective * FailureObjective);

	/**
	 * Sets the ObjectiveMarker in the Given Objective
	 * @param Objective The Objective we are setting the ObjectiveMarker in
	 * @param Marker The ObjectiveMarker
	 * @return The Objective with the newly set ObjectiveMarker
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjective * SetMarker(UObjective * Objective, AActor * Marker);

	/**
	 * Sets Both the Success & Failure Objectives in the given Objective
	 * @param Objective The Objective we are updating
	 * @param SuccessObjective The SuccessObjective
	 * @param FailureObjective The FailureObjective
	 * @return The Objective with its Success & Failure Objectives Updated
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjective * SetSuccessAndFailureObjectives(UObjective * Objective, UObjective * SuccessObjective, UObjective * FailureObjective);
	
	/**
	 * Creates a Basic Objective State
	 * @return Initialized Basic Objective State
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjectiveBasicState * CreateBasicState();
	
	/**
	 * Creates a LooseSubObjective Objective State
	 * ONLY the CompletionThreshold amount of SubObjectives need to be completed for this to evaluate as complete
	 * @param SubObjectives SubObjectives
	 * @param CompletionThreshold CompletionThreshold
	 * @return Initialized LooseSubObjective State
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjectiveLooseSubObjectiveState * CreateLooseSubObjectiveState(const TArray<FSubObjective>& SubObjectives, const int32 CompletionThreshold);

	/**
	 * Creates a StrictSubObjective Objective State
	 * ALL SubObjectives must be complete in order for this to evaluate as completed
	 * @param SubObjectives  SubObjectives
	 * @return Initialized StrictSubObjective State
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjectiveStrictSubObjectiveState * CreateStrictSubObjectiveState(const TArray<FSubObjective>& SubObjectives);

	/**
	 * Creates a TArray<FSubObjective> From a given item
	 * @param SubObjective The first Item in our SubObjective list
	 * @return The newly created SubObjectiveList with our newly added SubObjective
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static TArray<FSubObjective> CreateSubObjectiveListFromItem(const FSubObjective& SubObjective);

	/**
	 * Creates an empty SubObjectiveList
	 * @return The empty SubObjectiveList
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static TArray<FSubObjective> CreateSubObjectiveList();

	/**
	 * Appends an Item to the Given SubObjective List
	 * @param SubObjectiveList The List we are appending to
	 * @param SubObjective The Item we are adding
	 * @return The appended SubObjectiveList
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static TArray<FSubObjective> AppendItemToSubObjectiveList(TArray<FSubObjective> SubObjectiveList, const FSubObjective& SubObjective);

	/**
	 * Appends a SubObjectiveList to Another
	 * @param SubObjectiveListDes The Destination SubObjectiveList
	 * @param SubObjectiveListSrc The Source SubObjectiveList
	 * @return The Resulting SubObjective list from Appending Src to Des
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static TArray<FSubObjective> AppendListToSubObjectiveList(TArray<FSubObjective> SubObjectiveListDes, const TArray<FSubObjective>& SubObjectiveListSrc);
};