// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetworkManagerGameInstance.generated.h"

//Wrapper for BP data//

UCLASS(BlueprintType)
class USessionSearchResult : public UObject
{
	GENERATED_BODY()
private:
	FOnlineSessionSearchResult OnlineSessionSearchResult;
public:
	static USessionSearchResult * Make(const FOnlineSessionSearchResult& OnlineSessionSearchResultIn);

	FOnlineSessionSearchResult GetOnlineSessionSearchResult();
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
	
	TSharedPtr<FOnlineSessionSettings> SessionSettings;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	
	//Settings//
	
public:

	//Initialization//
	
	UNetworkManagerGameInstance() { }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	//Initialization//

	//Network Interface calls//

	/**
	 * Used to Create a new game Session
	 */
	UFUNCTION(BlueprintCallable, Category = "Network Manager")
	void CreateSession();

	/**
	 * Updates Session data
	 */
	UFUNCTION(BlueprintCallable, Category = "Network Manager")
	void UpdateSession();

	/**
	 * Starts the session
	 */
	UFUNCTION(BlueprintCallable, Category = "Network Manager")
	void StartSession() const;

	/**
	 * Ends the session
	 */
	UFUNCTION(BlueprintCallable, Category = "Network Manager")
	void EndSession() const;

	/**
	 * Destroys the session
	 */
	UFUNCTION(BlueprintCallable, Category = "Network Manager")
	void DestroySession() const;

	/**
	 * Finds Sessions
	 * @param MaxSearchResults Max amount of sessions we want to find
	 */
	UFUNCTION(BlueprintCallable, Category = "Network Manager")
	void FindSessions(const int32 MaxSearchResults);

	/**
	 * Tries to join the given session
	 * @param SessionResult The session we want to join
	 */
	UFUNCTION(BlueprintCallable, Category = "Network Manager")
	void JoinSession(USessionSearchResult * SessionResult) const;

	/**
	 * Try to server travel to the current map
	 * in the current session
	 * @return If we could server travel
	 */
	UFUNCTION(BlueprintCallable, Category = "Network Manager")
	bool TryToServerTravelToCurrentSession() const;

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
	 * @param SessionName SessionName
	 * @param Successful Operation succeeded
	 */
	void OnCreateSessionCompletionHandler(const FName SessionName, const bool Successful) const;

	/**
	 * Called by the IOnlineSessionInterface when create session completes
	 * @param SessionName SessionName that was updated
	 * @param Successful Operation succeeded
	 */
	void OnUpdateSessionCompletionHandler(const FName SessionName, const bool Successful) const;

	/**
	 * Called by the IOnlineSessionInterface when a session update completes
	 * @param SessionName SessionName that was updated
	 * @param Successful Operation succeeded
	 */
	void OnStartSessionCompletionHandler(const FName SessionName, const bool Successful) const;

	/**
	 * Called by the IOnlineSessionInterface when a session update completes
	 * @param SessionName SessionName that was ended
	 * @param Successful Operation succeeded
	 */
	void OnEndSessionCompletionHandler(const FName SessionName, const bool Successful) const;

	/**
	 * Called by the IOnlineSessionInterface when a session is destroyed
	 * @param SessionName SessionName that was destroyed
	 * @param Successful Operation succeeded
	 */
	void OnDestroySessionCompletionHandler(const FName SessionName, const bool Successful) const;

	/**
	 * Called by the IOnlineSessionInterface when it's done searching for sessions
	 * Found Session Results get stored in SessionSearch
	 * @param Successful Operation succeeded
	 */
	void OnFindSessionsCompletionHandler(const bool Successful) const;

	/**
	 * Called by the IOnlineSessionInterface when it's done trying to join a session
	 * @param SessionName Session we are trying to join
	 * @param Result State returned about whether could join the session
	 */
	void OnJoinSessionCompletionHandler(const FName SessionName, const EOnJoinSessionCompleteResult::Type Result) const;

	//Bindable functions, These get called by the IOnlineInterface//
};
