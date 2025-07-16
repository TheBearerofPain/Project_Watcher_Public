// Project Watcher 2024 & Beyond.

#include "Project_Watcher/Public/GameManagement/Objective_System/Data_Manager/DataManager.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveLooseSubObjective/ObjectiveLooseSubObjectiveState.h"
#include "GameManagement/Objective_System/Support/DataManagerSave.h"
#include "Kismet/GameplayStatics.h"

void UDataManager::AddObjective(UObjective * NewObjective)
{
	NewObjective->Added = true;
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
		UObjective * CachedObjective = RunningObjectives[i];
		if (CachedObjective->WID == WID)
		{
			CachedObjective->Added = false;
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
		UObjective * CachedObjective = PendingObjectives[i];
		if (CachedObjective->WID == WID)
		{
			CachedObjective->Added = false;
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
		UObjective * CachedObjective = NoTimerObjectives[i];
		if (CachedObjective->WID == WID)
		{
			CachedObjective->Added = false;
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
						RunningObjectives[i]->Added = false;
						RunningObjectives.RemoveAt(i);

						if (Objective->FailureObjective)
						{
							this->AddObjective(Objective->FailureObjective);
						}

						this->UpdateRunningTimerUntilNextObjective();
						
						break;
					case EObjectiveState::Completed:

						this->CallObjectiveCompleteDelegate(Objective);
						RunningObjectives[i]->Added = false;
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

FDataManagerSaveState UDataManager::GetSaveState()
{
	FDataManagerSaveState SaveState;
	
	SaveState.WorldID = this->WorldID;
	SaveState.CurrentGameTime = this->CurrentGameTime;
	SaveState.Objectives = this->SerializeObjectiveSaveGraph();

	this->LogSparseObjectiveGraph();
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
	
	TMap<int32, UObjective*> ObjectiveMap;

	TArray<int32> ObjectiveSaveKeys;
	SaveState.Objectives.GetKeys(ObjectiveSaveKeys);

	//First Reconstruct the Objectives from their base save state
	for (const int32 ObjectiveSaveKey : ObjectiveSaveKeys)
	{
		UObjective * Objective = UObjective::Make(*SaveState.Objectives.Find(ObjectiveSaveKey));
		ObjectiveMap.Add(ObjectiveSaveKey,Objective);
		
		if (Objective->Added)
		{//Any Objectives that should be in the lists should be added to them now!
			this->AddObjective(Objective);
		}
	}

	//Reconnect the Graph based on Success & Failure WorldID's
	
	TArray<int32> ObjectiveKeys;
	ObjectiveMap.GetKeys(ObjectiveKeys);
	
	for (const int32 ObjectiveKey : ObjectiveKeys)
	{
		UObjective * CachedObjective = *ObjectiveMap.Find(ObjectiveKey);

		if (CachedObjective->SuccessObjectiveWID != -1)
		{
			if (ObjectiveMap.Contains(CachedObjective->SuccessObjectiveWID))
			{
				CachedObjective->SuccessObjective = *ObjectiveMap.Find(CachedObjective->SuccessObjectiveWID);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to find objective for key: %d"), ObjectiveKey);
			}
		}

		if (CachedObjective->FailureObjectiveWID != -1)
		{
			if (ObjectiveMap.Contains(CachedObjective->FailureObjectiveWID))
			{
				CachedObjective->FailureObjective = *ObjectiveMap.Find(CachedObjective->FailureObjectiveWID);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to find objective for key: %d"), ObjectiveKey);
			}
		}
	}

	this->LogSparseObjectiveGraph();
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

TMap<int32, UObjective*> UDataManager::SerializeObjectiveGraph()
{	
	//Gather Listed Objectives
	TArray<UObjective*> Objectives;
	Objectives.Append(this->PendingObjectives);
	Objectives.Append(this->RunningObjectives);
	Objectives.Append(this->NoTimerObjectives);

	this->SerializedObjectiveMap.Empty(Objectives.Num());

	for (UObjective * Objective : Objectives)
	{
		this->SerializeObjectiveSubGraph(Objective);
	}
	
	return this->SerializedObjectiveMap;
}

void UDataManager::SerializeObjectiveSubGraph(UObjective* Objective)
{
	//Add Root Objective if it isn't present in the TMap
	if (!this->SerializedObjectiveMap.Contains(Objective->WID))
	{
		this->SerializedObjectiveMap.Add(Objective->WID, Objective);
	}

	//Check Children, Only Recurse on them if they are not present in the SerializationMap
	if (Objective->SuccessObjective)
	{
		if (!this->SerializedObjectiveMap.Contains(Objective->SuccessObjective->WID))
		{
			this->SerializeObjectiveSubGraph(Objective->SuccessObjective);
		}
	}

	if (Objective->FailureObjective)
	{
		if (!this->SerializedObjectiveMap.Contains(Objective->FailureObjective->WID))
		{
			this->SerializeObjectiveSubGraph(Objective->FailureObjective);
		}
	}
}

TMap<int32, FObjectiveSave> UDataManager::SerializeObjectiveSaveGraph()
{
	TMap<int32, FObjectiveSave> ObjectiveSaves;
	
	const TMap<int32, UObjective*> Objectives = this->SerializeObjectiveGraph();

	TArray<int32> Keys;
	Objectives.GetKeys(Keys);

	for (int32 Key : Keys)
	{
		ObjectiveSaves.Add(Key, (*Objectives.Find(Key))->GetSaveState());
	}
	
	return ObjectiveSaves;	
}

void UDataManager::LogVerboseObjectiveGraph()
{
	const TMap<int32, UObjective*> Objectives = this->SerializeObjectiveGraph();

	TArray<int32> Keys;
	Objectives.GetKeys(Keys);

	for (const int32 Key : Keys)
	{
		(*Objectives.Find(Key))->LogVerbose();
	}
}

void UDataManager::LogSparseObjectiveGraph()
{
	const TMap<int32, UObjective*> Objectives = this->SerializeObjectiveGraph();

	TArray<int32> Keys;
	Objectives.GetKeys(Keys);

	for (const int32 Key : Keys)
	{
		(*Objectives.Find(Key))->LogSparse();
	}
}

void UDataManager::LogVerboseObjectiveLists()
{
	UE_LOG(LogTemp, Display, TEXT("[Start Pending Objective List]"));

	for (const UObjective * Objective : this->PendingObjectives)
	{
		Objective->LogVerbose();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End Pending Objective List]"));
	
	UE_LOG(LogTemp, Display, TEXT("[Start Running Objective List]"));

	for (const UObjective * Objective : this->RunningObjectives)
	{
		Objective->LogVerbose();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End Running Objective List]"));

	UE_LOG(LogTemp, Display, TEXT("[Start NoTimer Objective List]"));

	for (const UObjective * Objective : this->NoTimerObjectives)
	{
		Objective->LogVerbose();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End NoTimer Objective List]"));
}

void UDataManager::LogSparseObjectiveLists()
{
	UE_LOG(LogTemp, Display, TEXT("[Start Pending Objective List]"));

	for (const UObjective * Objective : this->PendingObjectives)
	{
		Objective->LogSparse();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End Pending Objective List]"));
	
	UE_LOG(LogTemp, Display, TEXT("[Start Running Objective List]"));

	for (const UObjective * Objective : this->RunningObjectives)
	{
		Objective->LogSparse();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End Running Objective List]"));

	UE_LOG(LogTemp, Display, TEXT("[Start NoTimer Objective List]"));

	for (const UObjective * Objective : this->NoTimerObjectives)
	{
		Objective->LogSparse();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End NoTimer Objective List]"));
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
