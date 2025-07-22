// Project Watcher 2024 & Beyond.


#include "GameManagement/Objective_System/Support/ObjectiveGraphSupport.h"

UObjectiveGraphSupport * UObjectiveGraphSupport::Make()
{
	UObjectiveGraphSupport * ObjectiveGraphSerializer = NewObject<UObjectiveGraphSupport>();	
	return ObjectiveGraphSerializer;
}

TMap<int32, UObjective*> UObjectiveGraphSupport::SerializeObjective(UObjective * Objective)
{
	this->ObjectiveGraph.Empty();
	this->SerializeGraph(Objective);
	return this->ObjectiveGraph;
}

TMap<int32, UObjective*> UObjectiveGraphSupport::SerializeObjectiveList(const TArray<UObjective*>& Objectives)
{
	this->ObjectiveGraph.Empty(Objectives.Num());

	for (UObjective * Objective : Objectives)
	{
		this->SerializeGraph(Objective);
	}
	
	return this->ObjectiveGraph;
}

TMap<int32, FObjectiveSave> UObjectiveGraphSupport::GetObjectiveSaveGraphFromObjectiveGraph(const TMap<int32, UObjective*>& ObjectiveGraph)
{
	TMap<int32, FObjectiveSave> ObjectiveSaves;

	TArray<int32> Keys;
	ObjectiveGraph.GetKeys(Keys);

	for (int32 Key : Keys)
	{
		ObjectiveSaves.Add(Key, (*ObjectiveGraph.Find(Key))->GetSaveState());
	}
	
	return ObjectiveSaves;
}

void UObjectiveGraphSupport::SerializeGraph(UObjective * Objective)
{
	//Add Root Objective if it isn't present in the TMap
	if (!this->ObjectiveGraph.Contains(Objective->WorldID))
	{
		this->ObjectiveGraph.Add(Objective->WorldID, Objective);
	}

	//Check Children, Only Recurse on them if they are not present in the SerializationMap
	if (Objective->SuccessObjective)
	{
		if (!this->ObjectiveGraph.Contains(Objective->SuccessObjective->WorldID))
		{
			this->SerializeGraph(Objective->SuccessObjective);
		}
	}

	if (Objective->FailureObjective)
	{
		if (!this->ObjectiveGraph.Contains(Objective->FailureObjective->WorldID))
		{
			this->SerializeGraph(Objective->FailureObjective);
		}
	}
}
