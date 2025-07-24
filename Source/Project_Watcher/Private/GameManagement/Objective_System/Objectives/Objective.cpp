// Project Watcher 2024 & Beyond.

#include "GameManagement/Objective_System/Objectives/Objective.h"

void FObjectiveData::LogSparse() const
{
	const FString LogString = "[Objective WorldID: " + FString::FromInt(this->WorldID) + "]";
	UE_LOG(LogTemp, Display, TEXT("%s"), *LogString);
}

void FObjectiveData::LogVerbose() const
{
	FString LogString = "[Start Objective WorldID: " + FString::FromInt(this->WorldID) + "]\n";
	LogString += "Title: " + this->Title + "\n";
	LogString += "Description: " + this->Description + "\n";
	LogString += "StartTime: " + FString::FromInt(this->ObjectiveTime.StartTime) + "\n";
	LogString += "EndTime: " + FString::FromInt(this->ObjectiveTime.EndTime) + "\n";
	LogString += this->ObjectiveTime.NoTimer ? "NoTimer: True\n" : "NoTimer: False\n";
	LogString += "ObjectiveState ObjectName: " + this->ObjectiveState->GetName() + " ObjectiveState ClassName: " + this->ObjectiveState->GetClass()->GetName() + "\n";
	LogString += "SuccessObjective WID: " + FString::FromInt(this->SuccessObjectiveWorldID) + "\n";
	LogString += "FailureObjective WID: " + FString::FromInt(this->FailureObjectiveWorldID) + "\n";
	LogString += this->Scheduled ? "Scheduled: True\n" : "Scheduled: False\n";
	LogString += "[End Objective WorldID: " + FString::FromInt(this->WorldID) + "]\n";
	UE_LOG(LogTemp, Display, TEXT("%s"), *LogString);
}

void FObjectiveData::SetPending()
{
	this->ObjectiveState->SetPending();
	this->Running = false;
	this->Scheduled = true;
}

void FObjectiveData::SetInProgress()
{
	this->ObjectiveState->SetInProgress();
	this->Running = true;
	this->Scheduled = true;
}

/*void FObjectiveData::SetScheduled()
{
	this->Scheduled = true;
}

void FObjectiveData::SetNotScheduled()
{
	this->Scheduled = false;
}*/

EObjectiveState FObjectiveData::Evaluate() const
{
	EObjectiveState CurrentState = EObjectiveState::Pending;
	
	if (ObjectiveState)
	{
		CurrentState = ObjectiveState->Evaluate();
	}
	
	return CurrentState;
}

void FObjectiveData::UpdateObjectiveState(UObjectiveState* ObjectiveStateIn) const
{
	this->ObjectiveState->UpdateFromCopy(ObjectiveStateIn);
}

FObjectiveData FObjectiveData::GetCopy() const
{
	FObjectiveData ObjectiveData = FObjectiveData();

	ObjectiveData.WorldID = this->WorldID;
	ObjectiveData.Tag = this->Tag;
	ObjectiveData.Title = this->Title;
	ObjectiveData.Description = this->Description;
	ObjectiveData.ObjectiveTime = this->ObjectiveTime;
	ObjectiveData.ObjectiveState = this->ObjectiveState->GetCopy();
	ObjectiveData.ObjectiveMarker = this->ObjectiveMarker;
	ObjectiveData.SuccessObjectiveWorldID = this->SuccessObjectiveWorldID;
	ObjectiveData.FailureObjectiveWorldID = this->FailureObjectiveWorldID;
	ObjectiveData.Scheduled = this->Scheduled;
	
	return ObjectiveData;
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
}

void UObjective::SetFailureObjective(UObjective* FailureObjectiveIn)
{
	this->FailureObjective = FailureObjectiveIn;
}

void UObjective::SetSuccessAndFailureObjectives(UObjective* SuccessObjectiveIn, UObjective* FailureObjectiveIn)
{
	this->SetSuccessObjective(SuccessObjectiveIn);
	this->SetFailureObjective(FailureObjectiveIn);
}

void UObjective::SetTag(const FString& TagIn)
{
	this->Tag = TagIn;
}

FObjectiveData UObjective::GetObjectiveData() const
{
	FObjectiveData ObjectiveData = FObjectiveData();

	ObjectiveData.WorldID = this->WorldID;
	ObjectiveData.Tag = this->Tag;
	ObjectiveData.Title = this->Title;
	ObjectiveData.Description = this->Description;
	ObjectiveData.ObjectiveTime = this->ObjectiveTime;
	ObjectiveData.ObjectiveState = this->ObjectiveState->GetCopy();
	
	if (this->SuccessObjective)
	{
		ObjectiveData.SuccessObjectiveWorldID = this->SuccessObjective->WorldID;
	}

	if (this->FailureObjective)
	{
		ObjectiveData.FailureObjectiveWorldID = this->FailureObjective->WorldID;
	}
	
	return ObjectiveData;
}
