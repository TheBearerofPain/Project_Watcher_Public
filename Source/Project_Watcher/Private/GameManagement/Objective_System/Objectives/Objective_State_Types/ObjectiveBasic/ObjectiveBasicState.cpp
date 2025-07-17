// Project Watcher 2024 & Beyond.


#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveBasic/ObjectiveBasicState.h"

UObjectiveState* UObjectiveBasicState::GetCopy()
{
	UObjectiveBasicState * Copy = NewObject<UObjectiveBasicState>();

	Copy->ObjectiveState = this->ObjectiveState;
	
	return Copy;
}

void UObjectiveBasicState::UpdateFromCopy(UObjectiveState* ObjectiveStateIn)
{
	if (ObjectiveStateIn)
	{
		if (const UObjectiveBasicState * Source = Cast<UObjectiveBasicState>(ObjectiveStateIn))
		{
			this->ObjectiveState = Source->ObjectiveState;
		}
	}
}

void UObjectiveBasicState::SetPending()
{
	this->ObjectiveState = EObjectiveState::Pending;
}

void UObjectiveBasicState::SetInProgress()
{
	this->ObjectiveState = EObjectiveState::InProgress;
}

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
