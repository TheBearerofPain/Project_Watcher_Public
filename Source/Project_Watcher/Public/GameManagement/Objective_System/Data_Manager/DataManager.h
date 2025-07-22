// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameManagement/Objective_System/Objectives/Objective.h"
#include "DataManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FObjectiveDelegate, const FObjectiveData&, Objective);

USTRUCT()
struct PROJECT_WATCHER_API FDataManagerSaveState
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	int32 WorldID = -1;
	
	UPROPERTY(SaveGame)
	int64 CurrentGameTime = -1;
	
	UPROPERTY(SaveGame)
	TMap<int32, FObjectiveSave> Objectives;
};

//@TODO figure out how to do seasons in a nice configurable way

UENUM(BlueprintType)
enum ETimeAnchor
{
	GameStart UMETA(DisplayName = "Game Start"),
	GameCurrent UMETA(DisplayName = "Game Current"),
};

typedef TPair<ETimeAnchor,int64> FTAnchor;

/**
 * Used to manage world data, Functions Include:
 * Managing Objectives,
 * Managing GameTime,
 * Generating World IDs,
 * Saving & Restoring Objective State,
 * Saving & Restoring GameTime
 * Saving & Restoring World ID Counter
 */
UCLASS(Blueprintable)
class PROJECT_WATCHER_API UDataManager : public UObject
{
	GENERATED_BODY()
private:

	//Objective State//

	/*
	 * Objectives have the following life cycle,
	 * UnScheduled, They are present in the ObjectiveMap, But NOT in any lists.
	 * Pending, They are NOT actively running but are queued up to start in the future.
	 * Running, They ARE actively running, and will expire if given enough time, these can ALSO be failed or completed.
	 * NoTimer, They ARE actively running, and will NOT expire, they can only be failed or completed.
	 *
	 * End of Life, Once an Objective Expires, Fails or Completes, any child objectives will be queued up & the parent
	 * Objective is removed from the Map & Any lists it resided within.
	 */
	
	/* Pending Objective WorldID's */
	TArray<int32> PendingObjectives;

	/* Running Objective WorldID's */
	TArray<int32> RunningObjectives;

	/* NoTimer Objective WorldID's */
	TArray<int32> NoTimerObjectives;

	/* ObjectiveMap */
	UPROPERTY()
	TMap<int32, UObjective*> Objectives;

	//Objective Timer Handle for when the next soonest objective expires
	FTimerHandle RunningObjectiveTimerHandle;

	//Objective Timer Handle for when the next soonest objective starts
	FTimerHandle PendingObjectiveTimerHandle;

	//Objective State//

	//Game Time State//
	
	/**
	 *	This date is the Day our game starts on. As current game time progress to get the current date, we simply
	 *	convert GameAnchorDate to UnixTime + CurrentGameTime, then convert it back into date time to get the new date.
	 * 
	 *	To Convert back to GameTime
	 *	FDateTime::ToUnixTimeStamp - GameAnchorDate::ToUnixTimeStamp;
	 *
	 *	Game Starts on December 13, 2332 @ 3:00am
	 */
	const FDateTime GameAnchorDate = FDateTime(2332, 12, 13, 3, 0, 0, 0);

	/* This is the current game time in seconds, game start is 0 */
	int64 CurrentGameTime = 0;

	/* The Game time is sped up with this multiplier */
	const int64 GameTimeMultiplier = 24;

	/* Timer Handle for updating game time */
	FTimerHandle GameTimeTimerHandle;

	/* Map of TimeAnchors for various checkpoints in the game */
	TMap<ETimeAnchor, int64> TimeAnchorMap = {FTAnchor(GameStart,0)};
	
	//Game Time State//

	//World ID State//

	int32 WorldID = 0;
	
	//World ID State//

public:

	//Objective State//
	
	//Objective Delegate fires when an objective's completed state has been evaluated as completed
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Objective")
	FObjectiveDelegate ObjectiveCompleteDelegate;

	//Objective Delegate fires when an objective's completed state has been evaluated as failed
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Objective")
	FObjectiveDelegate ObjectiveFailedDelegate;
	
	//Objective Delegate fires when an objective's timer has expired
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Objective")
	FObjectiveDelegate ObjectiveExpiredDelegate;

	//Objective Delegate fires when an objective's timer has started
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Objective")
	FObjectiveDelegate ObjectiveStartedDelegate;

	/**
	 * Adds a new objective to the processor
	 * @param NewObjective The Objective we are adding
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void AddObjective(UObjective * NewObjective);

	/**
	 * Creates an Objective
	 * @param TitleIn Title
	 * @param DescriptionIn Description
	 * @param ObjectiveStateIn Objective State , How the Objective will be evaluated
	 * @param ObjectiveTimeIn Objective Time
	 * @return Initialized Objective
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective")
	UObjective * CreateObjective(const FString& TitleIn, const FString& DescriptionIn, UObjectiveState * ObjectiveStateIn, const FObjectiveTime& ObjectiveTimeIn);

	/**
	 * Creates an Objective
	 * @param TitleIn Title 
	 * @param DescriptionIn Description
	 * @param ObjectiveStateIn Objective State, How the Objective will be evaluated
	 * @param ObjectiveTimeIn Objective Time
	 * @param SuccessObjectiveIn SuccessObjective
	 * @return Initialized Objective
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective")
	UObjective * CreateObjectiveWithSuccessObjective(const FString& TitleIn, const FString& DescriptionIn, UObjectiveState * ObjectiveStateIn, const FObjectiveTime& ObjectiveTimeIn, UObjective * SuccessObjectiveIn);

	/**
	 * Creates Objective
	 * @param TitleIn Title
	 * @param DescriptionIn Description 
	 * @param ObjectiveStateIn Objective State, How the Objective will be evaluated
	 * @param ObjectiveTimeIn Objective Time
	 * @param FailureObjectiveIn FailureObjective
	 * @return Initialized Objective
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective")
	UObjective * CreateObjectiveWithFailureObjective(const FString& TitleIn, const FString& DescriptionIn, UObjectiveState * ObjectiveStateIn, const FObjectiveTime& ObjectiveTimeIn, UObjective * FailureObjectiveIn);

	/**
	 * Creates Objective
	 * @param TitleIn Title
	 * @param DescriptionIn Description 
	 * @param ObjectiveStateIn Objective State, How the Objective will be evaluated
	 * @param ObjectiveTimeIn Objective Time
	 * @param SuccessObjectiveIn SuccessObjective
	 * @param FailureObjectiveIn FailureObjective
	 * @return Initialized Objective
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Objective")
	UObjective * CreateObjectiveWithSuccessAndFailureObjectives(const FString& TitleIn, const FString& DescriptionIn, UObjectiveState * ObjectiveStateIn, const FObjectiveTime& ObjectiveTimeIn, UObjective * SuccessObjectiveIn, UObjective * FailureObjectiveIn);
	
	/**
	 * Updates the Objective with the matching WID, with the given state, We ONLY search running objectives to update
	 * @param WorldIDIn The World ID we are looking for
	 * @param ObjectiveStateIn The State we are updating with
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void UpdateObjectiveState(const int32 WorldIDIn, UObjectiveState * ObjectiveStateIn);

	//Objective State//

	//Game Time State//

	//FDateTime Accessors & Conversions//
	
	/**
	 * Returns the Current Game Time in seconds since game start
	 * @return The Current Game Time
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	int64 GetCurrentGameSeconds() const;

	/**
	 * Converts Real World seconds to Game Seconds
	 * @param RealWorldSeconds Real World Seconds
	 * @return Game Seconds in the Real World Seconds Provided
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	int64 RealWorldSecondsToGameSeconds(const int64 RealWorldSeconds) const;

	/**
	 * Converts RealWorldMinutes to GameSeconds
	 * @param RealWorldMinutes RealWorldMinutes
	 * @return Game Seconds in the Real World Minutes provided
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	int64 RealWorldMinutesToGameSeconds(const int64 RealWorldMinutes) const;

	/**
	 * Converts Real World Hours to Game Seconds
	 * Use this if you want an objective to take RealWorld Hours
	 * @param RealWorldHours Real World Hours
	 * @return Amount of Game Seconds in the Real World Hours Provided
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	int64 RealWorldHoursToGameSeconds(const int64 RealWorldHours) const;

	/**
	 * Converts RealWorld Hours & Minutes to Game Seconds
	 * Use this if you want an objective to take RealWorld Hours & Minutes
	 * @param RealWorldHours Real World Hours
	 * @param RealWorldMinutes Real World Minutes
	 * @return Amount of Game Seconds in the Provided Real World Hours & Minutes
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	int64 RealWorldHoursAndMinutesToGameSeconds(const int64 RealWorldHours, const int64 RealWorldMinutes) const;

	/**
	 * Takes an FDateTime & converts it to seconds since game start
	 * @param GameDate GameDate
	 * @return The Date in GameTime Seconds, Returned time will be 0 if it takes place before GameStart
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	int64 GameDateToGameSeconds(const FDateTime& GameDate) const;
	
	/**
	 * Takes in GameSeconds and Converts it into a Date
	 * @param GameSeconds The Game Seconds we input
	 * @return The Game Date
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	FDateTime GameSecondsToGameDate(const int64 GameSeconds) const;
	
	/**
	 * Computes GameDate.ToUnixTimeStamp + CurrentGameTime,
	 * Then Converts that back into FDateTime
	 * @return The Current Game Date
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	FDateTime GetCurrentGameDate() const;

	/**
	 * Returns the Objective StartTime as a Date
	 * @param ObjectiveTime The ObjectiveTime we are extracting the StartDate for
	 * @return The Objective StartDate
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	FDateTime GetObjectiveStartDate(const FObjectiveTime& ObjectiveTime) const;

	/**
	 * Returns the Objective EndTime as a Date
	 * @param ObjectiveTime The ObjectiveTime we are extracting the EndDate for
	 * @return The Objective EndDate
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	FDateTime GetObjectiveEndDate(const FObjectiveTime& ObjectiveTime) const;
	
	//FDateTime Accessors & Conversions//
	
	//Creators for ObjectiveTime//
	
	/**
	 * Creates an ObjectiveTime With a Delay in Seconds from the given Time Anchor
	 * @param TimeAnchor The TimeAnchor for the Objective, EXAMPLE: GameStart or the Start of a Season
	 * @param DelayInSeconds How long in seconds from the TimeAnchor we want to wait
	 * @param DurationInSeconds How Long the Objective will run for in seconds
	 * @return The setup ObjectiveTime
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	FObjectiveTime CreateDelayedObjectiveTime(const ETimeAnchor TimeAnchor,const int64 DelayInSeconds, const int64 DurationInSeconds);

	/**
	 * Creates an Objective with NoTimer
	 * @param TimeAnchor The TimeAnchor for the Objective, EXAMPLE: GameStart or the Start of a Season
	 * @param DelayInSeconds How long in seconds from the TimeAnchor we want to wait
	 * @return The setup ObjectiveTime
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	FObjectiveTime CreateDelayedConstantObjectiveTime(const ETimeAnchor TimeAnchor, const int64 DelayInSeconds);

	/**
	 * Creates an Objective from the StartDate & EndDate
	 * @param StartDate The StartDate of the Objective
	 * @param EndDate The EndDate of the Objective
	 * @return The setup ObjectiveTime
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	FObjectiveTime CreateObjectiveTimeFromDate(const FDateTime& StartDate, const FDateTime& EndDate) const;

	/**
	 * Creates an Objective From the StartDate with a specified duration
	 * @param StartDate The StartDate of the Objective
	 * @param DurationInSeconds The Duration of the objective in Real World Seconds
	 * @return The setup ObjectiveTime
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	FObjectiveTime CreateObjectiveTimeFromDateWithDuration(const FDateTime& StartDate, const int64 DurationInSeconds) const;

	/**
	 * Creates an Objective with NoTimer From a given Date
	 * @param StartDate The StartDate of the Objective
	 * @return The setup ObjectiveTime
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ObjectiveTime")
	FObjectiveTime CreateConstantObjectiveTimeFromDate(const FDateTime& StartDate) const;

	//Creators for ObjectiveTime//
	
	//Game Time State//

	/**
	 * Generates a new Unique World ID
	 * @return The new World ID
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "World ID")
	int32 GetWorldID();

	//Save State//

	UFUNCTION(BlueprintCallable, Category = "SaveTesting")
	void TestSave();

	UFUNCTION(BlueprintCallable, Category = "SaveTesting")
	void TestLoad();

	//Save State//

	// Logging //

	/**
	 * Logs the Objective Graph with verbose detail
	 */
	void LogVerboseObjectiveGraph();

	/**
	 * Logs the Objective Graph with sparse detail
	 */
	void LogSparseObjectiveGraph();

	/**
	 * Logs the Objectives in the Objective Lists in Verbose Detail
	 */
	void LogVerboseObjectiveLists();

	/**
	 * Logs the Objectives in the Objective Lists in Sparse Detail
	 */
	void LogSparseObjectiveLists();
	
	// Logging //

private:

	// Removers //

	/**
	 * Removes the given Objective from the Objective Lists
	 * @param WorldIDIn Objective WorldID we are looking to remove
	 */
	void RemoveObjective(const int32 WorldIDIn);
	
	// Removers //
	
	// Save State //

	/**
	 * Gets the SaveState of the DataManager
	 * @return The FDataManagerSaveState
	 */
	FDataManagerSaveState GetSaveState();

	/**
	 * Restores the DataManager with a given SaveState
	 * @param SaveState The FDataManagerSaveState
	 */
	void RestoreSaveState(const FDataManagerSaveState& SaveState);
	
	// Save State //
	
	//Objective State//
	
	/**
	 * Adds an Objective to the RunningObjective List
	 * @param NewObjective The Objective we are adding
	 */
	void AddObjectiveRunning(const UObjective * NewObjective);

	/**
	 * Adds an Objective to the PendingObjective List
	 * @param NewObjective The Objective we are adding
	 */
	void AddObjectivePending(const UObjective * NewObjective);

	/**
	 * Adds an Objective to the NoTimerObjective List
	 * @param NewObjective The Objective we are adding
	 */
	void AddObjectiveNoTimer(const UObjective * NewObjective);
	
	/**
	 * Sets the timer until the next objective IF any
	 * otherwise it'll just clear the timer
	 */
	void UpdateRunningTimerUntilNextObjective();

	/**
	 * Sets the timer until the next pending objective is ready to
	 * be added to the running list. IF no other pending objectives exist
	 * then the timer will be cleared
	 */
	void UpdatePendingTimerUntilNextObjective();

	/**
	 * Gets called when the ObjectiveTimer finishes
	 * This will grab the [0] indexed objective and pass it along the
	 * TimerExpiredDelegate
	 */
	UFUNCTION()
	void ObjectiveRunningTimerComplete();

	/**
	 * Gets called when the PendingObjectiveTimer finishes
	 * This will grab the [0] indexed objective and pass it to the
	 * RunningObjective List
	 */
	UFUNCTION()
	void ObjectivePendingTimerComplete();

	/**
	 * Used to CallObjectiveCompleteDelegate
	 * @param Objective The Objective we are broadcasting as completed
	 */
	void CallObjectiveCompleteDelegate(const UObjective * Objective) const;

	/**
	 * Used to CallObjectiveFailedDelegate
	 * @param Objective The Objective we are broadcasting as failed
	 */
	void CallObjectiveFailedDelegate(const UObjective * Objective) const;

	/**
	 * Used to CallObjectiveTimerExpiredDelegate
	 * @param Objective The Objective we are broadcasting as expired
	 */
	void CallObjectiveTimerExpiredDelegate(const UObjective * Objective) const;

	/**
	 * Used to CallObjectiveStartedDelegate
	 * @param Objective The Objective we are broadcasting as started
	 */
	void CallObjectiveTimerStartedDelegate(const UObjective * Objective) const;

	/**
	 * Evaluates an objectives completion state
	 * @param Objective The objective we are evaluating
	 * @return True if the objective is complete, False if the objective is NOT complete
	 */
	static EObjectiveState EvaluateObjective(const UObjective * Objective);

	//Objective State//

	//Game time State//

	/**
	 * Computes the TimeAnchor
	 * @param TimeAnchor The type of anchor we want
	 * @return GameTime in seconds for the TimeAnchor
	 */
	int64 ComputeTimeAnchor(const ETimeAnchor TimeAnchor);

	/**
	 * Starts the Game Time timer
	 */
	void StartGameTimer();

	/**
	 * Stops the Game Time timer
	 */
	void StopGameTimer();
	
	/**
	 * Increments the CurrentGameTime by GameTimeMultiplier
	 */
	UFUNCTION()
	void UpdateCurrentGameTime();
	
	//Game Time State//
	
};
