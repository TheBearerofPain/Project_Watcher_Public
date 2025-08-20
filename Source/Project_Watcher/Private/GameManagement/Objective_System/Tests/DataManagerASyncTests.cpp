#include "GameManagement/Objective_System/Data_Manager/DataManager.h"
#include "GameManagement/Objective_System/Support/LambdaProxy.h"
#include "GameManagement/Objective_System/Support/ObjectiveSupport.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameTimeTest, "Project_Watcher.DataManager.ASync.GameTimeTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ClientContext)

DEFINE_LATENT_AUTOMATION_COMMAND(FIsDone);

BEGIN_DEFINE_SPEC(FAsyncSpec, "Project_Watcher.DataManager.ASync", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ClientContext);

TSharedPtr<TStrongObjectPtr<UDataManager>> DataManagerRef = MakeShared<TStrongObjectPtr<UDataManager>>(nullptr);
TSharedPtr<TStrongObjectPtr<UObject>> OuterRef = MakeShared<TStrongObjectPtr<UObject>>(nullptr);

END_DEFINE_SPEC(FAsyncSpec);

void FAsyncSpec::Define()
{
	LatentBeforeEach([this](const FDoneDelegate& Done)
	{
		DataManagerRef.Reset();
		OuterRef.Reset();
		(*OuterRef) = TStrongObjectPtr(NewObject<UObject>());
		
		ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Core/Maps/Testing/Testing.Testing")));
		ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Done]
		{
			(*DataManagerRef) = TStrongObjectPtr(UDataManager::Make(GEditor->GetPIEWorldContext()->World()));
			(*DataManagerRef)->StartGameTimer();
			
			if (!Done.IsBound())
			{
				AddError(TEXT("Done Delegate was not bound for testing"));
				return true;
			}
			Done.Execute();
			return true;
		}));
	});

	LatentIt("Should Bind to OnComplete & Receive a BroadCast", [this](const FDoneDelegate& Done)
	{
		if (!DataManagerRef.IsValid())
		{
			AddError(TEXT("DataManagerRef is Invalid"));
			return;
		}
		
		constexpr int64 Duration = 5;
		const FObjectiveTime ObjectiveTime = DataManagerRef->Get()->CreateDelayedObjectiveTime(ETimeAnchor::GameCurrent,5,Duration);
		UObjective * Objective = DataManagerRef->Get()->CreateObjective(TEXT("Base Objective"), TEXT("Simple Test Objective"),UObjectiveSupport::CreateBasicState(), ObjectiveTime);
		UObject * Obj = this->OuterRef.Get()->Get();
		
		TLambdaProxy<const FObjectiveData&> * Proxy = TLambdaProxy<const FObjectiveData&>::Make(Obj, [this, Done] (const FObjectiveData& Data)
		{
			
		});
		
		DataManagerRef->Get()->ObjectiveCompleteDelegate.AddDynamic(Proxy, &TLambdaProxy<const FObjectiveData&>::Execute);
		DataManagerRef->Get()->AddObjective(Objective);
	});
	
	/*
	 * Objective Complete Callback test
	 * 1) Requires a UWorld
	 * 2) A Fresh DataManager
	 * 3) Something that can bind to the callback
	 * 4) Something to Add an objective
	 * 5) Something to start the world clock
	 * 6) Something to stop the world clock
	 *s
	 * Of these which need to be done fresh for each test run?
	 * UWorld
	 * DataManager
	 * Start World Clock
	 * End World Clock
	 */
	
	LatentAfterEach([this](const FDoneDelegate& Done)
	{
		
	});
}

bool FGameTimeTest::RunTest(const FString& Parameters)
{
	TSharedPtr<TStrongObjectPtr<UDataManager>> DataManager = MakeShared<TStrongObjectPtr<UDataManager>>(nullptr);
	constexpr int64 Seconds = 10;

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Core/Maps/Testing/Testing.Testing")));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, DataManager]
	{
		(*DataManager) = TStrongObjectPtr<UDataManager>(UDataManager::Make(GEditor->GetPIEWorldContext()->World()));
		(*DataManager)->StartGameTimer();
		return true;
	}));
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, DataManager, Seconds]
	{
		if (!DataManager.IsValid())
		{
			AddError(TEXT("Failed to access DataManager"));
			return true;
		}
		
		(*DataManager)->StopGameTimer();
		const int64 ResultingGameTime = (*DataManager)->GetCurrentGameSeconds();
		const int64 ExpectedGameTime = Seconds * (*DataManager)->GetGameTimeMultiplier();

		if (ResultingGameTime == ExpectedGameTime)
		{
			AddInfo(FString::Printf(TEXT("Resulting Time: %lld, Expected Time: %lld"), ResultingGameTime, ExpectedGameTime));
		}
		else
		{
			AddError(FString::Printf(TEXT("Resulting Time: %lld, Expected Time: %lld"), ResultingGameTime, ExpectedGameTime));
		}
		return true;
	}, Seconds));
	
	return true;
}
