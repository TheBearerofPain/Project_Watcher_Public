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
	FString LogString = "[Start Objective: " + FString::FromInt(this->WorldID) + "]\n";
	LogString += "ObjectName: " + this->GetName() + " ClassName: " + this->GetClass()->GetName() + "\n";
	LogString += "Title: " + this->Title + "\n";
	LogString += "Description: " + this->Description + "\n";
	LogString += "StartTime: " + FString::FromInt(this->ObjectiveTime.StartTime) + "\n";
	LogString += "EndTime: " + FString::FromInt(this->ObjectiveTime.EndTime) + "\n";
	LogString += this->ObjectiveTime.NoTimer ? "NoTimer: True\n" : "NoTimer: False\n";
	LogString += "ObjectiveState ObjectName: " + this->ObjectiveState->GetName() + " ObjectiveState ClassName: " + this->ObjectiveState->GetClass()->GetName() + "\n";
	LogString += "SuccessObjective WID: " + FString::FromInt(this->SuccessObjectiveWorldID) + "\n";
	LogString += "FailureObjective WID: " + FString::FromInt(this->FailureObjectiveWorldID) + "\n";
	LogString += this->Added ? "Added: True\n" : "Added: False\n";
	LogString += "[End Objective: " + FString::FromInt(this->WorldID) + "]\n";

	UE_LOG(LogTemp, Display, TEXT("%s"), *LogString);
}

void UObjective::LogSparse() const
{
	const FString LogString = "[Objective: " + FString::FromInt(this->WorldID) + "]";
	UE_LOG(LogTemp, Display, TEXT("%s"), *LogString);
}

void UObjective::Setup(const int32 WorldIDIn, const FString& TitleIn, const FString& DescriptionIn,
                       UObjectiveState* ObjectiveStateIn, const FObjectiveTime& ObjectiveTimeIn)
{
	this->WorldID = WorldIDIn;
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
	this->SuccessObjectiveWorldID = SuccessObjectiveIn->WorldID;
}

void UObjective::SetFailureObjective(UObjective* FailureObjectiveIn)
{
	this->FailureObjective = FailureObjectiveIn;
	this->FailureObjectiveWorldID = FailureObjectiveIn->WorldID;
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

	SaveState.WorldID = this->WorldID;
	SaveState.Title = this->Title;
	SaveState.Description = this->Description;
	SaveState.ObjectiveTime = this->ObjectiveTime;
	SaveState.ObjectiveState = this->ObjectiveState;
	SaveState.SuccessObjectiveWorldID = (this->SuccessObjective) ? this->SuccessObjective->WorldID : -1;
	SaveState.FailureObjectiveWorldID = (this->FailureObjective) ? this->FailureObjective->WorldID : -1;
	SaveState.Added	= this->Added;
	
	return SaveState;
}

void UObjective::RestoreSaveState(const FObjectiveSave& SaveState)
{
	this->WorldID = SaveState.WorldID;
	this->Title = SaveState.Title;
	this->Description = SaveState.Description;
	this->ObjectiveTime = SaveState.ObjectiveTime;
	this->ObjectiveState = SaveState.ObjectiveState;	
	this->SuccessObjectiveWorldID = SaveState.SuccessObjectiveWorldID;
	this->FailureObjectiveWorldID = SaveState.FailureObjectiveWorldID;
	this->Added = SaveState.Added;
}

FObjectiveData UObjective::GetObjectiveData() const
{
	FObjectiveData ObjectiveData = FObjectiveData();

	ObjectiveData.WorldID = this->WorldID;
	ObjectiveData.Title = this->Title;
	ObjectiveData.Description = this->Description;
	ObjectiveData.ObjectiveTime = this->ObjectiveTime;
	ObjectiveData.ObjectiveState = this->ObjectiveState->GetCopy();
	
	return ObjectiveData;
}

void UObjective::UpdateObjectiveState(UObjectiveState * ObjectiveStateIn) const
{
	this->ObjectiveState->UpdateFromCopy(ObjectiveStateIn);
}
