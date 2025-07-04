// Project Watcher 2024 & Beyond.

#include "Project_Watcher/Public/GameManagement/ObjectiveEventProcessor.h"

void UObjective::SetObjectiveData(const int32 WIDIn, const FString& TitleIn, const FString& DescriptionIn,
	const int64 StartTimeIn, const int64 EndTimeIn)
{
	this->WID = WIDIn;
	this->Title = TitleIn;
	this->Description = DescriptionIn;
	this->StartTime = StartTimeIn;
	this->EndTime = EndTimeIn;
}

void UObjective::SetSubObjectiveData(const TArray<FSubObjective>& SubObjectivesIn)
{
	this->SubObjectives = SubObjectivesIn;
}

void UObjective::SetObjectiveMarkerData(AActor* ObjectiveMarkerIn)
{
	this->ObjectiveMarker = ObjectiveMarkerIn;
}

void UObjective::SetSuccessObjectiveData(UObjective * SuccessObjectiveIn)
{
	this->SuccessObjective = SuccessObjectiveIn;
}

void UObjective::SetFailureObjectiveData(UObjective* FailureObjectiveIn)
{
	this->FailureObjective = FailureObjectiveIn;
}

void UObjectiveEventProcessor::AddObjective(UObjective * NewObjective)
{
	//Is this Objective Pending or Already Started?
	if (FDateTime::UtcNow().ToUnixTimestamp() > NewObjective->StartTime)
	{//The Objective Is Running
		this->AddObjectiveRunning(NewObjective);
	}
	else
	{//The Objective Is Pending
		this->AddObjectivePending(NewObjective);
	}
}

void UObjectiveEventProcessor::RemoveObjective(const int32 WID)
{
	for (int32 i = 0; i < RunningObjectives.Num(); i++)
	{
		if (RunningObjectives[i]->WID == WID)
		{
			RunningObjectives.RemoveAt(i);
			
			if (i == 0)
			{//We removed from the first position update the timer!
				this->UpdateRunningTimerUntilNextObjective();
			}
			return;
		}
	}

	for (int32 i = 0; i < PendingObjectives.Num(); i++)
	{
		if (PendingObjectives[i]->WID == WID)
		{
			PendingObjectives.RemoveAt(i);
			if (i == 0)
			{//We removed from the first position update the timer!
				this->UpdatePendingTimerUntilNextObjective();
			}
			return;
		}
	}
}

void UObjectiveEventProcessor::UpdateObjectiveCompletionState(const int32 WID, const bool Completed)
{
	for (int i = 0; i < RunningObjectives.Num(); i++)
	{
		if (RunningObjectives[i]->WID == WID)
		{
			UObjective * CachedObjective = RunningObjectives[i];
			CachedObjective->Completed = Completed;

			if (EvaluateObjective(CachedObjective))
			{//Objective is done remove it and let the system know it's done
				this->CallObjectiveCompleteDelegate(CachedObjective);
				RunningObjectives.RemoveAt(i);
				
				if (CachedObjective->SuccessObjective)
				{//If the success objective exists add it
					this->AddObjective(CachedObjective->SuccessObjective);
				}

				this->UpdateRunningTimerUntilNextObjective();
			}
			
			break;
		}
	}
}

void UObjectiveEventProcessor::UpdateObjectiveCompletionState(const int32 WID, const TArray<FSubObjective>& SubObjectives)
{
	for (int i = 0; i < RunningObjectives.Num(); i++)
	{
		if (RunningObjectives[i]->WID == WID && RunningObjectives[i]->SubObjectives.Num() > 0)
		{
			UObjective * CachedObjective = RunningObjectives[i];
			CachedObjective->SubObjectives = SubObjectives;

			if (EvaluateObjective(CachedObjective))
			{//Objective is done remove it and let the system know it's done
				this->CallObjectiveCompleteDelegate(CachedObjective);
				RunningObjectives.RemoveAt(i);

				if (CachedObjective->SuccessObjective)
				{//If the success objective exists add it
					this->AddObjective(CachedObjective->SuccessObjective);
				}
				
				this->UpdateRunningTimerUntilNextObjective();
			}
			
			break;
		}
	}
}

void UObjectiveEventProcessor::AddObjectiveRunning(UObjective* NewObjective)
{
	if (RunningObjectives.IsEmpty())
	{
		RunningObjectives.Add(NewObjective);
		this->UpdateRunningTimerUntilNextObjective();
	}
	else
	{
		for (int i = 0; i < RunningObjectives.Num(); i++)
		{
			if (RunningObjectives[i]->EndTime > NewObjective->EndTime)
			{
				RunningObjectives.Insert(NewObjective,i);

				if (i == 0)
				{//We inserted into the first position Update the Timer!
					this->UpdateRunningTimerUntilNextObjective();
				}
				break;
			}//if
		}//for
	}//if
}

void UObjectiveEventProcessor::AddObjectivePending(UObjective* NewObjective)
{
	if (PendingObjectives.IsEmpty())
	{
		PendingObjectives.Add(NewObjective);
		this->UpdatePendingTimerUntilNextObjective();
	}
	else
	{
		for (int i = 0; i < PendingObjectives.Num(); i++)
		{
			if (PendingObjectives[i]->StartTime > NewObjective->StartTime)
			{
				PendingObjectives.Insert(NewObjective,i);

				if (i == 0)
				{//We inserted into the first position Update the Timer!
					this->UpdatePendingTimerUntilNextObjective();
				}
				break;
			}//if
		}//for
	}//if
}

void UObjectiveEventProcessor::UpdateRunningTimerUntilNextObjective()
{
	if (!RunningObjectives.IsEmpty())
	{//Update the set timer with whatever is in the 0th position
		const float Rate = static_cast<float>(FMath::Min(RunningObjectives[0]->EndTime - FDateTime::UtcNow().ToUnixTimestamp(), 0));
		GetWorld()->GetTimerManager().SetTimer(this->RunningObjectiveTimerHandle, this, &UObjectiveEventProcessor::ObjectiveRunningTimerComplete, Rate, false);
	}
	else
	{//If nothing is present clear the timer
		GetWorld()->GetTimerManager().ClearTimer(this->RunningObjectiveTimerHandle);
	}
}

void UObjectiveEventProcessor::UpdatePendingTimerUntilNextObjective()
{
	if (PendingObjectives.Num() > 0)
	{//Update the set timer with whatever is in the 0th position
		const float Rate = static_cast<float>(FMath::Min(PendingObjectives[0]->StartTime - FDateTime::UtcNow().ToUnixTimestamp(),0));
		GetWorld()->GetTimerManager().SetTimer(this->PendingObjectiveTimerHandle, this, &UObjectiveEventProcessor::ObjectivePendingTimerComplete, Rate, false);
	}
	else
	{//If nothing is present clear the timer
		GetWorld()->GetTimerManager().ClearTimer(this->PendingObjectiveTimerHandle);
	}
}

void UObjectiveEventProcessor::ObjectiveRunningTimerComplete()
{
	if (!this->RunningObjectives.IsEmpty())
	{
		UObjective * Objective = this->RunningObjectives[0];
		this->RunningObjectives.RemoveAt(0);
		this->CallObjectiveTimerExpiredDelegate(Objective);

		//Add the Failure Objective IF it exists to the list of running objectives
		if (Objective->FailureObjective)
		{
			this->AddObjective(Objective->FailureObjective);
		}
		
		if (this->RunningObjectives.IsEmpty())
		{//Nothing Left to process
			this->GetWorld()->GetTimerManager().ClearTimer(this->RunningObjectiveTimerHandle);	
		}
	}	
}

void UObjectiveEventProcessor::ObjectivePendingTimerComplete()
{
	if (!this->PendingObjectives.IsEmpty())
	{
		UObjective * Objective = this->PendingObjectives[0];
		this->AddObjectiveRunning(Objective);
		this->PendingObjectives.RemoveAt(0);
		this->CallObjectiveTimerStartedDelegate(Objective);

		if (this->PendingObjectives.IsEmpty())
		{
			this->GetWorld()->GetTimerManager().ClearTimer(this->PendingObjectiveTimerHandle);
		}
	}
}

void UObjectiveEventProcessor::CallObjectiveCompleteDelegate(UObjective * Objective) const
{
	if (ObjectiveCompleteDelegate.IsBound())
	{
		ObjectiveCompleteDelegate.Broadcast(Objective);
	}
}

void UObjectiveEventProcessor::CallObjectiveTimerExpiredDelegate(UObjective * Objective) const
{
	if (ObjectiveExpiredDelegate.IsBound())
	{
		ObjectiveExpiredDelegate.Broadcast(Objective);
	}
}

void UObjectiveEventProcessor::CallObjectiveTimerStartedDelegate(UObjective * Objective) const
{
	if (ObjectiveStartedDelegate.IsBound())
	{
		ObjectiveStartedDelegate.Broadcast(Objective);
	}
}

bool UObjectiveEventProcessor::EvaluateObjective(UObjective * Objective)
{
	bool Completed = true;

	if (Objective->SubObjectives.Num() > 0)
	{
		for (const FSubObjective SubObjective : Objective->SubObjectives)
		{
			Completed &= SubObjective.Completed;
		}
	}
	else
	{
		Completed = Objective->Completed;
	}
	
	return Completed;
}
