// Project Watcher 2024 & Beyond.

#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveStrictSubObjective/ObjectiveStrictSubObjectiveState.h"

UObjectiveState * UObjectiveStrictSubObjectiveState::GetCopy()
{
	UObjectiveStrictSubObjectiveState * Copy = NewObject<UObjectiveStrictSubObjectiveState>();

	if (!IsValid(Copy))
	{
		ensureMsgf(false, TEXT("ObjectiveStrictSubObjectiveState::Copy failed: Object is Invalid"));
		return nullptr;
	}
	
	Copy->SubObjectiveList.Append(this->SubObjectiveList);
	return Copy;
}

void UObjectiveStrictSubObjectiveState::UpdateFromCopy(UObjectiveState* ObjectiveStateIn)
{
	if (!IsValid(ObjectiveStateIn))
	{
		ensureMsgf(false, TEXT("UObjectiveStrictSubObjectiveState::UpdateFromCopy failed: ObjectiveStateIn is Invalid"));
		return;
	}
	
	if (const UObjectiveStrictSubObjectiveState * Source = Cast<UObjectiveStrictSubObjectiveState>(ObjectiveStateIn))
	{
		this->SubObjectiveList.Empty();
		this->SubObjectiveList.Append(Source->SubObjectiveList);
	}
	else
	{
		ensureMsgf(false, TEXT("UObjectiveStrictSubObjectiveState::UpdateFromCopy failed: Cast to UObjectiveStrictSubObjectiveState failed"));
	}
}

void UObjectiveStrictSubObjectiveState::SetPending()
{
	for (int32 i = 0; i < this->SubObjectiveList.Num(); i++)
	{
		this->SubObjectiveList[i].ObjectiveState = EObjectiveState::Pending;
	}
}

void UObjectiveStrictSubObjectiveState::SetInProgress()
{
	for (int32 i = 0; i < this->SubObjectiveList.Num(); i++)
	{
		this->SubObjectiveList[i].ObjectiveState = EObjectiveState::InProgress;
	}
}

TArray<FSubObjective> UObjectiveStrictSubObjectiveState::GetSubObjectiveState_Implementation()
{
	return this->SubObjectiveList;
}

void UObjectiveStrictSubObjectiveState::SetSubObjectiveState_Implementation(const TArray<FSubObjective>& SubObjectiveListIn)
{
	this->SubObjectiveList = SubObjectiveListIn;
}

EObjectiveState UObjectiveStrictSubObjectiveState::Evaluate()
{
	EObjectiveState CurrentState = EObjectiveState::Completed;
	
	for (const FSubObjective SubObjective : this->SubObjectiveList)
	{
		if (SubObjective.ObjectiveState == EObjectiveState::InProgress)
		{
			/* If any 1 objective is InProgress then the state as a whole is InProgress */
			CurrentState = EObjectiveState::InProgress;
		}
		else if (SubObjective.ObjectiveState == EObjectiveState::Failed)
		{
			/*If any 1 objective is Failed then the state as a whole is Failed */
			CurrentState = EObjectiveState::Failed;
			break;
		}
	}
	
	return CurrentState;
}