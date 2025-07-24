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
 * Objective Representation after serialization
 */
USTRUCT(BlueprintType)
struct PROJECT_WATCHER_API FObjectiveData
{
	GENERATED_BODY()

	/* WorldID of the Objective */
	UPROPERTY(BlueprintType, BlueprintReadOnly, SaveGame)
	int32 WorldID = -1;

	/* Tag of the Objective */
	UPROPERTY(BlueprintType, BlueprintReadOnly, SaveGame)
	FString Tag = TEXT("");

	/* Title of the Objective */
	UPROPERTY(BlueprintType, BlueprintReadOnly, SaveGame)
	FString Title = TEXT("");

	/* Description of the Objective */
	UPROPERTY(BlueprintType, BlueprintReadOnly, SaveGame)
	FString Description = TEXT("");

	/* ObjectiveTime, Read Only */
	UPROPERTY(BlueprintType, BlueprintReadOnly, SaveGame)
	FObjectiveTime ObjectiveTime;
	
	/* ObjectiveState is a fresh Object not the source in order to prevent accidental tampering with the objective */
	UPROPERTY(BlueprintType, BlueprintReadWrite, SaveGame)
	UObjectiveState * ObjectiveState = nullptr;

	/* Original ObjectiveMarker associated with the Objective, Not included in save state */
	UPROPERTY(BlueprintType, BlueprintReadOnly)
	AActor * ObjectiveMarker = nullptr;

	/* SuccessObjective WorldID */
	UPROPERTY(SaveGame)
	int32 SuccessObjectiveWorldID = -1;

	/* FailureObjective WorldID */
	UPROPERTY(SaveGame)
	int32 FailureObjectiveWorldID = -1;

	/* Whether this objective was scheduled or not */
	UPROPERTY(SaveGame)
	bool Scheduled = false;

	/* Whether this Objective is running or not */
	UPROPERTY(SaveGame)
	bool Running = false;

	/**
	 * Logs sparse data about the objective
	 */
	void LogSparse() const;

	/**
	 * Logs verbose data about the objective
	 */
	void LogVerbose() const;

	/**
	 * Sets the ObjectiveState to Pending
	 */
	void SetPending();

	/**
	 * Sets the ObjectiveState to InProgress
	 */
	void SetInProgress();

	/**
	 * Evaluates the ObjectiveState
	 * @return Evaluated ObjectiveState
	 */
	EObjectiveState Evaluate() const;

	/**
	 * Updates ObjectiveState by Making a partial deep copy of the given ObjectiveState,
	 * We don't deep copy any SubObjective Markers
	 * @param ObjectiveStateIn ObjectiveState
	 */
	void UpdateObjectiveState(UObjectiveState * ObjectiveStateIn) const;

	FObjectiveData GetCopy() const;
};

/**
 * Objective, Used to construct an objective graph
 */
UCLASS(Blueprintable)
class PROJECT_WATCHER_API UObjective : public UObject
{
	GENERATED_BODY()
	
public:

	/* WorldID of this Objective */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	int32 WorldID = -1;

	/* Tag for this objective that can be used to associate this Objective with external events */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FString Tag = TEXT("");

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

	/* FailureObjective, If set gets queued up if this Objective Fails */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	UObjective * FailureObjective = nullptr;
	
public:

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
	 * Sets the Tag for this Objective
	 * @param TagIn The Tag we are using for this Objective
	 */
	void SetTag(const FString& TagIn);

	/**
	 * Gets the ObjectiveData for this Objective
	 * @return ObjectiveData for this Objective
	 */
	FObjectiveData GetObjectiveData() const;
};
