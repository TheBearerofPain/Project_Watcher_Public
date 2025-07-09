// Project Watcher 2024 & Beyond.


#include "GameManagement/Objective_System/Objectives/Objective.h"

void UObjective::SetObjectiveData(const int32 WIDIn, const FString& TitleIn, const FString& DescriptionIn,
								 UObjectiveState * ObjectiveDataIn, const int64 StartTimeIn, const int64 EndTimeIn)
{
	this->WID = WIDIn;
	this->Title = TitleIn;
	this->Description = DescriptionIn;
	this->ObjectiveState = ObjectiveDataIn;
	this->StartTime = StartTimeIn;
	this->EndTime = EndTimeIn;
}

void UObjective::SetObjectiveDataNoTimer(const int32 WIDIn, const FString& TitleIn, const FString& DescriptionIn,
	UObjectiveState * ObjectiveDataIn, const int64 StartTimeIn)
{
	this->WID = WIDIn;
	this->Title = TitleIn;
	this->Description = DescriptionIn;
	this->ObjectiveState = ObjectiveDataIn;
	this->StartTime = StartTimeIn;
	this->NoTimer = true;
}

void UObjective::SetObjectiveMarker(AActor* ObjectiveMarkerIn)
{
	this->ObjectiveMarker = ObjectiveMarkerIn;
}

void UObjective::SetSuccessObjective(UObjective * SuccessObjectiveIn)
{
	this->SuccessObjective = SuccessObjectiveIn;
}

void UObjective::SetFailureObjective(UObjective* FailureObjectiveIn)
{
	this->FailureObjective = FailureObjectiveIn;
}

EObjectiveState UObjective::Evaluate() const
{
	EObjectiveState CurrentState = EObjectiveState::Pending;
	
	if (ObjectiveState)
	{
		CurrentState = ObjectiveState->Evaluate();
	}
	
	return CurrentState;
}