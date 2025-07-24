// Project Watcher 2024 & Beyond.

#include "Project_Watcher/Public/GameManagement/Objective_System/Data_Manager/DataManager.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveLooseSubObjective/ObjectiveLooseSubObjectiveState.h"
#include "GameManagement/Objective_System/Support/DataManagerSave.h"
#include "GameManagement/Objective_System/Support/ObjectiveGraphSupport.h"
#include "Kismet/GameplayStatics.h"

void UDataManager::AddObjective(UObjective * NewObjective)
{
	if (NewObjective)
	{
		const int32 NewObjectiveWorldID = NewObjective->WorldID;
		
		if (NewObjective->ObjectiveTime.EvaluateAtScheduleTime)
		{//Use the CurrentTime to compute start and end times!
			NewObjective->ObjectiveTime.StartTime = this->GetCurrentGameSeconds() + NewObjective->ObjectiveTime.ScheduleStartTimeOffset;
			NewObjective->ObjectiveTime.EndTime = this->GetCurrentGameSeconds() + NewObjective->ObjectiveTime.ScheduleStartTimeOffset + NewObjective->ObjectiveTime.ScheduleDuration;
		}

		/* Serialize Graph & Create Deep Copy then append onto main graph */
		UObjectiveGraphSupport * ObjectiveGraphSupport = UObjectiveGraphSupport::Make();
		const TMap<int32, FObjectiveData> ObjectiveSubGraph = ObjectiveGraphSupport->GetObjectiveDataGraphFromObjective(NewObjective);
		this->Objectives.Append(ObjectiveSubGraph);
		
		if (this->CurrentGameTime > this->Objectives[NewObjectiveWorldID].ObjectiveTime.StartTime)
		{//The Objective Is Running
			if (this->Objectives[NewObjectiveWorldID].ObjectiveTime.NoTimer)
			{
				this->AddObjectiveNoTimer(NewObjectiveWorldID);
			}
			else
			{
				this->AddObjectiveRunning(NewObjectiveWorldID);
			}
		}
		else
		{//The Objective Is Pending
			this->AddObjectivePending(NewObjectiveWorldID);
		}
	}
}

UObjective * UDataManager::CreateObjective(const FString& Title, const FString& Description, UObjectiveState * ObjectiveState, const FObjectiveTime& ObjectiveTime)
{
	UObjective * NewObjective = NewObject<UObjective>();
	NewObjective->Setup(this->GetWorldID(), Title, Description, ObjectiveState, ObjectiveTime);
	return NewObjective;
}

UObjective* UDataManager::CreateObjectiveWithSuccessObjective(const FString& Title, const FString& Description,
	UObjectiveState* ObjectiveState, const FObjectiveTime& ObjectiveTime, UObjective* SuccessObjective)
{
	UObjective * NewObjective = NewObject<UObjective>();
	NewObjective->Setup(this->GetWorldID(), Title, Description, ObjectiveState, ObjectiveTime);
	NewObjective->SetSuccessObjective(SuccessObjective);
	return NewObjective;
}

UObjective* UDataManager::CreateObjectiveWithFailureObjective(const FString& Title, const FString& Description,
	UObjectiveState* ObjectiveState, const FObjectiveTime& ObjectiveTime, UObjective* FailureObjective)
{
	UObjective * NewObjective = NewObject<UObjective>();
	NewObjective->Setup(this->GetWorldID(), Title, Description, ObjectiveState, ObjectiveTime);
	NewObjective->SetFailureObjective(FailureObjective);
	return NewObjective;
}

UObjective* UDataManager::CreateObjectiveWithSuccessAndFailureObjectives(const FString& Title,
	const FString& Description, UObjectiveState* ObjectiveState, const FObjectiveTime& ObjectiveTime,
	UObjective* SuccessObjective, UObjective* FailureObjective)
{
	UObjective * NewObjective = NewObject<UObjective>();
	NewObjective->Setup(this->GetWorldID(), Title, Description, ObjectiveState, ObjectiveTime);
	NewObjective->SetSuccessAndFailureObjectives(SuccessObjective, FailureObjective);
	return NewObjective;
}

void UDataManager::RemoveObjective(const int32 ObjectiveWorldID)
{
	if (this->Objectives.Contains(ObjectiveWorldID))
	{
		this->Objectives.Remove(ObjectiveWorldID);
		this->NoTimerObjectives.Remove(ObjectiveWorldID);
		
		if (this->PendingObjectives.Remove(ObjectiveWorldID) > 0)
		{
			this->UpdatePendingTimerUntilNextObjective();
		}

		if (this->RunningObjectives.Remove(ObjectiveWorldID) > 0)
		{
			this->UpdateRunningTimerUntilNextObjective();
		}		
	}
}

void UDataManager::UpdateObjectiveState(const int32 ObjectiveWorldID, UObjectiveState * ObjectiveState)
{
	if (this->Objectives.Contains(ObjectiveWorldID) && ObjectiveState)
	{
		if (this->Objectives[ObjectiveWorldID].Running)
		{//We can only update running objectives states
			this->Objectives[ObjectiveWorldID].UpdateObjectiveState(ObjectiveState);
		
			switch (this->EvaluateObjective(ObjectiveWorldID))
			{
			case EObjectiveState::Failed:
					
				this->CallObjectiveFailedDelegate(ObjectiveWorldID);
				this->RemoveObjective(ObjectiveWorldID);
				if (const int32 FailureObjectiveWorldID = this->Objectives[ObjectiveWorldID].FailureObjectiveWorldID; FailureObjectiveWorldID != -1)
				{
					this->ScheduleObjective(FailureObjectiveWorldID);
				}

				this->UpdateRunningTimerUntilNextObjective();
						
				break;
			case EObjectiveState::Completed:

				this->CallObjectiveCompleteDelegate(ObjectiveWorldID);
				this->RemoveObjective(ObjectiveWorldID);
					
				if (const int32 SuccessObjectiveWorldID = this->Objectives[ObjectiveWorldID].SuccessObjectiveWorldID; SuccessObjectiveWorldID != -1)
				{
					this->ScheduleObjective(SuccessObjectiveWorldID);
				}
				
				this->UpdateRunningTimerUntilNextObjective();
				
				break;
			default:
				break;
			}//Switch
		}
	}
}

int64 UDataManager::GetCurrentGameSeconds() const
{
	return this->CurrentGameTime;
}

int64 UDataManager::RealWorldSecondsToGameSeconds(const int64 RealWorldSeconds) const
{
	return RealWorldSeconds * this->GameTimeMultiplier;
}

int64 UDataManager::RealWorldMinutesToGameSeconds(const int64 RealWorldMinutes) const
{
	return FMath::Max(0, RealWorldMinutes * 60 * this->GameTimeMultiplier);
}

int64 UDataManager::RealWorldHoursToGameSeconds(const int64 RealWorldHours) const
{
	return FMath::Max(0,RealWorldHours * 60 * 60 * this->GameTimeMultiplier);
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

FDateTime UDataManager::GameSecondsToGameDate(const int64 GameSeconds) const
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
	return this->GameSecondsToGameDate(ObjectiveTime.StartTime);
}

FDateTime UDataManager::GetObjectiveEndDate(const FObjectiveTime& ObjectiveTime) const
{
	return this->GameSecondsToGameDate(ObjectiveTime.EndTime);
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

FDataManagerSaveState UDataManager::GetSaveState()
{
	FDataManagerSaveState SaveState;
	
	SaveState.WorldID = this->WorldID;
	SaveState.CurrentGameTime = this->CurrentGameTime;
	SaveState.Objectives.Empty(this->Objectives.Num());
	SaveState.Objectives.Append(this->Objectives);

	this->LogVerboseObjectiveGraph();
	this->LogSparseObjectiveLists();
	
	return SaveState;
}

void UDataManager::RestoreSaveState(const FDataManagerSaveState& SaveState)
{
	this->WorldID = SaveState.WorldID;
	this->CurrentGameTime = SaveState.CurrentGameTime;

	//For Testing Purposes we empty these on SaveRestore
	this->PendingObjectives.Empty();
	this->RunningObjectives.Empty();
	this->NoTimerObjectives.Empty();
	this->Objectives.Empty(SaveState.Objectives.Num());
	this->Objectives.Append(SaveState.Objectives);

	TArray<int32> Keys;
	this->Objectives.GetKeys(Keys);

	for (const int32 Key : Keys)
	{
		if (this->Objectives[Key].Scheduled)
		{
			/* We don't want to re-evaluate start & end times so we just schedule them as they were */
			this->ScheduleRawObjective(Key);
		}
	}

	this->LogVerboseObjectiveGraph();
	this->LogSparseObjectiveLists();
}

void UDataManager::TestSave()
{
	UE_LOG(LogTemp, Display, TEXT("[Starting Save]"));
	if (UDataManagerSave * SaveGameInstance = Cast<UDataManagerSave>(UGameplayStatics::CreateSaveGameObject(UDataManagerSave::StaticClass())))
	{
		// Set data on the save game object.
		SaveGameInstance->SaveState = this->GetSaveState();
		
		// Save the data immediately.
		if (!UGameplayStatics::SaveGameToSlot(SaveGameInstance, UDataManagerSave::SaveSlot, UDataManagerSave::UserIndex))
		{
			UE_LOG(LogTemp, Warning, TEXT("Error Saving Data"));
		}
	}
	UE_LOG(LogTemp, Display, TEXT("[Done Saving]"));
}

void UDataManager::TestLoad()
{
	UE_LOG(LogTemp, Display, TEXT("[Starting Load]"));
	if (const UDataManagerSave * LoadedState = Cast<UDataManagerSave>(UGameplayStatics::LoadGameFromSlot(UDataManagerSave::SaveSlot, UDataManagerSave::UserIndex)))
	{
		this->RestoreSaveState(LoadedState->SaveState);
	}
	UE_LOG(LogTemp, Display, TEXT("[Done Loading]"));
}

void UDataManager::LogVerboseObjectiveGraph()
{
	TArray<int32> Keys;
	this->Objectives.GetKeys(Keys);

	for (const int32 Key : Keys)
	{
		Objectives.Find(Key)->LogVerbose();
	}
}

void UDataManager::LogSparseObjectiveGraph()
{
	TArray<int32> Keys;
	this->Objectives.GetKeys(Keys);

	for (const int32 Key : Keys)
	{
		Objectives.Find(Key)->LogSparse();
	}
}

void UDataManager::LogVerboseObjectiveLists()
{
	UE_LOG(LogTemp, Display, TEXT("[Start Pending Objective List]"));

	for (const int32 Key : this->PendingObjectives)
	{
		this->Objectives[Key].LogVerbose();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End Pending Objective List]"));
	
	UE_LOG(LogTemp, Display, TEXT("[Start Running Objective List]"));

	for (const int32 Key : this->RunningObjectives)
	{
		this->Objectives[Key].LogVerbose();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End Running Objective List]"));

	UE_LOG(LogTemp, Display, TEXT("[Start NoTimer Objective List]"));

	for (const int32 Key : this->NoTimerObjectives)
	{
		this->Objectives[Key].LogVerbose();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End NoTimer Objective List]"));
}

void UDataManager::LogSparseObjectiveLists()
{
	UE_LOG(LogTemp, Display, TEXT("[Start Pending Objective List]"));

	for (const int32 Key : this->PendingObjectives)
	{
		this->Objectives[Key].LogSparse();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End Pending Objective List]"));
	
	UE_LOG(LogTemp, Display, TEXT("[Start Running Objective List]"));

	for (const int32 Key : this->RunningObjectives)
	{
		this->Objectives[Key].LogSparse();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End Running Objective List]"));

	UE_LOG(LogTemp, Display, TEXT("[Start NoTimer Objective List]"));

	for (const int32 Key : this->NoTimerObjectives)
	{
		this->Objectives[Key].LogSparse();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End NoTimer Objective List]"));
}

void UDataManager::ScheduleObjective(const int32 ObjectiveWorldID)
{
	if (this->Objectives[ObjectiveWorldID].ObjectiveTime.EvaluateAtScheduleTime)
	{//Use the CurrentTime to compute start and end times!
		const FObjectiveTime CachedObjectiveTime = this->Objectives[ObjectiveWorldID].ObjectiveTime;
		this->Objectives[ObjectiveWorldID].ObjectiveTime.StartTime = this->GetCurrentGameSeconds() + CachedObjectiveTime.ScheduleStartTimeOffset;
		this->Objectives[ObjectiveWorldID].ObjectiveTime.EndTime = this->GetCurrentGameSeconds() + CachedObjectiveTime.ScheduleStartTimeOffset + CachedObjectiveTime.ScheduleDuration;
	}

	if (this->CurrentGameTime > this->Objectives[ObjectiveWorldID].ObjectiveTime.StartTime)
	{//The Objective Is Running
		if (this->Objectives[ObjectiveWorldID].ObjectiveTime.NoTimer)
		{
			this->AddObjectiveNoTimer(ObjectiveWorldID);
		}
		else
		{
			this->AddObjectiveRunning(ObjectiveWorldID);
		}
	}
	else
	{//The Objective Is Pending
		this->AddObjectivePending(ObjectiveWorldID);
	}
}

void UDataManager::ScheduleRawObjective(const int32 ObjectiveWorldID)
{
	if (this->Objectives.Contains(ObjectiveWorldID))
	{
		if (this->CurrentGameTime > this->Objectives[ObjectiveWorldID].ObjectiveTime.StartTime)
		{//The Objective Is Running
			if (this->Objectives[ObjectiveWorldID].ObjectiveTime.NoTimer)
			{
				this->AddObjectiveNoTimer(ObjectiveWorldID);
			}
			else
			{
				this->AddObjectiveRunning(ObjectiveWorldID);
			}
		}
		else
		{//The Objective Is Pending
			this->AddObjectivePending(ObjectiveWorldID);
		}
	}
}

void UDataManager::AddObjectiveRunning(const int32 ObjectiveWorldID)
{
	this->Objectives[ObjectiveWorldID].SetInProgress();
	
	if (RunningObjectives.IsEmpty())
	{
		RunningObjectives.Add(ObjectiveWorldID);
		this->UpdateRunningTimerUntilNextObjective();
	}
	else
	{
		for (int i = 0; i < RunningObjectives.Num(); i++)
		{
			if (this->Objectives[this->RunningObjectives[i]].ObjectiveTime.EndTime > this->Objectives[ObjectiveWorldID].ObjectiveTime.EndTime)
			{
				RunningObjectives.Insert(ObjectiveWorldID,i);

				if (i == 0)
				{//We inserted into the first position Update the Timer!
					this->UpdateRunningTimerUntilNextObjective();
				}
				break;
			}//if
		}//for
	}//if
}

void UDataManager::AddObjectivePending(const int32 ObjectiveWorldID)
{
	this->Objectives[ObjectiveWorldID].SetPending();
	
	if (PendingObjectives.IsEmpty())
	{
		PendingObjectives.Add(ObjectiveWorldID);
		this->UpdatePendingTimerUntilNextObjective();
	}
	else
	{
		for (int i = 0; i < PendingObjectives.Num(); i++)
		{
			if (this->Objectives[this->PendingObjectives[i]].ObjectiveTime.StartTime > this->Objectives[ObjectiveWorldID].ObjectiveTime.StartTime)
			{
				PendingObjectives.Insert(ObjectiveWorldID,i);

				if (i == 0)
				{//We inserted into the first position Update the Timer!
					this->UpdatePendingTimerUntilNextObjective();
				}
				break;
			}//if
		}//for
	}//if
}

void UDataManager::AddObjectiveNoTimer(const int32 ObjectiveWorldID)
{
	this->Objectives[ObjectiveWorldID].SetInProgress();
	NoTimerObjectives.Add(ObjectiveWorldID);
}

void UDataManager::UpdateRunningTimerUntilNextObjective()
{
	if (!RunningObjectives.IsEmpty())
	{//Update the set timer with whatever is in the 0th position
		const float Rate = static_cast<float>(FMath::Min(this->Objectives[this->RunningObjectives[0]].ObjectiveTime.EndTime - FDateTime::UtcNow().ToUnixTimestamp(), 0));
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
		//Need to evaluate times better!!!
		const float Rate = static_cast<float>(FMath::Min(this->Objectives[this->PendingObjectives[0]].ObjectiveTime.StartTime - FDateTime::UtcNow().ToUnixTimestamp(),0));
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
		if (const int32 ObjectiveWorldID = this->RunningObjectives[0]; this->Objectives.Contains(ObjectiveWorldID))
		{
			//Add the Failure Objective IF it exists to the list of running objectives
			if (const int32 FailureObjectiveWorldID = this->Objectives[ObjectiveWorldID].FailureObjectiveWorldID; FailureObjectiveWorldID != -1)
			{
				this->ScheduleObjective(FailureObjectiveWorldID);
			}
			
			this->CallObjectiveTimerExpiredDelegate(ObjectiveWorldID);
			this->RunningObjectives.RemoveAt(0);
			this->Objectives.Remove(ObjectiveWorldID);
			this->UpdateRunningTimerUntilNextObjective();
		}
	}	
}

void UDataManager::ObjectivePendingTimerComplete()
{
	if (!this->PendingObjectives.IsEmpty())
	{
		if (const int32 ObjectiveWorldID = this->PendingObjectives[0]; this->Objectives.Contains(ObjectiveWorldID))
		{
			if (this->Objectives[ObjectiveWorldID].ObjectiveTime.NoTimer)
			{
				this->AddObjectiveNoTimer(ObjectiveWorldID);
			}
			else
			{
				this->AddObjectiveRunning(ObjectiveWorldID);
			}
			
			this->CallObjectiveTimerStartedDelegate(ObjectiveWorldID);
			this->PendingObjectives.RemoveAt(0);
			this->UpdatePendingTimerUntilNextObjective();
		}
	}
}

void UDataManager::CallObjectiveCompleteDelegate(const int32 ObjectiveWorldID) const
{
	if (ObjectiveCompleteDelegate.IsBound() && this->Objectives.Contains(ObjectiveWorldID))
	{
		ObjectiveCompleteDelegate.Broadcast(this->Objectives[ObjectiveWorldID].GetCopy());
	}
}

void UDataManager::CallObjectiveFailedDelegate(const int32 ObjectiveWorldID) const
{
	if (ObjectiveFailedDelegate.IsBound() && this->Objectives.Contains(ObjectiveWorldID))
	{
		ObjectiveFailedDelegate.Broadcast(this->Objectives[ObjectiveWorldID].GetCopy());
	}
}

void UDataManager::CallObjectiveTimerExpiredDelegate(const int32 ObjectiveWorldID) const
{
	if (ObjectiveExpiredDelegate.IsBound() && this->Objectives.Contains(ObjectiveWorldID))
	{
		ObjectiveExpiredDelegate.Broadcast(this->Objectives[ObjectiveWorldID].GetCopy());
	}
}

void UDataManager::CallObjectiveTimerStartedDelegate(const int32 ObjectiveWorldID) const
{
	if (ObjectiveStartedDelegate.IsBound() && this->Objectives.Contains(ObjectiveWorldID))
	{
		ObjectiveStartedDelegate.Broadcast(this->Objectives[ObjectiveWorldID].GetCopy());
	}
}

EObjectiveState UDataManager::EvaluateObjective(const int32 ObjectiveWorldID)
{
	EObjectiveState ObjectiveState = EObjectiveState::Pending;
	
	if (this->Objectives.Contains(ObjectiveWorldID))
	{
		ObjectiveState = this->Objectives[ObjectiveWorldID].Evaluate();
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
		if (this->TimeAnchorMap.Contains(TimeAnchor))
		{
			SetTimeAnchor = *this->TimeAnchorMap.Find(TimeAnchor);
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
