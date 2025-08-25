#include "GameManagement/Objective_System/Data_Manager/DataManager.h"
#include "GameManagement/Objective_System/Support/ObjectiveSupport.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameTimeTest, "Project_Watcher.DataManager.ASync.GameTimeTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ClientContext)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCallbackTest, "Project_Watcher.DataManager.ASync.CallbackTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ClientContext)

DEFINE_LATENT_AUTOMATION_COMMAND(FIsDone);

//BEGIN_DEFINE_SPEC(FAsyncSpec, "Project_Watcher.DataManager.ASync", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ClientContext);

//TSharedPtr<TStrongObjectPtr<UDataManager>> DataManagerRef = MakeShared<TStrongObjectPtr<UDataManager>>(nullptr);

//END_DEFINE_SPEC(FAsyncSpec);

/*void FAsyncSpec::Define()
{
	LatentBeforeEach([this](const FDoneDelegate& Done)
	{
		/*DataManagerRef.Reset();
		
		ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Core/Maps/Testing/Testing.Testing")));
		ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
		ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
		ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, Done]
		{
			(*DataManagerRef) = TStrongObjectPtr(UDataManager::Make(GEditor->GetPIEWorldContext()->World()));
			DataManagerRef.Get()->Get()->AddToRoot();
			(*DataManagerRef)->StartGameTimer();
			
			if (!Done.IsBound())
			{
				AddError(TEXT("Done Delegate was not bound for testing"));
				return true;
			}
			Done.Execute();
			return true;
		}));#1#
	});

	/*LatentIt("Should Bind to OnComplete & Receive a BroadCast", [this](const FDoneDelegate& Done)
	{
		if (!DataManagerRef.IsValid())
		{
			AddError(TEXT("DataManagerRef is Invalid"));
			return;
		}
		
		constexpr int64 Duration = 5;
		const FObjectiveTime ObjectiveTime = DataManagerRef->Get()->CreateDelayedObjectiveTime(ETimeAnchor::GameCurrent,5,Duration);
		UObjective * Objective = DataManagerRef->Get()->CreateObjective(TEXT("Base Objective"), TEXT("Simple Test Objective"),UObjectiveSupport::CreateBasicState(), ObjectiveTime);

		UObjectiveLambdaProxy * Proxy = UObjectiveLambdaProxy::Make([this, Done](const FObjectiveData& Data)
		{
			UE_LOG(LogTemp, Display, TEXT("Objective Complete: %s"), *Data.Title);
			
			if (!Done.IsBound())
			{
				AddError(TEXT("Done Delegate was not bound for testing"));
				return;
			}
			Done.Execute();
		});
		
		/*TLambdaProxy<const FObjectiveData&> * Proxy = TLambdaProxy<const FObjectiveData&>::Make(GEditor->GetPIEWorldContext()->World(), [this, Done] (const FObjectiveData& Data)
		{
			UE_LOG(LogTemp, Display, TEXT("Objective Complete: %s"), *Data.Title);
			
			if (!Done.IsBound())
			{
				AddError(TEXT("Done Delegate was not bound for testing"));
				return;
			}
			
			Done.Execute();
		});#2#
		
		Proxy->AddToRoot();
		//using FProxyRef = TSharedPtr<TStrongObjectPtr<TLambdaProxy<const FObjectiveData&>>>;
		//const FProxyRef SharedStrongProxyRef = MakeShared<TStrongObjectPtr<TLambdaProxy<const FObjectiveData&>>>(TStrongObjectPtr(Proxy));
		
		DataManagerRef->Get()->ObjectiveCompleteDelegate.AddDynamic(Proxy, &UObjectiveLambdaProxy::Execute);
		DataManagerRef->Get()->AddObjective(Objective);
	});#1#
	
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
	 #1#
	
	LatentAfterEach([this](const FDoneDelegate& Done)
	{
		
	});
}*/

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

bool FCallbackTest::RunTest(const FString& Parameters)
{
	TSharedPtr<TStrongObjectPtr<UDataManager>> DataManager = MakeShared<TStrongObjectPtr<UDataManager>>(nullptr);
	
	constexpr int64 ObjectiveDuration = 10;
	constexpr int64 ObjectiveDelay = 1;
	TSharedPtr<int32> WorldID = MakeShared<int32>(-1);
	constexpr int64 ObjectiveUpdateDelay = 3;//This is how long until we update the objective state

	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(TEXT("/Game/Core/Maps/Testing/Testing.Testing")));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(true));
	ADD_LATENT_AUTOMATION_COMMAND(FWaitLatentCommand(1.0f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([this, DataManager, ObjectiveDuration, ObjectiveDelay, WorldID]
	{
		UWorld * World = GEditor->GetPIEWorldContext()->World();
		(*DataManager) = TStrongObjectPtr(UDataManager::Make(World));
		(*DataManager)->StartGameTimer();
		
		const FObjectiveTime ObjectiveTime = DataManager->Get()->CreateDelayedObjectiveTime(ETimeAnchor::GameCurrent, ObjectiveDelay, ObjectiveDuration);
		UObjective * Objective = DataManager->Get()->CreateObjective(TEXT("Base Objective"), TEXT("Simple Test Objective"),UObjectiveSupport::CreateBasicState(), ObjectiveTime);
		*WorldID = Objective->WorldID;
		
		DataManager->Get()->ObjectiveCompleteDelegate_Internal.AddLambda([this](const FObjectiveData& Data)
		{
			UE_LOG(LogTemp, Display, TEXT("Objective Complete: %s"), *Data.Title);
			AddInfo("Objective Complete");
		});
		
		DataManager->Get()->AddObjective(Objective);
		return true;
	}));
	
	ADD_LATENT_AUTOMATION_COMMAND(FDelayedFunctionLatentCommand([this, DataManager, WorldID]()
	{
		if (!DataManager.IsValid())
		{
			AddError(TEXT("Failed to access DataManager"));
			return true;
		}

		if (!WorldID.IsValid())
		{
			AddError(TEXT("Failed to access WorldID"));
			return true;
		}

		UObjectiveBasicState * State = UObjectiveSupport::CreateBasicState();
		State->ObjectiveState = EObjectiveState::Completed;
		
		AddInfo(TEXT("Updating Objective"));
		UE_LOG(LogTemp, Display, TEXT("Updating Objective direct set"));
		DataManager->Get()->UpdateObjectiveState(*WorldID.Get(), State);
		
		return true;
	}, ObjectiveUpdateDelay));
	
	return true;
}
