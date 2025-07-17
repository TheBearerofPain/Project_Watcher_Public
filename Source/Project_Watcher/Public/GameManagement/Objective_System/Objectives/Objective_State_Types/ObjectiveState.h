// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ObjectiveState.generated.h"

/**
 * Primitive objective completion states
 */
UENUM(BlueprintType)
enum EObjectiveState
{
	Pending,
	InProgress,
	Failed,
	Completed
};

/**
 * Primitive sub objective state
 */
USTRUCT(Blueprintable)
struct PROJECT_WATCHER_API FSubObjective
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubObjective")
	FString Title = TEXT("");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubObjective")
	FString Description = TEXT("");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SubObjective")
	TEnumAsByte<EObjectiveState> ObjectiveState = EObjectiveState::Pending;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubObjective")
	AActor * SubObjectiveMarker = nullptr;
};

/**
 * Generic Objective State Data
 */
UCLASS(BlueprintType, Abstract)
class PROJECT_WATCHER_API UObjectiveState : public UObject
{
	GENERATED_BODY()

public:
	
	/**
	 * Gets a Copy Object
	 * @return New UObjectiveState Object
	 */
	virtual UObjectiveState * GetCopy();

	/**
	 * Updates via Deep Copy with the given ObjectiveState
	 * @param ObjectiveStateIn ObjectiveState we are updating with
	 */
	virtual void UpdateFromCopy(UObjectiveState * ObjectiveStateIn);
	
	/**
	 * Sets the ObjectiveState to Pending
	 */
	virtual void SetPending();

	/**
	 * Sets the ObjectiveState to InProgress
	 */
	virtual void SetInProgress();

	/**
	 * Evaluates the ObjectiveState
	 * @return The State the Objective is in
	 */
	virtual EObjectiveState Evaluate();
};
