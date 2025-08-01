#include "GameManagement/Objective_System/Data_Manager/DataManager.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"
#include "Tests/AutomationEditorCommon.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGameTimeTest, "Project_Watcher.DataManager.ASync.GameTimeTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter | EAutomationTestFlags::ClientContext)

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
