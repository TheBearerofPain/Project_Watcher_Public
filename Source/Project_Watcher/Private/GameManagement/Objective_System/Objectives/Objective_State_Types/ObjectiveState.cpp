// Project Watcher 2024 & Beyond.

#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveState.h"

UObjectiveState* UObjectiveState::GetCopy()
{
	UObjectiveState * Copy = NewObject<UObjectiveState>();
	return Copy;
}

void UObjectiveState::UpdateFromCopy(UObjectiveState* ObjectiveStateIn){}

void UObjectiveState::SetPending(){}

void UObjectiveState::SetInProgress(){}

EObjectiveState UObjectiveState::Evaluate()
{
	return EObjectiveState::Pending;
}