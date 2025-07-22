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
		NewObjective->Scheduled = true;

		if (NewObjective->ObjectiveTime.EvaluateAtScheduleTime)
		{//Ue the CurrentTime to compute start and end times!
			NewObjective->ObjectiveTime.StartTime = this->GetCurrentGameSeconds() + NewObjective->ObjectiveTime.ScheduleStartTimeOffset;
			NewObjective->ObjectiveTime.EndTime = this->GetCurrentGameSeconds() + NewObjective->ObjectiveTime.ScheduleStartTimeOffset + NewObjective->ObjectiveTime.ScheduleDuration;
		}

		UObjectiveGraphSupport * ObjectiveGraphSupport = UObjectiveGraphSupport::Make();
		const TMap<int32, UObjective*> ObjectiveSubGraph = ObjectiveGraphSupport->SerializeObjective(NewObjective);
		this->Objectives.Append(ObjectiveSubGraph);//Add SubGraph to MainGraph
		
		if (this->CurrentGameTime > NewObjective->ObjectiveTime.StartTime)
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
}

UObjective * UDataManager::CreateObjective(const FString& TitleIn, const FString& DescriptionIn, UObjectiveState * ObjectiveStateIn, const FObjectiveTime& ObjectiveTimeIn)
{
	UObjective * NewObjective = NewObject<UObjective>();
	NewObjective->Setup(this->GetWorldID(), TitleIn, DescriptionIn, ObjectiveStateIn, ObjectiveTimeIn);
	return NewObjective;
}

UObjective* UDataManager::CreateObjectiveWithSuccessObjective(const FString& TitleIn, const FString& DescriptionIn,
	UObjectiveState* ObjectiveStateIn, const FObjectiveTime& ObjectiveTimeIn, UObjective* SuccessObjectiveIn)
{
	UObjective * NewObjective = NewObject<UObjective>();
	NewObjective->Setup(this->GetWorldID(), TitleIn, DescriptionIn, ObjectiveStateIn, ObjectiveTimeIn);
	NewObjective->SetSuccessObjective(SuccessObjectiveIn);
	return NewObjective;
}

UObjective* UDataManager::CreateObjectiveWithFailureObjective(const FString& TitleIn, const FString& DescriptionIn,
	UObjectiveState* ObjectiveStateIn, const FObjectiveTime& ObjectiveTimeIn, UObjective* FailureObjectiveIn)
{
	UObjective * NewObjective = NewObject<UObjective>();
	NewObjective->Setup(this->GetWorldID(), TitleIn, DescriptionIn, ObjectiveStateIn, ObjectiveTimeIn);
	NewObjective->SetFailureObjective(FailureObjectiveIn);
	return NewObjective;
}

UObjective* UDataManager::CreateObjectiveWithSuccessAndFailureObjectives(const FString& TitleIn,
	const FString& DescriptionIn, UObjectiveState* ObjectiveStateIn, const FObjectiveTime& ObjectiveTimeIn,
	UObjective* SuccessObjectiveIn, UObjective* FailureObjectiveIn)
{
	UObjective * NewObjective = NewObject<UObjective>();
	NewObjective->Setup(this->GetWorldID(), TitleIn, DescriptionIn, ObjectiveStateIn, ObjectiveTimeIn);
	NewObjective->SetSuccessAndFailureObjectives(SuccessObjectiveIn, FailureObjectiveIn);
	return NewObjective;
}

void UDataManager::RemoveObjective(const int32 WorldIDIn)
{
	if (this->Objectives.Contains(WorldIDIn))
	{
		this->Objectives[WorldIDIn]->Scheduled = false;
		this->Objectives.Remove(WorldIDIn);
		this->NoTimerObjectives.Remove(WorldIDIn);		
		
		if (this->PendingObjectives.Remove(WorldIDIn) > 0)
		{
			this->UpdatePendingTimerUntilNextObjective();
		}

		if (this->RunningObjectives.Remove(WorldIDIn) > 0)
		{
			this->UpdateRunningTimerUntilNextObjective();
		}		
	}
}

void UDataManager::UpdateObjectiveState(const int32 WorldIDIn, UObjectiveState * ObjectiveStateIn)
{
	if (ObjectiveStateIn)
	{
		TArray<int32> Keys;
		this->Objectives.GetKeys(Keys);

		for (const int32 Key : Keys)
		{
			if (Key == WorldIDIn)
			{
				switch (const UObjective * Objective = this->Objectives[Key]; this->EvaluateObjective(Objective))
				{
				case EObjectiveState::Failed:
						
					this->CallObjectiveFailedDelegate(Objective);
					this->RemoveObjective(Objective->WorldID);

					if (Objective->FailureObjective)
					{
						this->AddObjective(Objective->FailureObjective);
					}

					this->UpdateRunningTimerUntilNextObjective();
						
					break;
				case EObjectiveState::Completed:

					this->CallObjectiveCompleteDelegate(Objective);
					this->RemoveObjective(Objective->WorldID);
						
					if (Objective->SuccessObjective)
					{
						this->AddObjective(Objective->SuccessObjective);
					}
					
					this->UpdateRunningTimerUntilNextObjective();
					
					break;
				default:
					break;
				}//Switch
				break;
			}
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
	SaveState.Objectives = UObjectiveGraphSupport::GetObjectiveSaveGraphFromObjectiveGraph(this->Objectives);

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
	
	TMap<int32, UObjective*> ObjectiveMap;

	TArray<int32> ObjectiveSaveKeys;
	SaveState.Objectives.GetKeys(ObjectiveSaveKeys);

	//First Reconstruct the Objectives from their base save state
	for (const int32 ObjectiveSaveKey : ObjectiveSaveKeys)
	{
		UObjective * Objective = UObjective::Make(*SaveState.Objectives.Find(ObjectiveSaveKey));
		ObjectiveMap.Add(ObjectiveSaveKey,Objective);
		
		if (Objective->Scheduled)
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

		if (CachedObjective->SuccessObjectiveWorldID != -1)
		{
			if (ObjectiveMap.Contains(CachedObjective->SuccessObjectiveWorldID))
			{
				CachedObjective->SuccessObjective = *ObjectiveMap.Find(CachedObjective->SuccessObjectiveWorldID);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to find objective for key: %d"), ObjectiveKey);
			}
		}

		if (CachedObjective->FailureObjectiveWorldID != -1)
		{
			if (ObjectiveMap.Contains(CachedObjective->FailureObjectiveWorldID))
			{
				CachedObjective->FailureObjective = *ObjectiveMap.Find(CachedObjective->FailureObjectiveWorldID);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to find objective for key: %d"), ObjectiveKey);
			}
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
		(*Objectives.Find(Key))->LogVerbose();
	}
}

void UDataManager::LogSparseObjectiveGraph()
{
	TArray<int32> Keys;
	this->Objectives.GetKeys(Keys);

	for (const int32 Key : Keys)
	{
		(*Objectives.Find(Key))->LogSparse();
	}
}

void UDataManager::LogVerboseObjectiveLists()
{
	UE_LOG(LogTemp, Display, TEXT("[Start Pending Objective List]"));

	for (const int32 Key : this->PendingObjectives)
	{
		this->Objectives[Key]->LogVerbose();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End Pending Objective List]"));
	
	UE_LOG(LogTemp, Display, TEXT("[Start Running Objective List]"));

	for (const int32 Key : this->RunningObjectives)
	{
		this->Objectives[Key]->LogVerbose();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End Running Objective List]"));

	UE_LOG(LogTemp, Display, TEXT("[Start NoTimer Objective List]"));

	for (const int32 Key : this->NoTimerObjectives)
	{
		this->Objectives[Key]->LogVerbose();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End NoTimer Objective List]"));
}

void UDataManager::LogSparseObjectiveLists()
{
	UE_LOG(LogTemp, Display, TEXT("[Start Pending Objective List]"));

	for (const int32 Key : this->PendingObjectives)
	{
		this->Objectives[Key]->LogSparse();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End Pending Objective List]"));
	
	UE_LOG(LogTemp, Display, TEXT("[Start Running Objective List]"));

	for (const int32 Key : this->RunningObjectives)
	{
		this->Objectives[Key]->LogSparse();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End Running Objective List]"));

	UE_LOG(LogTemp, Display, TEXT("[Start NoTimer Objective List]"));

	for (const int32 Key : this->NoTimerObjectives)
	{
		this->Objectives[Key]->LogSparse();
	}
	
	UE_LOG(LogTemp, Display, TEXT("[End NoTimer Objective List]"));
}

void UDataManager::AddObjectiveRunning(const UObjective* NewObjective)
{
	NewObjective->SetInProgress();
	
	if (RunningObjectives.IsEmpty())
	{
		RunningObjectives.Add(NewObjective->WorldID);
		this->UpdateRunningTimerUntilNextObjective();
	}
	else
	{
		for (int i = 0; i < RunningObjectives.Num(); i++)
		{
			if (this->Objectives[this->RunningObjectives[i]]->ObjectiveTime.EndTime > NewObjective->ObjectiveTime.EndTime)
			{
				RunningObjectives.Insert(NewObjective->WorldID,i);

				if (i == 0)
				{//We inserted into the first position Update the Timer!
					this->UpdateRunningTimerUntilNextObjective();
				}
				break;
			}//if
		}//for
	}//if
}

void UDataManager::AddObjectivePending(const UObjective* NewObjective)
{
	if (NewObjective)
	{
		NewObjective->SetPending();
	}
	
	if (PendingObjectives.IsEmpty())
	{
		PendingObjectives.Add(NewObjective->WorldID);
		this->UpdatePendingTimerUntilNextObjective();
	}
	else
	{
		for (int i = 0; i < PendingObjectives.Num(); i++)
		{
			if (this->Objectives[this->PendingObjectives[i]]->ObjectiveTime.StartTime > NewObjective->ObjectiveTime.StartTime)
			{
				PendingObjectives.Insert(NewObjective->WorldID,i);

				if (i == 0)
				{//We inserted into the first position Update the Timer!
					this->UpdatePendingTimerUntilNextObjective();
				}
				break;
			}//if
		}//for
	}//if
}

void UDataManager::AddObjectiveNoTimer(const UObjective * NewObjective)
{
	NoTimerObjectives.Add(NewObjective->WorldID);
}

void UDataManager::UpdateRunningTimerUntilNextObjective()
{
	if (!RunningObjectives.IsEmpty())
	{//Update the set timer with whatever is in the 0th position
		const float Rate = static_cast<float>(FMath::Min(this->Objectives[this->RunningObjectives[0]]->ObjectiveTime.EndTime - FDateTime::UtcNow().ToUnixTimestamp(), 0));
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
		const float Rate = static_cast<float>(FMath::Min(this->Objectives[this->PendingObjectives[0]]->ObjectiveTime.StartTime - FDateTime::UtcNow().ToUnixTimestamp(),0));
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
		const UObjective * Objective = this->Objectives[this->RunningObjectives[0]];

		//Add the Failure Objective IF it exists to the list of running objectives
		if (Objective->FailureObjective)
		{
			this->AddObjective(Objective->FailureObjective);
		}

		this->RunningObjectives.RemoveAt(0);
		this->CallObjectiveTimerExpiredDelegate(Objective);
		this->UpdateRunningTimerUntilNextObjective();
	}	
}

void UDataManager::ObjectivePendingTimerComplete()
{
	if (!this->PendingObjectives.IsEmpty())
	{
		const UObjective * Objective = this->Objectives[this->PendingObjectives[0]];

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
		this->UpdatePendingTimerUntilNextObjective();
	}
}

void UDataManager::CallObjectiveCompleteDelegate(const UObjective * Objective) const
{
	if (ObjectiveCompleteDelegate.IsBound())
	{
		ObjectiveCompleteDelegate.Broadcast(Objective->GetObjectiveData());
	}
}

void UDataManager::CallObjectiveFailedDelegate(const UObjective * Objective) const
{
	if (ObjectiveFailedDelegate.IsBound())
	{
		ObjectiveFailedDelegate.Broadcast(Objective->GetObjectiveData());
	}
}

void UDataManager::CallObjectiveTimerExpiredDelegate(const UObjective * Objective) const
{
	if (ObjectiveExpiredDelegate.IsBound())
	{
		ObjectiveExpiredDelegate.Broadcast(Objective->GetObjectiveData());
	}
}

void UDataManager::CallObjectiveTimerStartedDelegate(const UObjective * Objective) const
{
	if (ObjectiveStartedDelegate.IsBound())
	{
		ObjectiveStartedDelegate.Broadcast(Objective->GetObjectiveData());
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
