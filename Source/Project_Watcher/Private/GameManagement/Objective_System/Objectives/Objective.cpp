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
	
	if (IsValid(this->ObjectiveState))
	{
		LogString += "ObjectiveState ObjectName: " + this->ObjectiveState->GetName() + " ObjectiveState ClassName: " + this->ObjectiveState->GetClass()->GetName() + "\n";
	}
	else
	{
		ensureMsgf(false, TEXT("UObjective::LogVerbose failed: ObjectiveState is Invalid"));
	}
	
	LogString += "SuccessObjective WID: " + FString::FromInt(this->SuccessObjectiveWorldID) + "\n";
	LogString += "FailureObjective WID: " + FString::FromInt(this->FailureObjectiveWorldID) + "\n";
	LogString += this->Scheduled ? "Scheduled: True\n" : "Scheduled: False\n";
	LogString += "[End Objective WorldID: " + FString::FromInt(this->WorldID) + "]\n";
	
	UE_LOG(LogTemp, Display, TEXT("%s"), *LogString);
}

void FObjectiveData::SetPending()
{
	if (IsValid(this->ObjectiveState))
	{
		this->ObjectiveState->SetPending();
	}
	else
	{
		ensureMsgf(false, TEXT("UObjective::SetPending failed: ObjectiveState is Invalid"));
	}
	
	this->Running = false;
	this->Scheduled = true;
}

void FObjectiveData::SetInProgress()
{
	if (IsValid(this->ObjectiveState))
	{
		this->ObjectiveState->SetInProgress();
	}
	else
	{
		ensureMsgf(false, TEXT("UObjective::SetInProgress failed: ObjectiveState is Invalid"));
	}
	
	this->Running = true;
	this->Scheduled = true;
}

EObjectiveState FObjectiveData::Evaluate() const
{
	EObjectiveState CurrentState = EObjectiveState::Pending;
	
	if (IsValid(this->ObjectiveState))
	{
		CurrentState = ObjectiveState->Evaluate();
	}
	else
	{
		ensureMsgf(false, TEXT("UObjective::Evaluate failed: ObjectiveState is Invalid"));
	}
	
	return CurrentState;
}

void FObjectiveData::UpdateObjectiveState(UObjectiveState* ObjectiveStateIn) const
{
	if (!IsValid(this->ObjectiveState))
	{
		ensureMsgf(false, TEXT("UObjective::UpdateObjectiveState failed: ObjectiveState is Invalid"));
		return;
	}

	if (!IsValid(ObjectiveStateIn))
	{
		ensureMsgf(false, TEXT("UObjective::UpdateObjectiveState failed: ObjectiveStateIn is Invalid"));
		return;
	}
	
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

	if (IsValid(this->ObjectiveState))
	{
		ObjectiveData.ObjectiveState = this->ObjectiveState->GetCopy();
	}
	else
	{
		ensureMsgf(false, TEXT("UObjective::GetCopy failed: ObjectiveState is Invalid"));
	}
	
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
	if (!IsValid(SuccessObjectiveIn))
	{
		ensureMsgf(false, TEXT("UObjective::SetSuccessAndFailureObjectives failed: SuccessObjectiveIn is Invalid"));
		return;	
	}

	if (!IsValid(FailureObjectiveIn))
	{
		ensureMsgf(false, TEXT("UObjective::SetSuccessAndFailureObjectives failed: FailureObjectiveIn is Invalid"));
		return;
	}
	
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

	if (IsValid(ObjectiveState))
	{
		ObjectiveData.ObjectiveState = this->ObjectiveState->GetCopy();
	}
	else
	{
		ensureMsgf(false, TEXT("UObjective::GetObjectiveData failed: ObjectiveState is Invalid"));
	}

	/* These 2 instances can be nullptr & we don't care if they are */
	
	if (IsValid(this->SuccessObjective))
	{
		ObjectiveData.SuccessObjectiveWorldID = this->SuccessObjective->WorldID;
	}

	if (IsValid(this->FailureObjective))
	{
		ObjectiveData.FailureObjectiveWorldID = this->FailureObjective->WorldID;
	}
	
	return ObjectiveData;
}
