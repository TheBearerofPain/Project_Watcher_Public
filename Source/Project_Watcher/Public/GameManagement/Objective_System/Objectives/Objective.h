// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Objective_State_Types/ObjectiveState.h"
#include "Objective.generated.h"

USTRUCT(BlueprintType)
struct PROJECT_WATCHER_API FObjectiveTime
{
	GENERATED_BODY()
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 StartTime = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int64 EndTime = -1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool NoTimer = false;

	/**
	 * Default Constructor
	 */
	FObjectiveTime()
	{
		StartTime = -1;
		EndTime = -1;
		NoTimer = false;
	}

	/**
	 * NoTimer Constructor
	 * @param StartTimeIn StartTime
	 */
	FObjectiveTime(const int64 StartTimeIn)
	{
		StartTime = StartTimeIn;
		EndTime = -1;
		NoTimer = true;
	}

	/**
	 * Normal Constructor
	 * @param StartTimeIn StartTime 
	 * @param EndTimeIn EndTime
	 */
	FObjectiveTime(const int64 StartTimeIn, const int64 EndTimeIn)
	{
		StartTime = StartTimeIn;
		EndTime = EndTimeIn;
		NoTimer = false;
	}
};

/**
 * Special Save State Struct for UObjectives
 */
USTRUCT()
struct PROJECT_WATCHER_API FObjectiveSave
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	int32 WID = -1;

	UPROPERTY(SaveGame)
	FString Title = TEXT("");

	UPROPERTY(SaveGame)
	FString Description = TEXT("");
	
	UPROPERTY(SaveGame)
	FObjectiveTime ObjectiveTime;
	
	UPROPERTY()
	UObjectiveState * ObjectiveState = nullptr;

	//AActor Marker
	//For now Ignore because certain markers are attached to actors that need to be spawned first
	//Since I don't know what is going to be spawned / how I'll ignore this as it is effectively useless

	UPROPERTY(SaveGame)
	int32 SuccessObjectiveWID = -1;

	UPROPERTY(SaveGame)
	int32 FailureObjectiveWID = -1;

	/* Flag for if the Objective was added into DataManager, otherwise it existed only as a reference somewhere in the objective graph */
	UPROPERTY(SaveGame)
	bool Added = false;
};

/**
 * Objective
 */
UCLASS(Blueprintable)
class PROJECT_WATCHER_API UObjective : public UObject
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	int32 WID = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FString Title = TEXT("");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FString Description = TEXT("");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	FObjectiveTime ObjectiveTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	UObjectiveState * ObjectiveState = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	AActor * ObjectiveMarker = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	UObjective * SuccessObjective = nullptr;

	/* This is used as a temp var store when reinitializing from a SaveState & NOTHING ELSE */
	int32 SuccessObjectiveWID = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	UObjective * FailureObjective = nullptr;

	/* This is used as a temp var store when reinitializing from a SaveState & NOTHING ELSE */
	int32 FailureObjectiveWID = -1;

	/* Flag for if the Objective was added into DataManager, otherwise it existed only as a reference somewhere in the objective graph */
	bool Added = false;
	
public:

	static UObjective * Make(const FObjectiveSave& Save);
	
	void LogVerbose() const;

	void LogSparse() const;
	
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void Setup(const int32 WIDIn, const FString& TitleIn, const FString& DescriptionIn, UObjectiveState * ObjectiveStateIn, const FObjectiveTime& ObjectiveTimeIn);
	
	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetObjectiveMarker(AActor * ObjectiveMarkerIn);

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetSuccessObjective(UObjective * SuccessObjectiveIn);

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetFailureObjective(UObjective * FailureObjectiveIn);

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetSuccessAndFailureObjectives(UObjective * SuccessObjectiveIn, UObjective * FailureObjectiveIn);

	void SetPending() const;

	void SetInProgress() const;
	
	EObjectiveState Evaluate() const;

	FObjectiveSave GetSaveState();

	void RestoreSaveState(const FObjectiveSave& SaveState);
};
