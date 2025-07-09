// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TimeData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_WATCHER_API UTimeData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	
	static int64 GetTimeAnchor();

	static int64 GetStartTime(const int64 TimeAnchor, const int64 TimeOffset);
};
