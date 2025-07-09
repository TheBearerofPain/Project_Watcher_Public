// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ObjectiveState.h"
#include "ObjectiveBasicState.generated.h"

/**
 * A basic single state objective evaluator
 */
UCLASS(BlueprintType)
class PROJECT_WATCHER_API UObjectiveBasicState : public UObjectiveState
{
	GENERATED_BODY()
private:
	EObjectiveState ObjectiveState = EObjectiveState::Pending;
public:
	
	EObjectiveState GetObjectiveState() const;

	UFUNCTION(BlueprintCallable, Category = "ObjectiveState")
	void SetObjectiveState(const EObjectiveState ObjectiveStateIn);

	virtual EObjectiveState Evaluate() override;
};
