// Project Watcher 2024 & Beyond.

#include "Project_Watcher/Public/GameManagement/Objective_System/Data_Manager/DataManager.h"
#include "GameManagement/Objective_System/Objectives/Objective_State_Types/ObjectiveLooseSubObjective/ObjectiveLooseSubObjectiveState.h"
#include "GameManagement/Objective_System/Support/DataManagerSave.h"
#include "GameManagement/Objective_System/Support/ObjectiveGraphSupport.h"
#include "Kismet/GameplayStatics.h"

UDataManager* UDataManager::Make(UWorld* World)
{
	if (!IsValid(World))
	{
		ensureMsgf(false, TEXT("UDataManager::Make failed: UWorld is Invalid"));
		return nullptr;
	}
	
	UDataManager * DataManager = NewObject<UDataManager>(World);

	if (!IsValid(DataManager))
	{
		ensureMsgf(false, TEXT("UDataManager::Make failed: object is Invalid"));
		return nullptr;
	}

	return DataManager;
}

void UDataManager::AddObjective(UObjective * NewObjective)
{
	if (!IsValid(NewObjective))
	{
		ensureMsgf(false, TEXT("AddObjective failed: NewObjective is Invalid"));
		return;
	}
	
	const int32 NewObjectiveWorldID = NewObjective->WorldID;
	const int64 CurrentTime = this->GetCurrentGameSeconds();
		
	if (NewObjective->ObjectiveTime.EvaluateAtScheduleTime)
	{
		NewObjective->ObjectiveTime.StartTime = CurrentTime + NewObjective->ObjectiveTime.ScheduleStartTimeOffset;
		NewObjective->ObjectiveTime.EndTime = CurrentTime + NewObjective->ObjectiveTime.ScheduleStartTimeOffset + NewObjective->ObjectiveTime.ScheduleDuration;
	}

	UObjectiveGraphSupport * ObjectiveGraphSupport = UObjectiveGraphSupport::Make();
	if (!IsValid(ObjectiveGraphSupport))
	{
		ensureMsgf(false, TEXT("AddObjective failed: ObjectiveGraphSupport is Invalid"));
		return;
	}
	
	/* Serialize Graph & Create Deep Copy then append onto main graph */
	const TMap<int32, FObjectiveData> ObjectiveSubGraph = ObjectiveGraphSupport->GetObjectiveDataGraphFromObjective(NewObjective);
	this->Objectives.Append(ObjectiveSubGraph);
	
	if (!this->Objectives.Contains(NewObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("AddObjective failed: Objective: %d, Wasn't added to objective map!"), NewObjectiveWorldID);
		return;
	}

	if (CurrentTime > this->Objectives[NewObjectiveWorldID].ObjectiveTime.StartTime)
	{
		if (this->Objectives[NewObjectiveWorldID].ObjectiveTime.NoTimer)
		{//The Objective Is Running, But is never going to expire
			this->AddObjectiveNoTimer(NewObjectiveWorldID);
		}
		else
		{//The Objective Is Running
			this->AddObjectiveRunning(NewObjectiveWorldID);
		}
	}
	else
	{//The Objective Is Pending
		this->AddObjectivePending(NewObjectiveWorldID);
	}
}

UObjective * UDataManager::CreateObjective(const FString& Title, const FString& Description, UObjectiveState * ObjectiveState, const FObjectiveTime& ObjectiveTime)
{
	UObjective * NewObjective = NewObject<UObjective>();

	if (!IsValid(NewObjective))
	{
		ensureMsgf(false, TEXT("CreateObjective failed: NewObjective is Invalid"));
		return nullptr;
	}
	
	NewObjective->Setup(this->GetWorldID(), Title, Description, ObjectiveState, ObjectiveTime);
	return NewObjective;
}

UObjective* UDataManager::CreateObjectiveWithSuccessObjective(const FString& Title, const FString& Description,
	UObjectiveState* ObjectiveState, const FObjectiveTime& ObjectiveTime, UObjective* SuccessObjective)
{
	UObjective * NewObjective = NewObject<UObjective>();

	if (!IsValid(NewObjective))
	{
		ensureMsgf(false, TEXT("CreateObjectiveWithSuccessObjective failed: NewObjective is Invalid"));
		return nullptr;
	}
	
	NewObjective->Setup(this->GetWorldID(), Title, Description, ObjectiveState, ObjectiveTime);
	NewObjective->SetSuccessObjective(SuccessObjective);
	return NewObjective;
}

UObjective* UDataManager::CreateObjectiveWithFailureObjective(const FString& Title, const FString& Description,
	UObjectiveState* ObjectiveState, const FObjectiveTime& ObjectiveTime, UObjective* FailureObjective)
{
	UObjective * NewObjective = NewObject<UObjective>();

	if (!IsValid(NewObjective))
	{
		ensureMsgf(false, TEXT("CreateObjectiveWithFailureObjective failed: NewObjective is Invalid"));
		return nullptr;
	}
	
	NewObjective->Setup(this->GetWorldID(), Title, Description, ObjectiveState, ObjectiveTime);
	NewObjective->SetFailureObjective(FailureObjective);
	return NewObjective;
}

UObjective* UDataManager::CreateObjectiveWithSuccessAndFailureObjectives(const FString& Title,
	const FString& Description, UObjectiveState* ObjectiveState, const FObjectiveTime& ObjectiveTime,
	UObjective* SuccessObjective, UObjective* FailureObjective)
{
	UObjective * NewObjective = NewObject<UObjective>();

	if (!IsValid(NewObjective))
	{
		ensureMsgf(false, TEXT("CreateObjectiveWithSuccessAndFailureObjectives failed: NewObjective is Invalid"));
		return nullptr;
	}
	
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
	if (!IsValid(ObjectiveState))
	{
		ensureMsgf(false, TEXT("UpdateObjectiveState failed: ObjectiveState is Invalid"));
		return;
	}

	if (!this->Objectives.Contains(ObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("UpdateObjectiveState failed: Objective list doesn't contain ObjectiveWorldID: %d"), ObjectiveWorldID);
		return;
	}
	
	if (this->Objectives[ObjectiveWorldID].Running)
	{
		//We can only update running objectives states
		this->Objectives[ObjectiveWorldID].UpdateObjectiveState(ObjectiveState);
	
		switch (this->EvaluateObjective(ObjectiveWorldID))
		{
		case EObjectiveState::Failed:
					
			this->CallObjectiveFailedDelegate(ObjectiveWorldID);
			
			if (const int32 FailureObjectiveWorldID = this->Objectives[ObjectiveWorldID].FailureObjectiveWorldID; FailureObjectiveWorldID != -1)
			{
				this->ScheduleObjective(FailureObjectiveWorldID);
			}

			this->RemoveObjective(ObjectiveWorldID);
			this->UpdateRunningTimerUntilNextObjective();
						
			break;
		case EObjectiveState::Completed:

			this->CallObjectiveCompleteDelegate(ObjectiveWorldID);
					
			if (const int32 SuccessObjectiveWorldID = this->Objectives[ObjectiveWorldID].SuccessObjectiveWorldID; SuccessObjectiveWorldID != -1)
			{
				this->ScheduleObjective(SuccessObjectiveWorldID);
			}

			this->RemoveObjective(ObjectiveWorldID);
			this->UpdateRunningTimerUntilNextObjective();
				
			break;
		default:
			break;
		}
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("Attempted to Update a NON RUNNING Objective"));
	}
}

FObjectiveData UDataManager::GetObjective(const int32 ObjectiveWorldID)
{	
	if (!this->Objectives.Contains(ObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("GetObjective failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
		return FObjectiveData();
	}

	return this->Objectives[ObjectiveWorldID].GetCopy();
}

int64 UDataManager::GetCurrentGameSeconds() const
{
	return this->CurrentGameTime;
}

int64 UDataManager::GetGameTimeMultiplier() const
{
	return this->GameTimeMultiplier;
}

int64 UDataManager::RealWorldSecondsToGameSeconds(const int64 RealWorldSeconds) const
{
	const int64 GameSeconds = RealWorldSeconds * this->GameTimeMultiplier;
	ensureMsgf(GameSeconds > -1, TEXT("RealWorldSecondsToGameSeconds failed: Computed GameSeconds was %lld "), GameSeconds);
	return FMath::Max(0, GameSeconds);
}

int64 UDataManager::RealWorldMinutesToGameSeconds(const int64 RealWorldMinutes) const
{
	const int64 GameSeconds = RealWorldMinutes * 60 * this->GameTimeMultiplier;
	ensureMsgf(GameSeconds > -1, TEXT("RealWorldMinutesToGameSeconds failed: Computed GameSeconds was %lld "), GameSeconds);
	return FMath::Max(0, GameSeconds);
}

int64 UDataManager::RealWorldHoursToGameSeconds(const int64 RealWorldHours) const
{
	const int64 GameSeconds = RealWorldHours * 60 * 60 * this->GameTimeMultiplier;
	ensureMsgf(GameSeconds > -1, TEXT("RealWorldHoursToGameSeconds failed: Computed GameSeconds was %lld "), GameSeconds);
	return FMath::Max(0,GameSeconds);
}

int64 UDataManager::RealWorldHoursAndMinutesToGameSeconds(const int64 RealWorldHours, const int64 RealWorldMinutes) const
{
	const int64 GameSeconds = (RealWorldHours * 60 * 60 * this->GameTimeMultiplier) + (RealWorldMinutes * 60 * this->GameTimeMultiplier);
	ensureMsgf(GameSeconds > -1, TEXT("RealWorldHoursAndMinutesToGameSeconds failed: Computed GameSeconds was %lld "), GameSeconds);
	return FMath::Max(0, GameSeconds);
}

int64 UDataManager::GameDateToGameSeconds(const FDateTime& GameDate) const
{
	const int64 GameSeconds = FMath::Max(0, GameDate.ToUnixTimestamp() - this->GameAnchorDate.ToUnixTimestamp());
	return GameSeconds;
}

FDateTime UDataManager::GameSecondsToGameDate(const int64 GameSeconds) const
{
	const FDateTime Date = FDateTime::FromUnixTimestamp(this->GameAnchorDate.ToUnixTimestamp() + GameSeconds);
	return Date;
}

FDateTime UDataManager::GetCurrentGameDate() const
{
	const int64 CurrentTime = this->GameAnchorDate.ToUnixTimestamp() + this->GetCurrentGameSeconds();
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
	if (!Objectives.Contains(ObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("ScheduleObjective failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
		return;
	}

	const int64 CurrentTime = this->GetCurrentGameSeconds();
	const FObjectiveTime CachedObjectiveTime = this->Objectives[ObjectiveWorldID].ObjectiveTime;
	
	if (CachedObjectiveTime.EvaluateAtScheduleTime)
	{
		this->Objectives[ObjectiveWorldID].ObjectiveTime.StartTime = CurrentTime + CachedObjectiveTime.ScheduleStartTimeOffset;
		this->Objectives[ObjectiveWorldID].ObjectiveTime.EndTime = CurrentTime + CachedObjectiveTime.ScheduleStartTimeOffset + CachedObjectiveTime.ScheduleDuration;
	}

	if (CurrentTime > this->Objectives[ObjectiveWorldID].ObjectiveTime.StartTime)
	{
		if (CachedObjectiveTime.NoTimer)
		{//The Objective Is Running, But will not expire
			this->AddObjectiveNoTimer(ObjectiveWorldID);
		}
		else
		{//The Objective Is Running
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
	if (!this->Objectives.Contains(ObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("ScheduleRawObjective failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
		return;
	}
	
	if (this->GetCurrentGameSeconds() > this->Objectives[ObjectiveWorldID].ObjectiveTime.StartTime)
	{
		if (this->Objectives[ObjectiveWorldID].ObjectiveTime.NoTimer)
		{//The Objective Is Running, But will not expire
			this->AddObjectiveNoTimer(ObjectiveWorldID);
		}
		else
		{//The Objective Is Running
			this->AddObjectiveRunning(ObjectiveWorldID);
		}
	}
	else
	{//The Objective Is Pending
		this->AddObjectivePending(ObjectiveWorldID);
	}
}

void UDataManager::AddObjectiveRunning(const int32 ObjectiveWorldID)
{
	if (!this->Objectives.Contains(ObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("AddObjectiveRunning failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
		return;
	}
	
	this->Objectives[ObjectiveWorldID].SetInProgress();
	
	if (RunningObjectives.IsEmpty())
	{
		RunningObjectives.Add(ObjectiveWorldID);
		this->UpdateRunningTimerUntilNextObjective();
	}
	else
	{
		const int64 CachedObjectiveEndTime = this->Objectives[ObjectiveWorldID].ObjectiveTime.EndTime;
		for (int32 i = 0; i < RunningObjectives.Num(); i++)
		{
			if (this->Objectives[this->RunningObjectives[i]].ObjectiveTime.EndTime > CachedObjectiveEndTime)
			{
				this->RunningObjectives.Insert(ObjectiveWorldID,i);

				if (i == 0)
				{//We inserted into the first position Update the Timer!
					this->UpdateRunningTimerUntilNextObjective();
				}
				return;
			}
		}

		/* Add at the end if we didn't insert prior */
		this->RunningObjectives.Add(ObjectiveWorldID);
	}
}

void UDataManager::AddObjectivePending(const int32 ObjectiveWorldID)
{
	if (!this->Objectives.Contains(ObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("AddObjectivePending failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
		return;
	}
	
	this->Objectives[ObjectiveWorldID].SetPending();
	
	if (PendingObjectives.IsEmpty())
	{
		PendingObjectives.Add(ObjectiveWorldID);
		this->UpdatePendingTimerUntilNextObjective();
	}
	else
	{
		const int64 CachedObjectiveStartTime = this->Objectives[ObjectiveWorldID].ObjectiveTime.StartTime;
		for (int32 i = 0; i < PendingObjectives.Num(); i++)
		{
			if (this->Objectives[this->PendingObjectives[i]].ObjectiveTime.StartTime > CachedObjectiveStartTime)
			{
				PendingObjectives.Insert(ObjectiveWorldID,i);

				if (i == 0)
				{//We inserted into the first position Update the Timer!
					this->UpdatePendingTimerUntilNextObjective();
				}
				return;
			}
		}

		/* Add at the end if we didn't insert prior */
		this->PendingObjectives.Add(ObjectiveWorldID);
	}
}

void UDataManager::AddObjectiveNoTimer(const int32 ObjectiveWorldID)
{
	if (!this->Objectives.Contains(ObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("AddObjectiveNoTimer failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
		return;
	}
	
	this->Objectives[ObjectiveWorldID].SetInProgress();
	NoTimerObjectives.Add(ObjectiveWorldID);
}

void UDataManager::UpdateRunningTimerUntilNextObjective()
{
	if (!RunningObjectives.IsEmpty())
	{//Update the set timer with whatever is in the 0th position
		const float Rate = static_cast<float>(FMath::Max(this->Objectives[this->RunningObjectives[0]].ObjectiveTime.EndTime - this->GetCurrentGameSeconds(), 0));

		if (!IsValid(GetWorld()))
		{
			ensureMsgf(false, TEXT("UWorld is Invalid"));
			return;
		}
		
		GetWorld()->GetTimerManager().SetTimer(this->RunningObjectiveTimerHandle, this, &UDataManager::ObjectiveRunningTimerComplete, Rate, false);
	}
	else
	{//If nothing is present clear the timer

		if (!IsValid(GetWorld()))
		{
			ensureMsgf(false, TEXT("UWorld is Invalid"));
			return;
		}
		
		GetWorld()->GetTimerManager().ClearTimer(this->RunningObjectiveTimerHandle);
	}
}

void UDataManager::UpdatePendingTimerUntilNextObjective()
{
	if (PendingObjectives.Num() > 0)
	{//Update the set timer with whatever is in the 0th position
		//Need to evaluate times better!!!
		const float Rate = static_cast<float>(FMath::Max(this->Objectives[this->PendingObjectives[0]].ObjectiveTime.StartTime - this->GetCurrentGameSeconds(),0));

		if (!IsValid(GetWorld()))
		{
			ensureMsgf(false, TEXT("UWorld is Invalid"));
			return;
		}
		
		GetWorld()->GetTimerManager().SetTimer(this->PendingObjectiveTimerHandle, this, &UDataManager::ObjectivePendingTimerComplete, Rate, false);
	}
	else
	{//If nothing is present clear the timer

		if (!IsValid(GetWorld()))
		{
			ensureMsgf(false, TEXT("UWorld is Invalid"));
			return;
		}
		
		GetWorld()->GetTimerManager().ClearTimer(this->PendingObjectiveTimerHandle);
	}
}

void UDataManager::ObjectiveRunningTimerComplete()
{
	if (this->RunningObjectives.IsEmpty())
	{
		ensureMsgf(false, TEXT("Running Objective list is empty, nothing to update!"));
		return;
	}
	
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
	else
	{
		ensureMsgf(false, TEXT("ObjectiveRunningTimerComplete failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
	}
}

void UDataManager::ObjectivePendingTimerComplete()
{
	if (this->PendingObjectives.IsEmpty())
	{	
		ensureMsgf(false, TEXT("Pending Objective list is empty, nothing to update!"));
		return;
	}
	
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
	else
	{
		ensureMsgf(false, TEXT("ObjectivePendingTimerComplete failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
	}
}

void UDataManager::CallObjectiveCompleteDelegate(const int32 ObjectiveWorldID) const
{
	if (!ObjectiveCompleteDelegate.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("Nothing Bound to ObjectiveCompleteDelegate"));
		return;
	}

	if (!this->Objectives.Contains(ObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("CallObjectiveCompleteDelegate failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
		return;
	}
	
	ObjectiveCompleteDelegate.Broadcast(this->Objectives[ObjectiveWorldID].GetCopy());
}

void UDataManager::CallObjectiveFailedDelegate(const int32 ObjectiveWorldID) const
{
	if (!ObjectiveFailedDelegate.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("Nothing Bound to ObjectiveFailedDelegate"));
		return;
	}

	if (!this->Objectives.Contains(ObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("CallObjectiveFailedDelegate failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
		return;
	}
	
	ObjectiveFailedDelegate.Broadcast(this->Objectives[ObjectiveWorldID].GetCopy());
}

void UDataManager::CallObjectiveTimerExpiredDelegate(const int32 ObjectiveWorldID) const
{
	if (!ObjectiveExpiredDelegate.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("Nothing Bound to ObjectiveExpiredDelegate"));
		return;
	}

	if (!this->Objectives.Contains(ObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("CallObjectiveTimerExpiredDelegate failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
		return;
	}
	
	ObjectiveExpiredDelegate.Broadcast(this->Objectives[ObjectiveWorldID].GetCopy());
}

void UDataManager::CallObjectiveTimerStartedDelegate(const int32 ObjectiveWorldID) const
{
	if (!ObjectiveStartedDelegate.IsBound())
	{
		UE_LOG(LogTemp, Warning, TEXT("Nothing Bound to ObjectiveStartedDelegate"));
		return;
	}

	if (!this->Objectives.Contains(ObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("CallObjectiveTimerStartedDelegate failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
		return;
	}
	
	ObjectiveStartedDelegate.Broadcast(this->Objectives[ObjectiveWorldID].GetCopy());
}

EObjectiveState UDataManager::EvaluateObjective(const int32 ObjectiveWorldID)
{
	if (!this->Objectives.Contains(ObjectiveWorldID))
	{
		ensureMsgf(false, TEXT("EvaluateObjective failed: ObjectiveWorldID %d not found"), ObjectiveWorldID);
		return EObjectiveState::Pending;
	}

	return this->Objectives[ObjectiveWorldID].Evaluate();
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
	if (!IsValid(GetWorld()))
	{
		ensureMsgf(false, TEXT("UWorld is Invalid"));
		return;
	}
	
	this->GetWorld()->GetTimerManager().SetTimer(this->GameTimeTimerHandle,this, &UDataManager::UpdateCurrentGameTime,1, true);
}

void UDataManager::StopGameTimer()
{
	if (!IsValid(GetWorld()))
	{
		ensureMsgf(false, TEXT("UWorld is Invalid"));
		return;
	}
	
	this->GetWorld()->GetTimerManager().ClearTimer(this->GameTimeTimerHandle);
}

void UDataManager::UpdateCurrentGameTime()
{
	this->CurrentGameTime = (this->CurrentGameTime + this->GameTimeMultiplier) % TNumericLimits<int64>::Max();
}
