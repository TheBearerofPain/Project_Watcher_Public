// Project Watcher 2024 & Beyond.

#include "GameManagement/Objective_System/Support/ObjectiveSupport.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveBasic/ObjectiveBasicState.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveLooseSubObjective/ObjectiveLooseSubObjectiveState.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveStrictSubObjective/ObjectiveStrictSubObjectiveState.h"

UObjective * UObjectiveSupport::SetSuccessObjective(UObjective* Objective, UObjective* SuccessObjective)
{
	Objective->SetSuccessObjective(SuccessObjective);
	return Objective;
}

UObjective * UObjectiveSupport::SetFailureObjective(UObjective* Objective, UObjective* FailureObjective)
{
	Objective->SetFailureObjective(FailureObjective);
	return Objective;
}

UObjective * UObjectiveSupport::SetMarker(UObjective* Objective, AActor* Marker)
{
	Objective->SetObjectiveMarker(Marker);
	return Objective;
}

UObjective * UObjectiveSupport::SetSuccessAndFailureObjectives(UObjective* Objective, UObjective* SuccessObjective, UObjective* FailureObjective)
{
	Objective->SetSuccessObjective(SuccessObjective);
	Objective->SetFailureObjective(FailureObjective);
	return Objective;
}

UObjectiveLooseSubObjectiveState * UObjectiveSupport::CreateLooseSubObjectiveState(const TArray<FSubObjective>& SubObjectives, const int32 CompletionThreshold)
{
	UObjectiveLooseSubObjectiveState * LooseSubObjectiveState = NewObject<UObjectiveLooseSubObjectiveState>();
	LooseSubObjectiveState->SetSubObjectiveState_Implementation(SubObjectives, CompletionThreshold);
	
	return LooseSubObjectiveState;
}

UObjectiveBasicState * UObjectiveSupport::CreateBasicState()
{
	UObjectiveBasicState * BasicState = NewObject<UObjectiveBasicState>();
	return BasicState;
}

UObjectiveStrictSubObjectiveState * UObjectiveSupport::CreateStrictSubObjectiveState(const TArray<FSubObjective>& SubObjectives)
{
	UObjectiveStrictSubObjectiveState * StrictSubObjectiveState = NewObject<UObjectiveStrictSubObjectiveState>();
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
