// Project Watcher 2024 & Beyond.

#include "GameManagement/Objective_System/Support/ObjectiveSupport.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveBasic/ObjectiveBasicState.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveLooseSubObjective/ObjectiveLooseSubObjectiveState.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveStrictSubObjective/ObjectiveStrictSubObjectiveState.h"

UObjective* UObjectiveSupport::CreateBasicObjective(const int32 WID, const FString& Title,
                                                    const FString& Description, const FObjectiveTime& ObjectiveTime)
{
	UObjectiveState * BasicState = NewObject<UObjectiveBasicState>();
	UObjective * Objective = NewObject<UObjective>();
	Objective->Setup(WID, Title, Description, BasicState, ObjectiveTime);
	return Objective;
}

UObjective* UObjectiveSupport::AddSuccessObjective(UObjective* Objective, UObjective* SuccessObjective)
{
	Objective->SetSuccessObjective(SuccessObjective);
	return Objective;
}

UObjective* UObjectiveSupport::AddFailureObjective(UObjective* Objective, UObjective* FailureObjective)
{
	Objective->SetFailureObjective(FailureObjective);
	return Objective;
}

UObjective* UObjectiveSupport::AddMarker(UObjective* Objective, AActor* Marker)
{
	Objective->SetObjectiveMarker(Marker);
	return Objective;
}

UObjective* UObjectiveSupport::AddSuccessAndFailureObjectives(UObjective* Objective, UObjective* SuccessObjective,
	UObjective* FailureObjective)
{
	Objective->SetSuccessObjective(SuccessObjective);
	Objective->SetFailureObjective(FailureObjective);
	return Objective;
}

UObjective* UObjectiveSupport::CreateLooseSubObjective(const int32 WID, const FString& Title,
                                                       const FString& Description, const TArray<FSubObjective>& SubObjectives, const int32 CompletionThreshold, const FObjectiveTime& ObjectiveTime)
{
	UObjectiveLooseSubObjectiveState * LooseSubObjectiveState = NewObject<UObjectiveLooseSubObjectiveState>();
	LooseSubObjectiveState->SetSubObjectiveState_Implementation(SubObjectives, CompletionThreshold);
	
	UObjective * Objective = NewObject<UObjective>();
	Objective->Setup(WID, Title, Description, LooseSubObjectiveState, ObjectiveTime);
	
	return Objective;
}

UObjective* UObjectiveSupport::CreateStrictSubObjective(const int32 WID, const FString& Title,
	const FString& Description, const TArray<FSubObjective>& SubObjectives, const FObjectiveTime& ObjectiveTime)
{
	UObjectiveStrictSubObjectiveState * StrictSubObjectiveState = NewObject<UObjectiveStrictSubObjectiveState>();
	StrictSubObjectiveState->SetSubObjectiveState_Implementation(SubObjectives);

	UObjective * Objective = NewObject<UObjective>();
	Objective->Setup(WID, Title, Description, StrictSubObjectiveState, ObjectiveTime);
	
	return Objective;
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
