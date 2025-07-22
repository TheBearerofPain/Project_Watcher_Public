// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Objective_State_Types/ObjectiveState.h"
#include "Objective.generated.h"

/**
 * Time Data about the Source UObjective
 * All time is in Game Seconds
 */
USTRUCT(BlueprintType)
struct PROJECT_WATCHER_API FObjectiveTime
{
	GENERATED_BODY()

	/* StartTime of the Objective in GameSeconds after GameStart */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 StartTime = -1;

	/* EndTime of Objective in GameSeconds after GameStart */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 EndTime = -1;

	/* If True EndTime is ignored and the Objective runs until Complete or Failed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool NoTimer = false;

	/* Used for schedule time evaluation */
	
	/* If True the Start & End times get evaluated during scheduling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool EvaluateAtScheduleTime = false;

	/* Offset for Start time during scheduling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ScheduleStartTimeOffset = 0;

	/* Offset for end time during scheduling */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 ScheduleDuration = 0;

	/* Used for schedule time evaluation */

	/**
	 * Setup for NoTimer ObjectiveTime
	 * @param StartTimeIn Objective Start Time
	 */
	void SetupForImmediateEvaluation(const int64 StartTimeIn)
	{
		this->StartTime = StartTimeIn;
		this->EndTime = -1;
		this->NoTimer = true;
		this->EvaluateAtScheduleTime = false;
	}

	/**
	 * Setup for a time limited Objective
	 * @param StartTimeIn Objective Start Time
	 * @param EndTimeIn Objective End Time
	 */
	void SetupForImmediateEvaluation(const int64 StartTimeIn, const int64 EndTimeIn)
	{
		this->StartTime = StartTimeIn;
		this->EndTime = EndTimeIn;
		this->NoTimer = false;
		this->EvaluateAtScheduleTime = false;
	}

	/**
	 * Setup for an Objective time that will be based on the game time when this objective is getting scheduled
	 * @param ScheduleStartTimeOffsetIn How long do we wait after current game time until this objective is running
	 * @param ScheduleDurationIn How long this objective will run for
	 */
	void SetupForScheduledEvaluation(const int64 ScheduleStartTimeOffsetIn, const int64 ScheduleDurationIn)
	{
		this->ScheduleStartTimeOffset = ScheduleStartTimeOffsetIn;
		this->ScheduleDuration = ScheduleDurationIn;
		this->NoTimer = false;
		this->EvaluateAtScheduleTime = true;
	}

	/**
	 * Setup for a NoTimer Objective, that will be based on game time when this objective is getting scheduled
	 * @param ScheduleStartTimeOffsetIn How long do we wait after current game time until this objective is running
	 */
	void SetupForScheduledEvaluation(const int64 ScheduleStartTimeOffsetIn)
	{
		this->ScheduleStartTimeOffset = ScheduleStartTimeOffsetIn;
		this->ScheduleDuration = -1;
		this->NoTimer = true;
		this->EvaluateAtScheduleTime = true;
	}
};

/**
 * Special Save State Struct for UObjectives
 */
USTRUCT()
struct PROJECT_WATCHER_API FObjectiveSave
{
	GENERATED_BODY()

	/* WorldID of Objective */
	UPROPERTY(SaveGame)
	int32 WorldID = -1;

	/* Title of Objective */
	UPROPERTY(SaveGame)
	FString Title = TEXT("");

	/* Description of Objective */
	UPROPERTY(SaveGame)
	FString Description = TEXT("");

	/* ObjectiveTime of Objective */
	UPROPERTY(SaveGame)
	FObjectiveTime ObjectiveTime;

	/* ObjectiveState of Objective */
	UPROPERTY(SaveGame)
	UObjectiveState * ObjectiveState = nullptr;

	//AActor Marker
	//For now Ignore because certain markers are attached to actors that need to be spawned first
	//Since I don't know what is going to be spawned

	/* WorldID of SuccessObjective for this Objective */
	UPROPERTY(SaveGame)
	int32 SuccessObjectiveWorldID = -1;

	/* WorldID of FailureObjective for this Objective */
	UPROPERTY(SaveGame)
	int32 FailureObjectiveWorldID = -1;

	/* Flag for if the Objective was added into DataManager, otherwise it existed only as a reference somewhere in the objective graph */
	UPROPERTY(SaveGame)
	bool Added = false;
};

/**
 * Data Struct for when we update the source UObjective
 * OR
 * Have updates regarding the source UObjective
 */
USTRUCT(BlueprintType)
struct PROJECT_WATCHER_API FObjectiveData
{
	GENERATED_BODY()

	/* WorldID of the Objective, Read Only */
	UPROPERTY(BlueprintType, BlueprintReadOnly)
	int32 WorldID = -1;

	/* Title of the Objective */
	UPROPERTY(BlueprintType, BlueprintReadOnly)
	FString Title = TEXT("");

	/* Description of the Objective */
	UPROPERTY(BlueprintType, BlueprintReadOnly)
	FString Description = TEXT("");

	/* ObjectiveTime, Read Only */
	UPROPERTY(BlueprintType, BlueprintReadOnly)
	FObjectiveTime ObjectiveTime;
	
	/* ObjectiveState is a fresh Object not the source in order to prevent accidental tampering with the objective */
	UPROPERTY(BlueprintType, BlueprintReadWrite)
	UObjectiveState * ObjectiveState = nullptr;

	/* Original ObjectiveMarker associated with the Objective */
	UPROPERTY(BlueprintType, BlueprintReadOnly)
	AActor * ObjectiveMarker = nullptr;
};

/**
 * Objective
 */
UCLASS(Blueprintable)
class PROJECT_WATCHER_API UObjective : public UObject
{
	GENERATED_BODY()
	
public:

	/* WorldID of this Objective */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	int32 WorldID = -1;

	/* Title of this Objective */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FString Title = TEXT("");

	/* Description of this Objective */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FString Description = TEXT("");

	/* Objective Time */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FObjectiveTime ObjectiveTime;

	/* ObjectiveState, Used to evaluate if this Objective is Complete, Pending, InProgress or Failed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	UObjectiveState * ObjectiveState = nullptr;

	/* If Set, refers to a Marker in the world associated with this Objective */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	AActor * ObjectiveMarker = nullptr;

	/* SuccessObjective, If set gets queued up if this Objective Succeeds */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	UObjective * SuccessObjective = nullptr;

	/* This is used as a temp var store when reinitializing from a SaveState & NOTHING ELSE */
	int32 SuccessObjectiveWorldID = -1;

	/* FailureObjective, If set gets queued up if this Objective Fails */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	UObjective * FailureObjective = nullptr;

	/* This is used as a temp var store when reinitializing from a SaveState & NOTHING ELSE */
	int32 FailureObjectiveWorldID = -1;

	/* Flag for if the Objective was scheduled in the DataManager, otherwise it existed only as a reference somewhere in the objective graph */
	bool Scheduled = false;
	
public:
	/**
	 * Static Initializer for UObjective
	 * @param Save The Data we use to Initialize the Objective
	 * @return The Initialized Objective
	 */
	static UObjective * Make(const FObjectiveSave& Save);

	/**
	 * Logs Verbose information to Console about this Objective
	 */
	void LogVerbose() const;

	/**
	 * Logs Sparse information to Console about this Objective
	 */
	void LogSparse() const;

	/**
	 * Set the given data to this Objective
	 * @param WorldIDIn WorldID
	 * @param TitleIn Title
	 * @param DescriptionIn Description 
	 * @param ObjectiveStateIn ObjectiveState
	 * @param ObjectiveTimeIn ObjectiveTime
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void Setup(const int32 WorldIDIn, const FString& TitleIn, const FString& DescriptionIn, UObjectiveState * ObjectiveStateIn, const FObjectiveTime& ObjectiveTimeIn);

	/**
	 * Sets the ObjectiveMarker for this Objective
	 * @param ObjectiveMarkerIn ObjectiveMarker
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetObjectiveMarker(AActor * ObjectiveMarkerIn);

	/**
	 * Sets the SuccessObjective for this Objective
	 * @param SuccessObjectiveIn SuccessObjective
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetSuccessObjective(UObjective * SuccessObjectiveIn);

	/**
	 * Sets the FailureObjective for this Objective
	 * @param FailureObjectiveIn FailureObjective
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetFailureObjective(UObjective * FailureObjectiveIn);

	/**
	 * Sets both Success & Failure Objectives for this Objective
	 * @param SuccessObjectiveIn SuccessObjective
	 * @param FailureObjectiveIn FailureObjective
	 */
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetSuccessAndFailureObjectives(UObjective * SuccessObjectiveIn, UObjective * FailureObjectiveIn);

	/**
	 * Sets the ObjectiveState to Pending
	 */
	void SetPending() const;

	/**
	 * Sets the ObjectiveState to InProgress
	 */
	void SetInProgress() const;

	/**
	 * Evaluates the ObjectiveState
	 * @return Evaluated ObjectiveState
	 */
	EObjectiveState Evaluate() const;

	/**
	 * Get the SaveState Struct for this Objective
	 * @return SaveState
	 */
	FObjectiveSave GetSaveState();

	/**
	 * Restores the SaveState for this Objective
	 * @param SaveState SaveState we are restoring from
	 */
	void RestoreSaveState(const FObjectiveSave& SaveState);

	/**
	 * Gets the ObjectiveData for this Objective
	 * @return ObjectiveData for this Objective
	 */
	FObjectiveData GetObjectiveData() const;

	/**
	 * Updates ObjectiveState by Making a partial deep copy of the given ObjectiveState,
	 * We don't deep copy any SubObjective Markers
	 * @param ObjectiveStateIn ObjectiveState
	 */
	void UpdateObjectiveState(UObjectiveState * ObjectiveStateIn) const;
};
