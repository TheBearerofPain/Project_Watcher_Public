// Project Watcher 2024 & Beyond.


#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveBasicState.h"

EObjectiveState UObjectiveBasicState::GetObjectiveState() const
{
	return this->ObjectiveState;
}

void UObjectiveBasicState::SetObjectiveState(const EObjectiveState ObjectiveStateIn)
{
	this->ObjectiveState = ObjectiveStateIn;
}

EObjectiveState UObjectiveBasicState::Evaluate()
{
	return this->ObjectiveState;
}