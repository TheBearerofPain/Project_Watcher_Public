// Project Watcher 2024 & Beyond.


#include "GameManagement/Objective_System/Objectives/Objective.h"

void UObjective::Setup(const int32 WIDIn, const FString& TitleIn, const FString& DescriptionIn,
	UObjectiveState* ObjectiveStateIn, const FObjectiveTime& ObjectiveTime, const bool NoTimerIn)
{
	this->WID = WIDIn;
	this->Title = TitleIn;
	this->Description = DescriptionIn;
	this->ObjectiveState = ObjectiveStateIn;
	this->StartTime = ObjectiveTime.StartTime;
	this->EndTime = ObjectiveTime.EndTime;
	this->NoTimer = NoTimerIn;
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

void UObjective::SetSuccessAndFailureObjectives(UObjective* SuccessObjectiveIn, UObjective* FailureObjectiveIn)
{
	this->SuccessObjective = SuccessObjectiveIn;
	this->FailureObjective = FailureObjectiveIn;
}

void UObjective::SetPending() const
{
	this->ObjectiveState->SetPending();
}

void UObjective::SetInProgress() const
{
	this->ObjectiveState->SetInProgress();
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
