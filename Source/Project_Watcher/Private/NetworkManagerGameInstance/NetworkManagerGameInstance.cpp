//Project Watcher 2024 & Beyond

#include "NetworkManagerGameInstance.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "AssetRegistry/AssetData.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "Interfaces/OnlineSessionDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include "Online/OnlineSessionNames.h"

DECLARE_LOG_CATEGORY_EXTERN(LogNetworkManager, Log, All);
DEFINE_LOG_CATEGORY(LogNetworkManager);

USessionSearchResult* USessionSearchResult::Make(const FOnlineSessionSearchResult& OnlineSessionSearchResultIn)
{
	USessionSearchResult * SessionSearchResults = NewObject<USessionSearchResult>();
	SessionSearchResults->OnlineSessionSearchResult = OnlineSessionSearchResultIn;
	return SessionSearchResults;
}

FOnlineSessionSearchResult USessionSearchResult::GetOnlineSessionSearchResult()
{
	return this->OnlineSessionSearchResult;
}

FSessionData USessionSearchResult::GetSessionData() const
{
	const FString SessionName = this->OnlineSessionSearchResult.Session.OwningUserName + "'s Session";
	const bool IsFull = (this->OnlineSessionSearchResult.Session.NumOpenPrivateConnections <= 0) && (this->OnlineSessionSearchResult.Session.NumOpenPublicConnections <= 0);
	int32 OpenPlayerSlots = 0;
	bool IsPrivate = false;
	
	if (!IsFull)
	{
		if (this->OnlineSessionSearchResult.Session.NumOpenPrivateConnections > 0)
		{
			OpenPlayerSlots = this->OnlineSessionSearchResult.Session.NumOpenPrivateConnections;
			IsPrivate = true;
		}
		else
		{
			OpenPlayerSlots = this->OnlineSessionSearchResult.Session.NumOpenPublicConnections;
		}
	}
	
	return FSessionData(SessionName, IsPrivate, OpenPlayerSlots, IsFull);
}

int32 UNetworkManagerGameInstance::CheckPlayerCountInput(const int32 MaxPlayersIn) const
{
	if (MaxPlayersIn >= 1 && MaxPlayersIn <= MaxPlayers)
	{
		return MaxPlayersIn;
	}
	else if (MaxPlayersIn > MaxPlayers)
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Desired Player count of: %d is greater than %d, setting to %d"), MaxPlayersIn, MaxPlayers, MaxPlayers);
		return MaxPlayers;
	}
	else if (MaxPlayersIn < 1)
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Desired Player count of: %d is less than 1, setting to 1"), MaxPlayersIn);
		return 1;
	}
	
	return 1;
}

void UNetworkManagerGameInstance::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	this->SetupCallbacks();
}

void UNetworkManagerGameInstance::Deinitialize()
{
	Super::Deinitialize();
}

void UNetworkManagerGameInstance::CreateSession(const int32 PlayerCount, const bool IsPrivate)
{
	const int32 VerifiedPlayerCount = this->CheckPlayerCountInput(PlayerCount);
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	
	if (!SessionInterface.IsValid())
	{
		this->CallOnCreateSessionFailure(TEXT("Session Interface is Invalid"));
		return;
	}

	SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->NumPublicConnections = 8;
	SessionSettings->bAllowInvites = true;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bIsDedicated = false;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bIsLANMatch = false;//Online::GetSubsystem(GetWorld())->GetSubsystemName() == "NULL";
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->Set(SETTING_MAPNAME, FString(this->MainMenuMap), EOnlineDataAdvertisementType::ViaOnlineService);

	//Few bugs to address 1) when a server travel occurs it appears that I don't join
	//need to assess load order on things
	
	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings))
	{
		this->CallOnCreateSessionFailure(TEXT("Failed to create Session"));
	}
}

void UNetworkManagerGameInstance::UpdateSession()
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface.IsValid())
	{
		CallOnUpdateSessionFailure(TEXT("Session interface isn't Valid"));
		return;
	}

	//TODO Update to new Map name proper!
	const TSharedPtr<FOnlineSessionSettings> UpdatedSessionSettings = MakeShareable(new FOnlineSessionSettings(*SessionSettings));
	UpdatedSessionSettings->Set(SETTING_MAPNAME, FString("Updated Level Name"), EOnlineDataAdvertisementType::ViaOnlineService);

	if (!SessionInterface->UpdateSession(NAME_GameSession, *UpdatedSessionSettings))
	{
		this->CallOnUpdateSessionFailure(TEXT("Failed to update session"));
	}
	else
	{
		SessionSettings = UpdatedSessionSettings;
	}
}

void UNetworkManagerGameInstance::StartSession() const
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface.IsValid())
	{
		this->CallOnStartSessionFailure(TEXT("Session Interface is Invalid"));
		return;
	}

	if (!SessionInterface->StartSession(NAME_GameSession))
	{
		this->CallOnStartSessionFailure(TEXT("Failed to start session"));
	}
}

void UNetworkManagerGameInstance::EndSession() const
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		this->CallOnEndSessionFailure(TEXT("Session interface is Invalid"));
		return;
	}

	if (!sessionInterface->EndSession(NAME_GameSession))
	{
		this->CallOnEndSessionFailure(TEXT("Failed to end session"));
	}
}

void UNetworkManagerGameInstance::DestroySession() const
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface.IsValid())
	{
		this->CallOnDestroySessionFailure(TEXT("SessionInterface is invalid"));
		return;
	}

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		this->CallOnDestroySessionFailure(TEXT("Failed to destroy session"));
	}
}

void UNetworkManagerGameInstance::FindSessions(const int32 MaxSearchResults)
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface.IsValid())
	{
		this->CallOnFindSessionsFailure(TEXT("SessionInterface is Invalid"));
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = MaxSearchResults;
	SessionSearch->bIsLanQuery = false;

	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		this->CallOnFindSessionsFailure(TEXT("Failed to find sessions"));
	}
}

void UNetworkManagerGameInstance::JoinSession(USessionSearchResult* SessionResult) const
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	if (!SessionInterface.IsValid())
	{
		this->CallOnJoinSessionFailure(TEXT("SessionInterface is invalid"));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult->GetOnlineSessionSearchResult()))
	{
		this->CallOnJoinSessionFailure(TEXT("Error joining Session"));
	}
}

bool UNetworkManagerGameInstance::TryToServerTravelToCurrentSession() const
{
	//Non seamless travel
	return GetWorld()->ServerTravel(this->MainGameMap);
}

void UNetworkManagerGameInstance::SetupCallbacks()
{
	const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld());
	check(SessionInterface.IsValid());
	
	SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionCompletionHandler));
	SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(FOnUpdateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionCompletionHandler));
	SessionInterface->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionCompletionHandler));
	SessionInterface->AddOnEndSessionCompleteDelegate_Handle(FOnEndSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnEndSessionCompletionHandler));
	SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionCompletionHandler));
	SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsCompletionHandler));
	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UNetworkManagerGameInstance::OnJoinSessionCompletionHandler));
}

void UNetworkManagerGameInstance::CallOnCreateSessionComplete(const FName SessionNameIn) const
{
	if (this->OnCreateSessionComplete.IsBound())
	{
		this->OnCreateSessionComplete.Broadcast(SessionNameIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnCreateSessionComplete"));
	}
}

void UNetworkManagerGameInstance::CallOnCreateSessionFailure(const FString& FailureIn) const
{
	if (this->OnCreateSessionFailure.IsBound())
	{
		this->OnCreateSessionFailure.Broadcast(FailureIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnCreateSessionFailure: %s"), *FailureIn);
	}
}

void UNetworkManagerGameInstance::CallOnUpdateSessionComplete(const FName SessionNameIn) const
{
	if (this->OnUpdateSessionComplete.IsBound())
	{
		this->OnUpdateSessionComplete.Broadcast(SessionNameIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnUpdateSessionComplete"));
	}
}

void UNetworkManagerGameInstance::CallOnUpdateSessionFailure(const FString& FailureIn) const
{
	if (this->OnUpdateSessionFailure.IsBound())
	{
		this->OnUpdateSessionFailure.Broadcast(FailureIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnUpdateSessionFailure: %s"), *FailureIn);
	}
}

void UNetworkManagerGameInstance::CallOnStartSessionComplete(const FName SessionNameIn) const
{
	if (this->OnStartSessionComplete.IsBound())
	{
		this->OnStartSessionComplete.Broadcast(SessionNameIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnStartSessionComplete"));
	}
}

void UNetworkManagerGameInstance::CallOnStartSessionFailure(const FString& FailureIn) const
{
	if (this->OnStartSessionFailure.IsBound())
	{
		this->OnStartSessionFailure.Broadcast(FailureIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnStartSessionFailure: %s"), *FailureIn);
	}
}

void UNetworkManagerGameInstance::CallOnEndSessionComplete(const FName SessionNameIn) const
{
	if (this->OnEndSessionComplete.IsBound())
	{
		this->OnEndSessionComplete.Broadcast(SessionNameIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnEndSessionComplete"));
	}
}

void UNetworkManagerGameInstance::CallOnEndSessionFailure(const FString& FailureIn) const
{
	if (this->OnEndSessionFailure.IsBound())
	{
		this->OnEndSessionFailure.Broadcast(FailureIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnEndSessionFailure: %s"), *FailureIn);
	}
}

void UNetworkManagerGameInstance::CallOnDestroySessionComplete(const FName SessionNameIn) const
{
	if (this->OnDestroySessionComplete.IsBound())
	{
		this->OnDestroySessionComplete.Broadcast(SessionNameIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnDestroySessionComplete"));
	}
}

void UNetworkManagerGameInstance::CallOnDestroySessionFailure(const FString& FailureIn) const
{
	if (this->OnDestroySessionFailure.IsBound())
	{
		this->OnDestroySessionFailure.Broadcast(FailureIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnDestroySessionFailure: %s"), *FailureIn);
	}
}

void UNetworkManagerGameInstance::CallOnFindSessionsComplete(const TArray<USessionSearchResult*>& SessionResultsIn) const
{
	if (this->OnFindSessionsComplete.IsBound())
	{
		this->OnFindSessionsComplete.Broadcast(SessionResultsIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnFindSessionsComplete"));
	}
}

void UNetworkManagerGameInstance::CallOnFindSessionsFailure(const FString& FailureIn) const
{
	if (this->OnFindSessionsFailure.IsBound())
	{
		this->OnFindSessionsFailure.Broadcast(FailureIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnFindSessionsFailure: %s"), *FailureIn);
	}
}

void UNetworkManagerGameInstance::CallOnJoinSessionComplete(const FName SessionIn) const
{
	if (this->OnJoinSessionComplete.IsBound())
	{
		this->OnJoinSessionComplete.Broadcast(SessionIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnJoinSessionComplete"));
	}
}

void UNetworkManagerGameInstance::CallOnJoinSessionFailure(const FString& FailureIn) const
{
	if (this->OnJoinSessionFailure.IsBound())
	{
		this->OnJoinSessionFailure.Broadcast(FailureIn);
	}
	else
	{
		UE_LOG(LogNetworkManager, Warning, TEXT("Nothing bound to OnJoinSessionFailure: %s"), *FailureIn);
	}
}

void UNetworkManagerGameInstance::OnCreateSessionCompletionHandler(const FName SessionName, const bool Successful) const
{
	if (Successful)
	{
		this->CallOnCreateSessionComplete(SessionName);
	}
	else
	{
		this->CallOnCreateSessionFailure(TEXT("Failed to create Session"));
	}
}

void UNetworkManagerGameInstance::OnUpdateSessionCompletionHandler(const FName SessionName, const bool Successful) const
{
	if (Successful)
	{
		this->CallOnUpdateSessionComplete(SessionName);
	}
	else
	{
		this->CallOnCreateSessionFailure(TEXT("Failed to update Session"));
	}
}

void UNetworkManagerGameInstance::OnStartSessionCompletionHandler(const FName SessionName, const bool Successful) const
{
	if (Successful)
	{
		this->CallOnStartSessionComplete(SessionName);
	}
	else
	{
		this->CallOnStartSessionFailure(TEXT("Failed to Start Session"));
	}
}

void UNetworkManagerGameInstance::OnEndSessionCompletionHandler(const FName SessionName, const bool Successful) const
{
	if (Successful)
	{
		this->CallOnEndSessionComplete(SessionName);
	}
	else
	{
		this->CallOnEndSessionFailure(TEXT("Failed to end session"));
	}
}

void UNetworkManagerGameInstance::OnDestroySessionCompletionHandler(const FName SessionName, const bool Successful) const
{
	if (Successful)
	{
		this->CallOnDestroySessionComplete(SessionName);
	}
	else
	{
		this->CallOnDestroySessionFailure(TEXT("Failed to destroy session"));
	}
}

void UNetworkManagerGameInstance::OnFindSessionsCompletionHandler(const bool Successful) const
{
	if (Successful)
	{		
		TArray<USessionSearchResult*> FoundSessions;
		
		for (FOnlineSessionSearchResult SearchResult : SessionSearch->SearchResults)
		{
			FoundSessions.Add(USessionSearchResult::Make(SearchResult));
		}

		if (!FoundSessions.IsEmpty())
		{
			this->CallOnFindSessionsComplete(FoundSessions);
		}
		else
		{
			this->CallOnFindSessionsFailure(TEXT("No Sessions Found"));
		}
	}
	else
	{
		this->CallOnFindSessionsFailure(TEXT("Failed to Find Sessions"));
	}
}

void UNetworkManagerGameInstance::OnJoinSessionCompletionHandler(const FName SessionName, const EOnJoinSessionCompleteResult::Type Result) const
{
	switch(Result)
	{
	case EOnJoinSessionCompleteResult::Type::Success:
		this->CallOnJoinSessionComplete(SessionName);
		break;
	case EOnJoinSessionCompleteResult::Type::SessionIsFull:
		this->CallOnJoinSessionFailure(TEXT("Session is full"));
		break;
	case EOnJoinSessionCompleteResult::Type::UnknownError:
		this->CallOnJoinSessionFailure(TEXT("Unknown error when joining session"));
		break;
	case EOnJoinSessionCompleteResult::Type::AlreadyInSession:
		this->CallOnJoinSessionFailure(TEXT("Already in session"));
		break;
	case EOnJoinSessionCompleteResult::Type::CouldNotRetrieveAddress:
		this->CallOnJoinSessionFailure(TEXT("Couldn't retrieve address"));
		break;
	case EOnJoinSessionCompleteResult::Type::SessionDoesNotExist:
		this->CallOnJoinSessionFailure(TEXT("Session doesn't exist"));
		break;
	}
}
