// Project Watcher 2024 & Beyond.

#include "GameManagement/Objective_System/Support/ObjectiveSupport.h"

UObjective * UObjectiveSupport::SetSuccessObjective(UObjective* Objective, UObjective* SuccessObjective)
{
	if (!IsValid(Objective))
	{
		ensureMsgf(false, TEXT("SetSuccessObjective failed: Objective is Invalid"));
		return nullptr;
	}
	
	if (!IsValid(SuccessObjective))
	{
		ensureMsgf(false, TEXT("SetSuccessObjective failed: SuccessObjective is Invalid"));
		return nullptr;
	}
	
	Objective->SetSuccessObjective(SuccessObjective);
	
	return Objective;
}

UObjective * UObjectiveSupport::SetFailureObjective(UObjective* Objective, UObjective* FailureObjective)
{
	if (!IsValid(Objective))
	{
		ensureMsgf(false, TEXT("SetFailureObjective failed: Objective is Invalid"));
		return nullptr;
	}

	if (!IsValid(FailureObjective))
	{
		ensureMsgf(false, TEXT("SetFailureObjective failed: FailureObjective is Invalid"));
		return nullptr;
	}
	
	Objective->SetFailureObjective(FailureObjective);

	return Objective;
}

UObjective * UObjectiveSupport::SetMarker(UObjective* Objective, AActor* Marker)
{
	if (!IsValid(Objective))
	{
		ensureMsgf(false, TEXT("SetMarker failed: Objective is Invalid"));
		return nullptr;
	}

	if (!IsValid(Marker))
	{
		ensureMsgf(false, TEXT("SetMarker failed: Marker is Invalid"));
		return nullptr;
	}
	
	Objective->SetObjectiveMarker(Marker);

	return Objective;
}

UObjective * UObjectiveSupport::SetSuccessAndFailureObjectives(UObjective* Objective, UObjective* SuccessObjective, UObjective* FailureObjective)
{
	if (!IsValid(Objective))
	{
		ensureMsgf(false, TEXT("SetSuccessAndFailureObjectives failed: Objective is Invalid"));
		return nullptr;
	}

	if (!IsValid(SuccessObjective))
	{
		ensureMsgf(false, TEXT("SetSuccessAndFailureObjectives failed: SuccessObjective is Invalid"));
		return nullptr;
	}

	if (!IsValid(FailureObjective))
	{
		ensureMsgf(false, TEXT("SetFailureObjectives failed: FailureObjective is Invalid"));
		return nullptr;
	}
	
	Objective->SetSuccessObjective(SuccessObjective);
	Objective->SetFailureObjective(FailureObjective);

	return Objective;
}

UObjectiveLooseSubObjectiveState * UObjectiveSupport::CreateLooseSubObjectiveState(const TArray<FSubObjective>& SubObjectives, const int32 CompletionThreshold)
{
	UObjectiveLooseSubObjectiveState * LooseSubObjectiveState = NewObject<UObjectiveLooseSubObjectiveState>();
	
	if (!IsValid(LooseSubObjectiveState))
	{
		ensureMsgf(false, TEXT("CreateLooseSubObjectiveState failed: Object is Invalid"));
		return nullptr;
	}
	
	LooseSubObjectiveState->SetSubObjectiveState_Implementation(SubObjectives, CompletionThreshold);
	return LooseSubObjectiveState;
}

UObjectiveBasicState * UObjectiveSupport::CreateBasicState()
{
	UObjectiveBasicState * BasicState = NewObject<UObjectiveBasicState>();

	if (!IsValid(BasicState))
	{
		ensureMsgf(false, TEXT("CreateBasicState failed: Object is Invalid"));
		return nullptr;
	}
	
	return BasicState;
}

UObjectiveStrictSubObjectiveState * UObjectiveSupport::CreateStrictSubObjectiveState(const TArray<FSubObjective>& SubObjectives)
{
	UObjectiveStrictSubObjectiveState * StrictSubObjectiveState = NewObject<UObjectiveStrictSubObjectiveState>();

	if (!IsValid(StrictSubObjectiveState))
	{
		ensureMsgf(false, TEXT("CreateStrictSubObjectiveState failed: Object is Invalid"));
		return nullptr;
	}
	
	StrictSubObjectiveState->SetSubObjectiveState_Implementation(SubObjectives);
	return StrictSubObjectiveState;
}

TArray<FSubObjective> UObjectiveSupport::CreateSubObjectiveListFromItem(const FSubObjective& SubObjective)
{
	TArray<FSubObjective> SubObjectives;
	SubObjectives.Add(SubObjective);
	return SubObjectives;
}

TArray<FSubObjective> UObjectiveSupport::CreateSubObjectiveList()
{
	TArray<FSubObjective> SubObjectives;
	return SubObjectives;
}

TArray<FSubObjective> UObjectiveSupport::AppendItemToSubObjectiveList(TArray<FSubObjective> SubObjectiveList,
	const FSubObjective& SubObjective)
{
	SubObjectiveList.Add(SubObjective);
	return SubObjectiveList;
}

TArray<FSubObjective> UObjectiveSupport::AppendListToSubObjectiveList(TArray<FSubObjective> SubObjectiveListDes,
	const TArray<FSubObjective>& SubObjectiveListSrc)
{
	SubObjectiveListDes.Append(SubObjectiveListSrc);
	return SubObjectiveListDes;
}
