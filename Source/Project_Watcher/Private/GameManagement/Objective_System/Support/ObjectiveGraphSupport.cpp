// Project Watcher 2024 & Beyond.

#include "GameManagement/Objective_System/Support/ObjectiveGraphSupport.h"

UObjectiveGraphSupport * UObjectiveGraphSupport::Make()
{
	UObjectiveGraphSupport * ObjectiveGraphSerializer = NewObject<UObjectiveGraphSupport>();

	if (!IsValid(ObjectiveGraphSerializer))
	{
		ensureMsgf(false, TEXT("UObjectiveGraphSerializer::Make failed: Object is Invalid"));
		return nullptr;
	}
	
	return ObjectiveGraphSerializer;
}

TMap<int32, UObjective*> UObjectiveGraphSupport::SerializeObjective(UObjective * Objective)
{	
	this->ObjectiveGraph.Empty();

	if (IsValid(Objective))
	{
		this->SerializeGraph(Objective);
	}
	else
	{
		ensureMsgf(false, TEXT("SerializeObjective failed: Objective is Invalid"));
	}
	
	return this->ObjectiveGraph;
}

TMap<int32, UObjective*> UObjectiveGraphSupport::SerializeObjectiveList(const TArray<UObjective*>& Objectives)
{
	this->ObjectiveGraph.Empty(Objectives.Num());

	for (UObjective * Objective : Objectives)
	{
		if (IsValid(Objective))
		{
			this->SerializeGraph(Objective);
		}
		else
		{
			ensureMsgf(false, TEXT("SerializeObjectiveList failed: Objective is Invalid"));
		}
	}
	
	return this->ObjectiveGraph;
}

TMap<int32, FObjectiveData> UObjectiveGraphSupport::GetObjectiveDataGraphFromObjective(UObjective* Objective)
{
	TMap<int32, FObjectiveData> ObjectiveDataGraph;
	TMap<int32, UObjective*> SourceObjectiveGraph;
	
	if (IsValid(Objective))
	{
		SourceObjectiveGraph = this->SerializeObjective(Objective);
	}
	else
	{
		ensureMsgf(false, TEXT("GetObjectiveDataGraphFromObjective failed: Objective is Invalid"));
	}

	TArray<int32> Keys;
	SourceObjectiveGraph.GetKeys(Keys);

	for (const int32 Key : Keys)
	{
		if (const UObjective * CachedObjective = SourceObjectiveGraph[Key]; IsValid(CachedObjective))
		{
			ObjectiveDataGraph.Add(Key, CachedObjective->GetObjectiveData());
		}
		else
		{
			ensureMsgf(false, TEXT("GetObjectiveDataGraphFromObjective failed: Objective KeyPair is Invalid"));
		}
	}
	
	return ObjectiveDataGraph;
}

void UObjectiveGraphSupport::SerializeGraph(UObjective * Objective)
{
	if (IsValid(Objective))
	{
		if (!this->ObjectiveGraph.Contains(Objective->WorldID))
		{
			this->ObjectiveGraph.Add(Objective->WorldID, Objective);
		}

		/* Check Children, Only Recurse on them if they are not present in the SerializationMap */
	
		if (IsValid(Objective->SuccessObjective))
		{
			if (!this->ObjectiveGraph.Contains(Objective->SuccessObjective->WorldID))
			{
				this->SerializeGraph(Objective->SuccessObjective);
			}
		}

		if (IsValid(Objective->FailureObjective))
		{
			if (!this->ObjectiveGraph.Contains(Objective->FailureObjective->WorldID))
			{
				this->SerializeGraph(Objective->FailureObjective);
			}
		}
	}
	else
	{
		ensureMsgf(false, TEXT("SerializeGraph failed: Objective is Invalid"));
	}
}
