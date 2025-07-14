// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameManagement/Objective_System/Objectives/Objective.h"
#include "ObjectiveSupport.generated.h"

/**
 * A group of support functions for creating and managing objectives
 */
UCLASS()
class PROJECT_WATCHER_API UObjectiveSupport : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	/**
	 * Creates a Basic Objective Type
	 * @param WID WorldID of the objective
	 * @param Title Title of the Objective
	 * @param Description Description of the Objective
	 * @param ObjectiveTime The Time when the objective will start and end (endTime gets ignored if NoTimer is true)
	 * @return Initialized Basic Objective
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjective * CreateBasicObjective(const int32 WID, const FString& Title, const FString& Description, const FObjectiveTime& ObjectiveTime);

	/**
	 * Sets the SuccessObjective in the Given Objective
	 * @param Objective The Objective we are setting the SuccessObjective in
	 * @param SuccessObjective The SuccessObjective
	 * @return The Objective with the newly set SuccessObjective
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjective * AddSuccessObjective(UObjective * Objective, UObjective * SuccessObjective);

	/**
	 * Sets the FailureObjective in the Given Objective
	 * @param Objective The Objective we are setting the FailureObjective in
	 * @param FailureObjective The FailureObjective
	 * @return The Objective with the newly set Failure Objective
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjective * AddFailureObjective(UObjective * Objective, UObjective * FailureObjective);

	/**
	 * Sets the ObjectiveMarker in the Given Objective
	 * @param Objective The Objective we are setting the ObjectiveMarker in
	 * @param Marker The ObjectiveMarker
	 * @return The Objective with the newly set ObjectiveMarker
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjective * AddMarker(UObjective * Objective, AActor * Marker);

	/**
	 * Sets Both the Success & Failure Objectives in the given Objective
	 * @param Objective The Objective we are updating
	 * @param SuccessObjective The SuccessObjective
	 * @param FailureObjective The FailureObjective
	 * @return The Objective with its Success & Failure Objectives Updated
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjective * AddSuccessAndFailureObjectives(UObjective * Objective, UObjective * SuccessObjective, UObjective * FailureObjective);
	
	/**
	 * Creates a LooseSubObjective Objective Type
	 * @param WID WorldID of the objective
	 * @param Title Title of the Objective
	 * @param Description Description of the Objective
	 * @param SubObjectives The Objective List for this objective
	 * @param CompletionThreshold How many sub objectives need to be complete in order for this objective to be considered complete
	 * @param ObjectiveTime The Time when the objective will start and end (endTime gets ignored if NoTimer is true)
	 * @return Initialized LooseSubObjective Type
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjective * CreateLooseSubObjective(const int32 WID, const FString& Title, const FString& Description, const TArray<FSubObjective>& SubObjectives, const int32 CompletionThreshold, const FObjectiveTime& ObjectiveTime);

	/**
	 * Creates a StrictSubObjective Objective Type , ALL SubObjectives need to be complete in order for this objective to be complete
	 * @param WID WorldID of the objective
	 * @param Title Title of the Objective
	 * @param Description Description of the Objective
	 * @param SubObjectives The Objective List for this objective
	 * @param ObjectiveTime The Time when the objective will start and end (endTime gets ignored if NoTimer is true)
	 * @return Initialized StrictSubObjective Type
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective System")
	static UObjective * CreateStrictSubObjective(const int32 WID, const FString& Title, const FString& Description, const TArray<FSubObjective>& SubObjectives, const FObjectiveTime& ObjectiveTime);

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