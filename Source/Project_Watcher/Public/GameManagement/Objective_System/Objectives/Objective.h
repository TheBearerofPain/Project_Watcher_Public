// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Objective_State_Types/ObjectiveState.h"
#include "Objective.generated.h"

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
	int64 StartTime = -1;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	int64 EndTime = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Objective")
	bool NoTimer = false;//Use this flag if you don't want this objective to time out

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
	void SetObjectiveData(const int32 WIDIn, const FString& TitleIn, const FString& DescriptionIn, UObjectiveState * ObjectiveDataIn, const int64 StartTimeIn, const int64 EndTimeIn);

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetObjectiveDataNoTimer(const int32 WIDIn, const FString& TitleIn, const FString& DescriptionIn, UObjectiveState * ObjectiveDataIn, const int64 StartTimeIn);

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetObjectiveMarker(AActor * ObjectiveMarkerIn);

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetSuccessObjective(UObjective * SuccessObjectiveIn);

	UFUNCTION(BlueprintCallable, Category = "Objective")
	void SetFailureObjective(UObjective * FailureObjectiveIn);

	void SetPending() const;

	void SetInProgress() const;
	
	EObjectiveState Evaluate() const;
};

