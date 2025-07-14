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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	UObjective * FailureObjective = nullptr;
	
public:

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
};

