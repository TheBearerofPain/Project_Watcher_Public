//Project Watcher 2024 & Beyond

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataManagement.generated.h"

/**
 * Responsible for managing the data points that are relevant for
 * holo-table data replication
 */
UCLASS()
class PROJECT_WATCHER_API UDataManagement : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	//Initialization//

	UDataManagement() { }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;
};
