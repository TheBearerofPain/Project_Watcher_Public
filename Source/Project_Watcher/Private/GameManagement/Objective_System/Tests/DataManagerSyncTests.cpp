#include "CoreMinimal.h"
#include "GameManagement/Objective_System/Data_Manager/DataManager.h"
#include "GameManagement/Objective_System/Support/ObjectiveSupport.h"
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWorldIDCounterTest, "Project_Watcher.DataManager.Sync.WorldCounterTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FWorldIDCounterWrapperTest, "Project_Watcher.DataManager.Sync.WorldIDCounterWrapperTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCreateObjectiveTest, "Project_Watcher.DataManager.Sync.CreateObjectiveTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCreateObjectiveNullTest, "Project_Watcher.DataManager.Sync.CreateObjectiveNullTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCreateObjectiveTest_W_Success, "Project_Watcher.DataManager.Sync.CreateObjectiveTest_W_Success", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCreateObjectiveNullTest_W_Success, "Project_Watcher.DataManager.Sync.CreateObjectiveNullTest_W_Success", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCreateObjectiveTest_W_Failure, "Project_Watcher.DataManager.Sync.CreateObjectiveTest_W_Failure", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCreateObjectiveNullTest_W_Failure, "Project_Watcher.DataManager.Sync.CreateObjectiveNullTest_W_Failure", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCreateObjectiveTest_W_Success_And_Failure, "Project_Watcher.DataManager.Sync.CreateObjectiveTest_W_Success_And_Failure", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCreateObjectiveNullTest_W_Success_And_Failure, "Project_Watcher.DataManager.Sync.CreateObjectiveNullTest_W_Success_And_Failure", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAddObjectiveTest, "Project_Watcher.DataManager.Sync.AddObjectiveTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ClientContext)


/**
 * Test to see if the world id is being counted correctly
 */
bool FWorldIDCounterTest::RunTest(const FString& Parameters)
{
	constexpr int32 Runs = 10;
	bool Passed = true;

	UDataManager * DataManager = NewObject<UDataManager>();

	for (int32 i = 0; i < Runs-1; i++)
	{
		Passed &= DataManager->GetWorldID() == i;
	}
	
	return Passed;
}

bool FWorldIDCounterWrapperTest::RunTest(const FString& Parameters)
{
	constexpr int32 Runs = TNumericLimits<int32>::Max();
	bool Passed = true;

	UDataManager * DataManager = NewObject<UDataManager>();
	
	for (int32 i = 0; i < Runs; i++)
	{
		DataManager->GetWorldID();
	}

	const int32 FinalWorldID = DataManager->GetWorldID();
	UE_LOG(LogTemp, Display, TEXT("Final WorldID: %d"), FinalWorldID);
	
	Passed &= FinalWorldID == 0;
	
	return Passed;
}

/**
 * Tests to see if the created objective contains the correct data, based on what was given
 */
bool FCreateObjectiveTest::RunTest(const FString& Parameters)
{
	bool Passed = true;
	UDataManager * DataManager = NewObject<UDataManager>();
	const FString Title = TEXT("Title");
	const FString Description = TEXT("Description");
	UObjectiveBasicState * ObjectiveState = UObjectiveSupport::CreateBasicState();
	constexpr FObjectiveTime ObjectiveTime = FObjectiveTime();
	
	const UObjective * Objective = DataManager->CreateObjective(Title, Description, ObjectiveState, ObjectiveTime);

	Passed &= Objective->Title.Equals(Title);
	Passed &= Objective->Description.Equals(Description);
	Passed &= Objective->ObjectiveState != nullptr;
	Passed &= Objective->ObjectiveTime.StartTime == -1;
	Passed &= Objective->ObjectiveTime.EndTime == -1;
	
	return Passed;
}

bool FCreateObjectiveNullTest::RunTest(const FString& Parameters)
{
	bool Passed = true;
	UDataManager * DataManager = NewObject<UDataManager>();
	const FString Title = TEXT("Title");
	const FString Description = TEXT("Description");
	constexpr FObjectiveTime ObjectiveTime = FObjectiveTime();

	const UObjective * Objective = DataManager->CreateObjective(Title, Description, nullptr, ObjectiveTime);

	Passed &= Objective->Title.Equals(Title);
	Passed &= Objective->Description.Equals(Description);
	Passed &= Objective->ObjectiveState == nullptr;
	Passed &= Objective->ObjectiveTime.StartTime == -1;
	Passed &= Objective->ObjectiveTime.EndTime == -1;

	return Passed;
}

bool FCreateObjectiveTest_W_Success::RunTest(const FString& Parameters)
{
	bool Passed = true;
	UDataManager * DataManager = NewObject<UDataManager>();
	const FString Title = TEXT("Title");
	const FString Description = TEXT("Description");
	UObjectiveBasicState * ObjectiveState = UObjectiveSupport::CreateBasicState();
	constexpr FObjectiveTime ObjectiveTime = FObjectiveTime();

	UObjective * SuccessObjective = DataManager->CreateObjective(Title, Description, ObjectiveState, ObjectiveTime);
	const UObjective * Objective = DataManager->CreateObjectiveWithSuccessObjective(Title, Description, ObjectiveState, ObjectiveTime, SuccessObjective);
	
	Passed &= Objective->Title.Equals(Title);
	Passed &= Objective->Description.Equals(Description);
	Passed &= Objective->ObjectiveState != nullptr;
	Passed &= Objective->ObjectiveTime.StartTime == -1;
	Passed &= Objective->ObjectiveTime.EndTime == -1;
	Passed &= Objective->SuccessObjective != nullptr;
	
	return Passed;
}

bool FCreateObjectiveNullTest_W_Success::RunTest(const FString& Parameters)
{
	bool Passed = true;
	UDataManager * DataManager = NewObject<UDataManager>();
	const FString Title = TEXT("Title");
	const FString Description = TEXT("Description");
	constexpr FObjectiveTime ObjectiveTime = FObjectiveTime();
	
	const UObjective * Objective = DataManager->CreateObjectiveWithSuccessObjective(Title, Description, nullptr, ObjectiveTime, nullptr);
	
	Passed &= Objective->Title.Equals(Title);
	Passed &= Objective->Description.Equals(Description);
	Passed &= Objective->ObjectiveState == nullptr;
	Passed &= Objective->ObjectiveTime.StartTime == -1;
	Passed &= Objective->ObjectiveTime.EndTime == -1;
	Passed &= Objective->SuccessObjective == nullptr;
	
	return Passed;
}


bool FCreateObjectiveTest_W_Failure::RunTest(const FString& Parameters)
{
	bool Passed = true;
	UDataManager * DataManager = NewObject<UDataManager>();
	const FString Title = TEXT("Title");
	const FString Description = TEXT("Description");
	UObjectiveBasicState * ObjectiveState = UObjectiveSupport::CreateBasicState();
	constexpr FObjectiveTime ObjectiveTime = FObjectiveTime();
	
	UObjective * FailureObjective = DataManager->CreateObjective(Title, Description, ObjectiveState, ObjectiveTime);
	const UObjective * Objective = DataManager->CreateObjectiveWithFailureObjective(Title, Description, ObjectiveState, ObjectiveTime, FailureObjective);
	
	Passed &= Objective->Title.Equals(Title);
	Passed &= Objective->Description.Equals(Description);
	Passed &= Objective->ObjectiveState != nullptr;
	Passed &= Objective->ObjectiveTime.StartTime == -1;
	Passed &= Objective->ObjectiveTime.EndTime == -1;
	Passed &= Objective->FailureObjective != nullptr;
	
	return Passed;
}

bool FCreateObjectiveNullTest_W_Failure::RunTest(const FString& Parameters)
{
	bool Passed = true;
	UDataManager * DataManager = NewObject<UDataManager>();
	const FString Title = TEXT("Title");
	const FString Description = TEXT("Description");
	constexpr FObjectiveTime ObjectiveTime = FObjectiveTime();
	
	const UObjective * Objective = DataManager->CreateObjectiveWithFailureObjective(Title, Description, nullptr, ObjectiveTime, nullptr);
	
	Passed &= Objective->Title.Equals(Title);
	Passed &= Objective->Description.Equals(Description);
	Passed &= Objective->ObjectiveState == nullptr;
	Passed &= Objective->ObjectiveTime.StartTime == -1;
	Passed &= Objective->ObjectiveTime.EndTime == -1;
	Passed &= Objective->FailureObjective == nullptr;
	
	return Passed;
}

bool FCreateObjectiveTest_W_Success_And_Failure::RunTest(const FString& Parameters)
{
	bool Passed = true;
	UDataManager * DataManager = NewObject<UDataManager>();
	const FString Title = TEXT("Title");
	const FString Description = TEXT("Description");
	UObjectiveBasicState * ObjectiveState = UObjectiveSupport::CreateBasicState();
	constexpr FObjectiveTime ObjectiveTime = FObjectiveTime();

	UObjective * SuccessObjective = DataManager->CreateObjective(Title, Description, ObjectiveState, ObjectiveTime);
	UObjective * FailureObjective = DataManager->CreateObjective(Title, Description, ObjectiveState, ObjectiveTime);
	const UObjective * Objective = DataManager->CreateObjectiveWithSuccessAndFailureObjectives(Title, Description, ObjectiveState, ObjectiveTime, SuccessObjective, FailureObjective);
	
	Passed &= Objective->Title.Equals(Title);
	Passed &= Objective->Description.Equals(Description);
	Passed &= Objective->ObjectiveState != nullptr;
	Passed &= Objective->ObjectiveTime.StartTime == -1;
	Passed &= Objective->ObjectiveTime.EndTime == -1;
	Passed &= Objective->SuccessObjective != nullptr;
	Passed &= Objective->FailureObjective != nullptr;
	
	return Passed;
}

bool FCreateObjectiveNullTest_W_Success_And_Failure::RunTest(const FString& Parameters)
{
	bool Passed = true;
	UDataManager * DataManager = NewObject<UDataManager>();
	const FString Title = TEXT("Title");
	const FString Description = TEXT("Description");
	constexpr FObjectiveTime ObjectiveTime = FObjectiveTime();
	
	const UObjective * Objective = DataManager->CreateObjectiveWithSuccessAndFailureObjectives(Title, Description, nullptr, ObjectiveTime, nullptr, nullptr);
	
	Passed &= Objective->Title.Equals(Title);
	Passed &= Objective->Description.Equals(Description);
	Passed &= Objective->ObjectiveState == nullptr;
	Passed &= Objective->ObjectiveTime.StartTime == -1;
	Passed &= Objective->ObjectiveTime.EndTime == -1;
	Passed &= Objective->SuccessObjective == nullptr;
	Passed &= Objective->FailureObjective == nullptr;
	
	return Passed;
}

bool FAddObjectiveTest::RunTest(const FString& Parameters)
{
	UDataManager * DataManager = UDataManager::Make(FAutomationEditorCommonUtils::CreateNewMap());

	if (!IsValid(DataManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("DataManager is not valid"));
		return false;
	}
		
	const FString Title = TEXT("Title");
	const FString Description = TEXT("Description");
	UObjectiveBasicState * ObjectiveState = UObjectiveSupport::CreateBasicState();
	constexpr FObjectiveTime ObjectiveTime = FObjectiveTime();

	UObjective * Objective = DataManager->CreateObjective(Title, Description, ObjectiveState, ObjectiveTime);
	DataManager->AddObjective(Objective);
	const FObjectiveData ObjectiveCopy = DataManager->GetObjective(0);

	bool Passed = true;
	Passed &= ObjectiveCopy.Title.Equals(Title);
	Passed &= ObjectiveCopy.Description.Equals(Description);
	Passed &= ObjectiveCopy.ObjectiveState != nullptr;
	Passed &= ObjectiveCopy.ObjectiveTime.StartTime == -1;
	Passed &= ObjectiveCopy.ObjectiveTime.EndTime == -1;
	Passed &= ObjectiveCopy.SuccessObjectiveWorldID == -1;
	Passed &= ObjectiveCopy.FailureObjectiveWorldID == -1;
		
	return Passed;
}