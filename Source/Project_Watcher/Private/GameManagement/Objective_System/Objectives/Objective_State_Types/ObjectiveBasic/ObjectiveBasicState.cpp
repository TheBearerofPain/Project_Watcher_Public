// Project Watcher 2024 & Beyond.


#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveBasic/ObjectiveBasicState.h"

EObjectiveState UObjectiveBasicState::GetObjectiveState_Implementation()
{
	return this->ObjectiveState;
}

void UObjectiveBasicState::SetObjectiveState_Implementation(const EObjectiveState ObjectiveStateIn)
{
	this->ObjectiveState = ObjectiveStateIn;
}

EObjectiveState UObjectiveBasicState::Evaluate()
{
	return this->ObjectiveState;
}