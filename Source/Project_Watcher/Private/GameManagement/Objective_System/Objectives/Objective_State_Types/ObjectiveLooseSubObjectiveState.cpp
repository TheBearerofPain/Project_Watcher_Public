// Project Watcher 2024 & Beyond.

#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveLooseSubObjectiveState.h"

TArray<FSubObjective> UObjectiveLooseSubObjectiveState::GetSubObjectiveList()
{
	return this->SubObjectiveList;
}

void UObjectiveLooseSubObjectiveState::SetLooseSubObjectiveList(const TArray<FSubObjective>& SubObjectiveListIn, const int32 CompletionThresholdIn)
{
	this->SubObjectiveList = SubObjectiveListIn;
	this->CompletionThreshold = CompletionThresholdIn;
}

EObjectiveState UObjectiveLooseSubObjectiveState::Evaluate()
{
	int32 TotalComplete = 0;
	int32 TotalInProgress = 0;
	EObjectiveState CurrentState;
	
	for (const FSubObjective SubObjective : this->SubObjectiveList)
	{
		switch (SubObjective.ObjectiveState)
		{
		case EObjectiveState::InProgress:
			TotalInProgress++;
			break;
		case EObjectiveState::Completed:
			TotalComplete++;
			break;
		default:
			break;
		}
	}

	if (TotalComplete >= this->CompletionThreshold)
	{//We are ObjectiveComplete
		CurrentState = EObjectiveState::Completed;
	}
	else if (TotalInProgress >= this->CompletionThreshold && TotalComplete < this->CompletionThreshold)
	{//We are still InProgress
		CurrentState = EObjectiveState::InProgress;
	}
	else
	{//If not Complete or InProgress we are in a failure state, We ignore pending states when evaluating
		CurrentState = EObjectiveState::Failed;
	}
	
	return CurrentState;
}