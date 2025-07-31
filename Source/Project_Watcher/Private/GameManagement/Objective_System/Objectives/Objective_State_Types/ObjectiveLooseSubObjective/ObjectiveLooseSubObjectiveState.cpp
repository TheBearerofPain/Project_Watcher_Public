// Project Watcher 2024 & Beyond.

#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveLooseSubObjective/ObjectiveLooseSubObjectiveState.h"

UObjectiveState* UObjectiveLooseSubObjectiveState::GetCopy()
{
	UObjectiveLooseSubObjectiveState * Copy = NewObject<UObjectiveLooseSubObjectiveState>();

	if (!IsValid(Copy))
	{
		ensureMsgf(false, TEXT("UObjectiveLooseSubObjectiveState::GetCopy failed: object is Invalid"));
		return nullptr;
	}
	
	Copy->SubObjectiveList.Append(this->SubObjectiveList);
	Copy->CompletionThreshold = this->CompletionThreshold;
	return Copy;
}

void UObjectiveLooseSubObjectiveState::UpdateFromCopy(UObjectiveState* ObjectiveStateIn)
{
	if (!IsValid(ObjectiveStateIn))
	{
		ensureMsgf(false, TEXT("UObjectiveLooseSubObjectiveState::UpdateFromCopy failed: ObjectiveStateIn is Invalid"));
		return;
	}
	
	if (const UObjectiveLooseSubObjectiveState * Source = Cast<UObjectiveLooseSubObjectiveState>(ObjectiveStateIn))
	{
		this->SubObjectiveList.Empty();
		this->SubObjectiveList.Append(Source->SubObjectiveList);
		this->CompletionThreshold = Source->CompletionThreshold;
	}
	else
	{
		ensureMsgf(false, TEXT("UObjectiveLooseSubObjectiveState::UpdateFromCopy failed: Cast to UObjectiveLooseSubObjectiveState failed"));
	}
}

void UObjectiveLooseSubObjectiveState::SetPending()
{
	for (int32 i = 0; i < this->SubObjectiveList.Num(); i++)
	{
		this->SubObjectiveList[i].ObjectiveState = EObjectiveState::Pending;
	}
}

void UObjectiveLooseSubObjectiveState::SetInProgress()
{
	for (int32 i = 0; i < this->SubObjectiveList.Num(); i++)
	{
		this->SubObjectiveList[i].ObjectiveState = EObjectiveState::InProgress;
	}
}

TArray<FSubObjective> UObjectiveLooseSubObjectiveState::GetSubObjectiveState_Implementation()
{
	return this->SubObjectiveList;
}

void UObjectiveLooseSubObjectiveState::SetSubObjectiveState_Implementation(const TArray<FSubObjective>& SubObjectiveListIn, const int32 CompletionThresholdIn)
{
	this->SubObjectiveList = SubObjectiveListIn;
	this->CompletionThreshold = FMath::Max(1, FMath::Min(this->SubObjectiveList.Num(),CompletionThresholdIn));
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
