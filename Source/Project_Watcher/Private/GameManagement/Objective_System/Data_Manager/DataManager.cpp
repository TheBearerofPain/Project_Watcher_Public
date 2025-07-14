// Project Watcher 2024 & Beyond.

#include "Project_Watcher/Public/GameManagement/Objective_System/Data_Manager/DataManager.h"

void UDataManager::AddObjective(UObjective * NewObjective)
{
	//Is this Objective Pending or Already Started?
	if (FDateTime::UtcNow().ToUnixTimestamp() > NewObjective->ObjectiveTime.StartTime)
	{//The Objective Is Running
		if (NewObjective->ObjectiveTime.NoTimer)
		{
			this->AddObjectiveNoTimer(NewObjective);
		}
		else
		{
			this->AddObjectiveRunning(NewObjective);
		}
	}
	else
	{//The Objective Is Pending
		this->AddObjectivePending(NewObjective);
	}
}

void UDataManager::RemoveObjective(const int32 WID)
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

	for (int32 i = 0; i < NoTimerObjectives.Num(); i++)
	{
		if (NoTimerObjectives[i]->WID == WID)
		{
			NoTimerObjectives.RemoveAt(i);
			return;
		}
	}
}

void UDataManager::UpdateObjectiveState(const int32 WID, UObjectiveState * ObjectiveStateIn)
{
	if (ObjectiveStateIn)
	{
		for (int32 i = 0; i < RunningObjectives.Num(); i++)
		{
			if (RunningObjectives[i]->WID == WID)
			{
				UObjective * Objective = RunningObjectives[i];
				Objective->ObjectiveState = ObjectiveStateIn;

				switch (this->EvaluateObjective(Objective))
				{
					case EObjectiveState::InProgress:
						break;
					case EObjectiveState::Failed:
						
						this->CallObjectiveFailedDelegate(Objective);
						RunningObjectives.RemoveAt(i);

						if (Objective->FailureObjective)
						{
							this->AddObjective(Objective->FailureObjective);
						}

						this->UpdateRunningTimerUntilNextObjective();
						
						break;
					case EObjectiveState::Completed:

						this->CallObjectiveCompleteDelegate(Objective);
						RunningObjectives.RemoveAt(i);
						
						if (Objective->SuccessObjective)
						{//If the success objective exists add it
							this->AddObjective(Objective->SuccessObjective);
						}
					
						this->UpdateRunningTimerUntilNextObjective();
					
						break;
					default:
						break;
				}
				
				break;
			}
		}
	}
}

int64 UDataManager::GetCurrentGameSeconds() const
{
	return this->CurrentGameTime;
}

int64 UDataManager::GameMinutesToGameSeconds(const int64 GameMinutes) const
{
	return FMath::Max(0, GameMinutes * 60);
}

int64 UDataManager::RealWorldMinutesToGameSeconds(const int64 RealWorldMinutes) const
{
	return FMath::Max(0, RealWorldMinutes * 60 * this->GameTimeMultiplier);
}

int64 UDataManager::GameHoursToGameSeconds(const int64 GameHours) const
{
	return FMath::Max(0, GameHours * 60 * 60);
}

int64 UDataManager::RealWorldHoursToGameSeconds(const int64 RealWorldHours) const
{
	return FMath::Max(0,RealWorldHours * 60 * 60 * this->GameTimeMultiplier);
}

int64 UDataManager::GameHoursAndMinutesToGameSeconds(const int64 GameHours, const int64 GameMinutes) const
{
	const int64 Seconds = (GameHours * 60 * 60) + (GameMinutes * 60);
	return FMath::Max(0, Seconds);
}

int64 UDataManager::RealWorldHoursAndMinutesToGameSeconds(const int64 RealWorldHours, const int64 RealWorldMinutes) const
{
	const int64 Seconds = (RealWorldHours * 60 * 60 * this->GameTimeMultiplier) + (RealWorldMinutes * 60 * this->GameTimeMultiplier);
	return FMath::Max(0, Seconds);
}

int64 UDataManager::GameDateToGameSeconds(const FDateTime& GameDate) const
{
	const int64 DateSeconds = FMath::Max(0, GameDate.ToUnixTimestamp() - this->GameAnchorDate.ToUnixTimestamp());
	return DateSeconds;
}

FDateTime UDataManager::GameSecondsToGameDateTime(const int64 GameSeconds) const
{
	const FDateTime Date = FDateTime( this->GameAnchorDate.ToUnixTimestamp() + GameSeconds );
	return Date;
}

FDateTime UDataManager::GetCurrentGameDate() const
{
	int64 CurrentTime = this->GameAnchorDate.ToUnixTimestamp();
	CurrentTime += this->CurrentGameTime;
	return FDateTime::FromUnixTimestamp(CurrentTime);
}

FDateTime UDataManager::GetObjectiveStartDate(const FObjectiveTime& ObjectiveTime) const
{
	return FDateTime::FromUnixTimestamp(ObjectiveTime.StartTime);
}

FDateTime UDataManager::GetObjectiveEndDate(const FObjectiveTime& ObjectiveTime) const
{
	return FDateTime::FromUnixTimestamp(ObjectiveTime.EndTime);
}

FObjectiveTime UDataManager::CreateDelayedObjectiveTime(const ETimeAnchor TimeAnchor, const int64 DelayInSeconds, const int64 DurationInSeconds)
{	
	const int64 ComputedTimeAnchor = this->ComputeTimeAnchor(TimeAnchor);
	const FObjectiveTime NewObjectiveTime(ComputedTimeAnchor + DelayInSeconds, ComputedTimeAnchor + DelayInSeconds + DurationInSeconds);
	return NewObjectiveTime;
}

FObjectiveTime UDataManager::CreateDelayedConstantObjectiveTime(const ETimeAnchor TimeAnchor, const int64 DelayInSeconds)
{
	const int64 ComputedTimeAnchor = this->ComputeTimeAnchor(TimeAnchor);
	const FObjectiveTime NewObjectiveTime(ComputedTimeAnchor + DelayInSeconds, -1);
	return NewObjectiveTime;
}

FObjectiveTime UDataManager::CreateObjectiveTimeFromDate(const FDateTime& StartDate, const FDateTime& EndDate) const
{
	const FObjectiveTime ObjectiveTime(this->GameDateToGameSeconds(StartDate), this->GameDateToGameSeconds(EndDate));	
	return ObjectiveTime;
}

FObjectiveTime UDataManager::CreateObjectiveTimeFromDateWithDuration(const FDateTime& StartDate, const int64 DurationInSeconds) const
{
	const int64 StartTime = this->GameDateToGameSeconds(StartDate);
	const FObjectiveTime ObjectiveTime(StartTime, StartTime + DurationInSeconds);
	return ObjectiveTime;
}

FObjectiveTime UDataManager::CreateConstantObjectiveTimeFromDate(const FDateTime& StartDate) const
{
	const FObjectiveTime ObjectiveTime(this->GameDateToGameSeconds(StartDate));
	return ObjectiveTime;
}

int32 UDataManager::GetWorldID()
{
	const int32 NewWorldID = this->WorldID;
	
	this->WorldID = (this->WorldID + 1) % TNumericLimits<int32>::Max();
	
	return NewWorldID;
}

void UDataManager::AddObjectiveRunning(UObjective* NewObjective)
{
	if (NewObjective)
	{//Set it so that it's in progress now!
		NewObjective->SetInProgress();
	}
	
	if (RunningObjectives.IsEmpty())
	{
		RunningObjectives.Add(NewObjective);
		this->UpdateRunningTimerUntilNextObjective();
	}
	else
	{
		for (int i = 0; i < RunningObjectives.Num(); i++)
		{
			if (RunningObjectives[i]->ObjectiveTime.EndTime > NewObjective->ObjectiveTime.EndTime)
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

void UDataManager::AddObjectivePending(UObjective* NewObjective)
{
	if (NewObjective)
	{
		NewObjective->SetPending();
	}
	
	if (PendingObjectives.IsEmpty())
	{
		PendingObjectives.Add(NewObjective);
		this->UpdatePendingTimerUntilNextObjective();
	}
	else
	{
		for (int i = 0; i < PendingObjectives.Num(); i++)
		{
			if (PendingObjectives[i]->ObjectiveTime.StartTime > NewObjective->ObjectiveTime.StartTime)
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

void UDataManager::AddObjectiveNoTimer(UObjective * NewObjective)
{
	NoTimerObjectives.Add(NewObjective);
}

void UDataManager::UpdateRunningTimerUntilNextObjective()
{
	if (!RunningObjectives.IsEmpty())
	{//Update the set timer with whatever is in the 0th position
		const float Rate = static_cast<float>(FMath::Min(RunningObjectives[0]->ObjectiveTime.EndTime - FDateTime::UtcNow().ToUnixTimestamp(), 0));
		GetWorld()->GetTimerManager().SetTimer(this->RunningObjectiveTimerHandle, this, &UDataManager::ObjectiveRunningTimerComplete, Rate, false);
	}
	else
	{//If nothing is present clear the timer
		GetWorld()->GetTimerManager().ClearTimer(this->RunningObjectiveTimerHandle);
	}
}

void UDataManager::UpdatePendingTimerUntilNextObjective()
{
	if (PendingObjectives.Num() > 0)
	{//Update the set timer with whatever is in the 0th position
		const float Rate = static_cast<float>(FMath::Min(PendingObjectives[0]->ObjectiveTime.StartTime - FDateTime::UtcNow().ToUnixTimestamp(),0));
		GetWorld()->GetTimerManager().SetTimer(this->PendingObjectiveTimerHandle, this, &UDataManager::ObjectivePendingTimerComplete, Rate, false);
	}
	else
	{//If nothing is present clear the timer
		GetWorld()->GetTimerManager().ClearTimer(this->PendingObjectiveTimerHandle);
	}
}

void UDataManager::ObjectiveRunningTimerComplete()
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

void UDataManager::ObjectivePendingTimerComplete()
{
	if (!this->PendingObjectives.IsEmpty())
	{
		UObjective * Objective = this->PendingObjectives[0];

		if (Objective->ObjectiveTime.NoTimer)
		{
			this->AddObjectiveNoTimer(Objective);
		}
		else
		{
			this->AddObjectiveRunning(Objective);
		}
		
		this->PendingObjectives.RemoveAt(0);
		this->CallObjectiveTimerStartedDelegate(Objective);

		if (this->PendingObjectives.IsEmpty())
		{
			this->GetWorld()->GetTimerManager().ClearTimer(this->PendingObjectiveTimerHandle);
		}
	}
}

void UDataManager::CallObjectiveCompleteDelegate(UObjective * Objective) const
{
	if (ObjectiveCompleteDelegate.IsBound())
	{
		ObjectiveCompleteDelegate.Broadcast(Objective);
	}
}

void UDataManager::CallObjectiveFailedDelegate(UObjective * Objective) const
{
	if (ObjectiveFailedDelegate.IsBound())
	{
		ObjectiveFailedDelegate.Broadcast(Objective);
	}
}

void UDataManager::CallObjectiveTimerExpiredDelegate(UObjective * Objective) const
{
	if (ObjectiveExpiredDelegate.IsBound())
	{
		ObjectiveExpiredDelegate.Broadcast(Objective);
	}
}

void UDataManager::CallObjectiveTimerStartedDelegate(UObjective * Objective) const
{
	if (ObjectiveStartedDelegate.IsBound())
	{
		ObjectiveStartedDelegate.Broadcast(Objective);
	}
}

EObjectiveState UDataManager::EvaluateObjective(const UObjective * Objective)
{
	EObjectiveState ObjectiveState = EObjectiveState::Pending;
	
	if (Objective)
	{
		ObjectiveState = Objective->Evaluate();
	}
	
	return ObjectiveState;
}

int64 UDataManager::ComputeTimeAnchor(const ETimeAnchor TimeAnchor)
{
	int64 SetTimeAnchor = 0;
	if (TimeAnchor == ETimeAnchor::GameCurrent)
	{
		SetTimeAnchor = this->CurrentGameTime;
	}
	else
	{
		if (this->TimeMap.Contains(TimeAnchor))
		{
			SetTimeAnchor = *this->TimeMap.Find(TimeAnchor);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("TimeAnchor not found: %s"), *UEnum::GetDisplayValueAsText(TimeAnchor).ToString());
		}
	}

	return SetTimeAnchor;
}

void UDataManager::StartGameTimer()
{
	this->GetWorld()->GetTimerManager().SetTimer(this->GameTimeTimerHandle,this, &UDataManager::UpdateCurrentGameTime,1, true);
}

void UDataManager::StopGameTimer()
{
	this->GetWorld()->GetTimerManager().ClearTimer(this->GameTimeTimerHandle);
}

void UDataManager::UpdateCurrentGameTime()
{
	this->CurrentGameTime = (this->CurrentGameTime + this->GameTimeMultiplier) % TNumericLimits<int64>::Max();
}
