// Project Watcher 2024 & Beyond.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GameManagement/Objective_System/Objectives/Objective.h"
#include "ObjectiveGraphSupport.generated.h"

/**
 * Used to support an Objective Graph
 */
UCLASS()
class PROJECT_WATCHER_API UObjectiveGraphSupport : public UObject
{
	GENERATED_BODY()

	/* Graph that gets built from the Root Objective during serialization */
	UPROPERTY()
	TMap<int32, UObjective*> ObjectiveGraph;

public:
	
	/**
	 * Creates an ObjectiveGraphSerializer
	 * @return Initialized ObjectiveGraphSerializer
	 */
	static UObjectiveGraphSupport * Make();

	/**
	 * Serializes the given Objective into an Objective Graph
	 * @param Objective Root Objective
	 * @return Serialized ObjectiveGraph
	 */
	TMap<int32, UObjective*> SerializeObjective(UObjective * Objective);

	/**
	 * Serializes a group of Objectives into one Graph map
	 * @param Objectives Objectives
	 * @return Serialized ObjectiveGraph
	 */
	TMap<int32, UObjective*> SerializeObjectiveList(const TArray<UObjective*>& Objectives);

	/**
	 * Serializes an Objective then returns the resulting graph formed from ObjectiveData
	 * @param Objective Source Objective
	 * @return Deep Copy of Objective Graph
	 */
	TMap<int32, FObjectiveData> GetObjectiveDataGraphFromObjective(UObjective * Objective);
	
private:
	
	/**
	 * Recursive Objective Serializer
	 * @param Objective Local Root Objective in the graph
	 */
	void SerializeGraph(UObjective * Objective);
};
