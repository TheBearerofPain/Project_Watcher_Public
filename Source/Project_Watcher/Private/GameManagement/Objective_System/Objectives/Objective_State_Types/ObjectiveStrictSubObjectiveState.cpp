// Project Watcher 2024 & Beyond.

#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveStrictSubObjectiveState.h"

TArray<FSubObjective> UObjectiveStrictSubObjectiveState::GetSubObjectiveList()
{
	return this->SubObjectiveList;
}

void UObjectiveStrictSubObjectiveState::SetStrictSubObjectiveList(const TArray<FSubObjective>& SubObjectiveListIn)
{
	this->SubObjectiveList = SubObjectiveListIn;
}

EObjectiveState UObjectiveStrictSubObjectiveState::Evaluate()
{
	EObjectiveState CurrentState = EObjectiveState::Completed;
	
	for (const FSubObjective SubObjective : this->SubObjectiveList)
	{
		if (SubObjective.ObjectiveState == EObjectiveState::InProgress)
		{//If any 1 objective is InProgress then the state as a whole is InProgress
			CurrentState = EObjectiveState::InProgress;
		}
		else if (SubObjective.ObjectiveState == EObjectiveState::Failed)
		{//If any 1 objective is Failed then the state as a whole is Failed
			CurrentState = EObjectiveState::Failed;
			break;
		}
	}
	
	return CurrentState;
}