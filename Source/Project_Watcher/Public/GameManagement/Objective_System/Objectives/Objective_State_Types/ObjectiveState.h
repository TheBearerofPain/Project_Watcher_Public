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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SubObjective")
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

	virtual void SetPending();

	virtual void SetInProgress();
	
	virtual EObjectiveState Evaluate();
};
