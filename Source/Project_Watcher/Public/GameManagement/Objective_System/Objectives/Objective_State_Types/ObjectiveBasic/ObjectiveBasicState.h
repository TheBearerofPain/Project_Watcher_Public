// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveState.h"
#include "ObjectiveBasic.h"
#include "ObjectiveBasicState.generated.h"

/**
 * A basic single state objective evaluator
 */
UCLASS(BlueprintType)
class PROJECT_WATCHER_API UObjectiveBasicState : public UObjectiveState , public IObjectiveBasic
{
	GENERATED_BODY()
private:
	
	EObjectiveState ObjectiveState = EObjectiveState::Pending;

public:

	virtual UObjectiveState * GetCopy() override;

	virtual void UpdateFromCopy(UObjectiveState* ObjectiveStateIn) override;

	virtual void SetPending() override;

	virtual void SetInProgress() override;
	
	virtual EObjectiveState GetObjectiveState_Implementation() override;

	virtual void SetObjectiveState_Implementation(const EObjectiveState ObjectiveStateIn) override;

	virtual EObjectiveState Evaluate() override;
};
