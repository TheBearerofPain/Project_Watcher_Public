//Project Watcher 2024 & Beyond

#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetworkManagerGameInstance.generated.h"

//Wrapper for BP data//

USTRUCT(Blueprintable)
struct FSessionData
{
	GENERATED_USTRUCT_BODY()
public:
	UPROPERTY(BlueprintReadWrite, Blueprintable, Category = "Online")
	FString SessionName = "";
	UPROPERTY(BlueprintReadWrite, Blueprintable, Category = "Online")
	bool IsPrivate = false;
	UPROPERTY(BlueprintReadWrite, Blueprintable, Category = "Online")
	int32 OpenPlayerSlots = -1;
	UPROPERTY(BlueprintReadWrite, Blueprintable, Category = "Online")
	bool IsFull = false;

	FSessionData(){}
	
	FSessionData(const FString& SessionNameIn, const bool IsPrivateIn, const int32 OpenPlayerSlotsIn, const bool IsFullIn)
	{
		SessionName = SessionNameIn;
		IsPrivate = IsPrivateIn;
		OpenPlayerSlots = OpenPlayerSlotsIn;
		IsFull = IsFullIn;
	}
};

UCLASS(BlueprintType)
class USessionSearchResult : public UObject
{
	GENERATED_BODY()
private:
	FOnlineSessionSearchResult OnlineSessionSearchResult;
public:
	/**
	 * Static Make function for constructing this UObject
	 * @param OnlineSessionSearchResultIn The OnlineSessionSearchResult Data used to init this object
	 * @return USessionSearchResult UObject
	 */
	static USessionSearchResult * Make(const FOnlineSessionSearchResult& OnlineSessionSearchResultIn);

	/**
	 * Used for getting the nested OnlineSessionSearchResult
	 * @return FOnlineSessionSearchResult
	 */
	FOnlineSessionSearchResult GetOnlineSessionSearchResult();

	/**
	 * Gets the SessionData in a USTRUCT for UI Usage
	 * @return The parsed USTRUCT session data
	 */
	UFUNCTION(BlueprintCallable, Category = "Online")
	FSessionData GetSessionData() const;
};

//Wrapper for BP data//

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnCreateSessionComplete, const FName, SessionName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnCreateSessionFailure, const FString&, Failure);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnUpdateSessionComplete, const FName, SessionName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnUpdateSessionFailure, const FString&, Failure);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnStartSessionComplete, const FName, SessionName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnStartSessionFailure, const FString&, Failure);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnEndSessionComplete, const FName, SessionName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnEndSessionFailure, const FString&, Failure);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnDestroySessionComplete, const FName, SessionName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnDestroySessionFailure, const FString&, Failure);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnFindSessionsComplete, const TArray<USessionSearchResult*>&, SessionResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnFindSessionsFailure, const FString&, Failure);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnJoinSessionComplete, const FName, Session);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNetworkManager_OnJoinSessionFailure, const FString&, Failure);

/**
 * Game subsystem that handles requests for hosting and joining online games.
 * One subsystem is created for each game instance and can be accessed from blueprints or C++ code.
 */
UCLASS(BlueprintType)
class UNetworkManagerGameInstance : public UGameInstanceSubsystem
{
	GENERATED_BODY()
private:
	//Settings//

	/* SessionSettings established by host */
	TSharedPtr<FOnlineSessionSettings> SessionSettings;

	/* SessionSearch Results by clients */
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	/* The name of the session we are a part of, gets set on JoinSession (Client) or CreateSession (Host) */
	FName SessionName = TEXT("Default Game Session");

	/* Access / Mutators for ease of code checking */

	/* Updates the SessionName */
	void SetSessionName(const FName NewSessionName);

	/* Retrieves the SessionName */
	FName GetSessionName() const;

	/* Access / Mutators for ease of code checking */
	
	/* The data of the session we are a part of from the perspective of the client */
	FOnlineSessionSearchResult SessionData;
	
	//Settings//

private:

	/* max players we allow in a session (advisable PeerToPeer limit) */
	const int32 MaxPlayers = 8;
	/* Main Menu Level Name */
	const FString MainMenuMap = TEXT("/Game/Core/Maps/MainMenu_Map");
	/* Main Game Level Name */
	const FString MainGameMap = TEXT("/Game/Core/Maps/MainGame_Map");

	/* Main Game Level path used for hosting */
	FString BuildMainGameMapPathForHosting() const;

	/* Main Game Level path used for joining */
	FString BuildMainGameMapPathForJoining() const;
	
private:
	/**
	 * Checker function used to make sure MaxPlayersIn = [1,8]
	 * @param MaxPlayersIn MaxPlayers input by player
	 * @return the corrected / verified MaxPlayersIn value
	 */
	int32 CheckPlayerCountInput(const int32 MaxPlayersIn) const;
	
public:

	//Initialization//
	
	UNetworkManagerGameInstance() { }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	//Initialization//

	//Network Interface calls//

	/**
	 * Used to Create a new game Session
	 * @param PlayerCount The desired PlayerCount for this session
	 * @param IsPrivate If the session is private or publicly join-able
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Network Manager")
	void CreateSession(const int32 PlayerCount, const bool IsPrivate);

	/**
	 * Updates Session data
	 */
	//UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Network Manager")
	/* This function will remain hidden until such a need arises, for now it's function is undetermined */
	void UpdateSession();
	
	/**
	 * Starts the session
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Network Manager")
	void StartSession() const;

	/**
	 * Ends the session, Graceful shutdown notifies players in advance
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Network Manager")
	void EndSession() const;

	/**
	 * Destroys the session, hard shutdown
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Network Manager")
	void DestroySession() const;

	/**
	 * Finds Sessions
	 * @param MaxSearchResults Max amount of sessions we want to find
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Network Manager")
	void FindSessions(const int32 MaxSearchResults);

	/**
	 * Tries to join the given session
	 * @param SessionResult The session we want to join
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Network Manager")
	void JoinSession(USessionSearchResult * SessionResult);

	/**
	 * Try to server travel to the current map in the current session as a host
	 * @return If we could server travel
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Network Manager")
	bool ServerTravelAsHost_GameMap() const;
	
	/**
	 * Try to server travel to the current map in the current session as a client
	 * @return If we could server travel
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure=false, Category = "Network Manager")
	bool ServerTravelAsClient_GameMap() const;

	//Network Interface calls//

	//Network Interface Delegates//

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnCreateSessionComplete OnCreateSessionComplete;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnCreateSessionFailure OnCreateSessionFailure;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnUpdateSessionComplete OnUpdateSessionComplete;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnUpdateSessionFailure OnUpdateSessionFailure;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnStartSessionComplete OnStartSessionComplete;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnStartSessionFailure OnStartSessionFailure;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnEndSessionComplete OnEndSessionComplete;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnEndSessionFailure OnEndSessionFailure;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnDestroySessionComplete OnDestroySessionComplete;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnDestroySessionFailure OnDestroySessionFailure;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnFindSessionsComplete OnFindSessionsComplete;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnFindSessionsFailure OnFindSessionsFailure;

	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnJoinSessionComplete OnJoinSessionComplete;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category = "Network Manager")
	FNetworkManager_OnJoinSessionFailure OnJoinSessionFailure;

	//Network Interface Delegates//

private:

	//Setup Code//

	void SetupCallbacks();
	
	//Setup Code//

	//Network Interface Delegate callers//
	
	void CallOnCreateSessionComplete(const FName SessionNameIn) const;
	void CallOnCreateSessionFailure(const FString& FailureIn) const;

	void CallOnUpdateSessionComplete(const FName SessionNameIn) const;
	void CallOnUpdateSessionFailure(const FString& FailureIn) const;

	void CallOnStartSessionComplete(const FName SessionNameIn) const;
	void CallOnStartSessionFailure(const FString& FailureIn) const;

	void CallOnEndSessionComplete(const FName SessionNameIn) const;
	void CallOnEndSessionFailure(const FString& FailureIn) const;

	void CallOnDestroySessionComplete(const FName SessionNameIn) const;
	void CallOnDestroySessionFailure(const FString& FailureIn) const;

	void CallOnFindSessionsComplete(const TArray<USessionSearchResult*>& SessionResultsIn) const;
	void CallOnFindSessionsFailure(const FString& FailureIn) const;

	void CallOnJoinSessionComplete(const FName SessionIn) const;
	void CallOnJoinSessionFailure(const FString& FailureIn) const;

	//Network Interface Delegate callers//

	//Bindable functions, These get called by the IOnlineInterface//

	/**
	 * Called by IOnlineSessionInterface when create session completes
	 * @param SessionNameIn SessionName
	 * @param Successful Operation succeeded
	 */
	void OnCreateSessionCompletionHandler(const FName SessionNameIn, const bool Successful) const;

	/**
	 * Called by the IOnlineSessionInterface when create session completes
	 * @param SessionNameIn SessionName that was updated
	 * @param Successful Operation succeeded
	 */
	void OnUpdateSessionCompletionHandler(const FName SessionNameIn, const bool Successful) const;

	/**
	 * Called by the IOnlineSessionInterface when a session update completes
	 * @param SessionNameIn SessionName that was updated
	 * @param Successful Operation succeeded
	 */
	void OnStartSessionCompletionHandler(const FName SessionNameIn, const bool Successful) const;

	/**
	 * Called by the IOnlineSessionInterface when a session update completes
	 * @param SessionNameIn SessionName that was ended
	 * @param Successful Operation succeeded
	 */
	void OnEndSessionCompletionHandler(const FName SessionNameIn, const bool Successful) const;

	/**
	 * Called by the IOnlineSessionInterface when a session is destroyed
	 * @param SessionNameIn SessionName that was destroyed
	 * @param Successful Operation succeeded
	 */
	void OnDestroySessionCompletionHandler(const FName SessionNameIn, const bool Successful) const;

	/**
	 * Called by the IOnlineSessionInterface when it's done searching for sessions
	 * Found Session Results get stored in SessionSearch
	 * @param Successful Operation succeeded
	 */
	void OnFindSessionsCompletionHandler(const bool Successful) const;

	/**
	 * Called by the IOnlineSessionInterface when it's done trying to join a session
	 * @param SessionNameIn Session we are trying to join
	 * @param Result State returned about whether you could join the session
	 */
	void OnJoinSessionCompletionHandler(const FName SessionNameIn, const EOnJoinSessionCompleteResult::Type Result) const;

	//Bindable functions, These get called by the IOnlineInterface//
};
