// Project Watcher 2024 & Beyond.

#include "GameManagement/Objective_System/Objectives/Objective.h"

UObjective* UObjective::Make(const FObjectiveSave& Save)
{
	UObjective * Objective = NewObject<UObjective>();
	Objective->RestoreSaveState(Save);
	return Objective;
}

void UObjective::LogVerbose() const
{
	FString LogString = "[Start Objective: " + FString::FromInt(this->WID) + "]\n";
	LogString += "ObjectName: " + this->GetName() + " ClassName: " + this->GetClass()->GetName() + "\n";
	LogString += "Title: " + this->Title + "\n";
	LogString += "Description: " + this->Description + "\n";
	LogString += "StartTime: " + FString::FromInt(this->ObjectiveTime.StartTime) + "\n";
	LogString += "EndTime: " + FString::FromInt(this->ObjectiveTime.EndTime) + "\n";
	LogString += this->ObjectiveTime.NoTimer ? "NoTimer: True\n" : "NoTimer: False\n";
	LogString += "ObjectiveState ObjectName: " + this->ObjectiveState->GetName() + " ObjectiveState ClassName: " + this->ObjectiveState->GetClass()->GetName() + "\n";
	LogString += "SuccessObjective WID: " + FString::FromInt(this->SuccessObjectiveWID) + "\n";
	LogString += "FailureObjective WID: " + FString::FromInt(this->FailureObjectiveWID) + "\n";
	LogString += this->Added ? "Added: True\n" : "Added: False\n";
	LogString += "[End Objective: " + FString::FromInt(this->WID) + "]\n";

	UE_LOG(LogTemp, Display, TEXT("%s"), *LogString);
}

void UObjective::LogSparse() const
{
	const FString LogString = "[Objective: " + FString::FromInt(this->WID) + "]";
	UE_LOG(LogTemp, Display, TEXT("%s"), *LogString);
}

void UObjective::Setup(const int32 WIDIn, const FString& TitleIn, const FString& DescriptionIn,
                       UObjectiveState* ObjectiveStateIn, const FObjectiveTime& ObjectiveTimeIn)
{
	this->WID = WIDIn;
	this->Title = TitleIn;
	this->Description = DescriptionIn;
	this->ObjectiveState = ObjectiveStateIn;
	this->ObjectiveTime = ObjectiveTimeIn;
}

void UObjective::SetObjectiveMarker(AActor* ObjectiveMarkerIn)
{
	this->ObjectiveMarker = ObjectiveMarkerIn;
}

void UObjective::SetSuccessObjective(UObjective * SuccessObjectiveIn)
{
	this->SuccessObjective = SuccessObjectiveIn;
	this->SuccessObjectiveWID = SuccessObjectiveIn->WID;
}

void UObjective::SetFailureObjective(UObjective* FailureObjectiveIn)
{
	this->FailureObjective = FailureObjectiveIn;
	this->FailureObjectiveWID = FailureObjectiveIn->WID;
}

void UObjective::SetSuccessAndFailureObjectives(UObjective* SuccessObjectiveIn, UObjective* FailureObjectiveIn)
{
	this->SetSuccessObjective(SuccessObjectiveIn);
	this->SetFailureObjective(FailureObjectiveIn);
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

FObjectiveSave UObjective::GetSaveState()
{
	FObjectiveSave SaveState = FObjectiveSave();

	SaveState.WID = this->WID;
	SaveState.Title = this->Title;
	SaveState.Description = this->Description;
	SaveState.ObjectiveTime = this->ObjectiveTime;
	SaveState.ObjectiveState = this->ObjectiveState;
	SaveState.SuccessObjectiveWID = (this->SuccessObjective) ? this->SuccessObjective->WID : -1;
	SaveState.FailureObjectiveWID = (this->FailureObjective) ? this->FailureObjective->WID : -1;
	SaveState.Added	= this->Added;
	
	return SaveState;
}

void UObjective::RestoreSaveState(const FObjectiveSave& SaveState)
{
	this->WID = SaveState.WID;
	this->Title = SaveState.Title;
	this->Description = SaveState.Description;
	this->ObjectiveTime = SaveState.ObjectiveTime;
	this->ObjectiveState = SaveState.ObjectiveState;	
	this->SuccessObjectiveWID = SaveState.SuccessObjectiveWID;
	this->FailureObjectiveWID = SaveState.FailureObjectiveWID;
	this->Added = SaveState.Added;
}
