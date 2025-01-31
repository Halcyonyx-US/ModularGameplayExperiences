// Copyright Chronicler.


#pragma once

#include "GameplayTagStack.h"
#include "ModularPlayerState.h"
#include "DataAsset/ModularPawnData.h"
#include "GameMode/ModularExperienceDefinition.h"
#include "Messages/ModularVerbMessage.h"

#include "ModularExperiencePlayerState.generated.h"

/** Defines the types of client connected */
UENUM()
enum class EModularPlayerConnectionType : uint8
{
	// An active player.
	Player = 0,

	// Spectator connected to a running game.
	LiveSpectator,

	// Spectating a demo recording offline.
	ReplaySpectator,

	// A deactivated player (disconnected).
	InactivePlayer
};

/**
 * Player state that supports custom Pawn data.
 */
UCLASS(Config="Game", Blueprintable)
class MODULARGAMEPLAYEXPERIENCES_API AModularExperiencePlayerState : public AModularPlayerState
{
	GENERATED_BODY()

public:
	explicit AModularExperiencePlayerState(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	template <class T>
	const T* GetPawnData() const { return Cast<T>(PawnData); }

	virtual void SetPawnData(const UModularPawnData* InPawnData);

	// Gets the replicated view rotation of this player, used for spectating.
	FRotator GetReplicatedViewRotation() const;

	// Sets the replicated view rotation, only valid on the server.
	void SetReplicatedViewRotation(const FRotator& NewRotation);

	/**
	 * @ingroup APlayerState
	 * @{
	 */
	virtual void ClientInitialize(AController* Controller) override;
	//virtual void Reset() override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	virtual void OnDeactivated() override;
	virtual void OnReactivated() override;
	/**
	 * @}
	 */

	/**
	 * @ingroup AActor
	 * @{
	 */
	virtual void PreInitializeComponents() override;
	virtual void PostInitializeComponents() override;
	/**
	 * @}
	 */

	void SetPlayerConnectionType(EModularPlayerConnectionType NewType);
	EModularPlayerConnectionType GetPlayerConnectionType() const { return MyPlayerConnectionType; }
	
	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1).
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "ModularAbility|Tags")
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1).
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category = "ModularAbility|Tags")
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present).
	UFUNCTION(BlueprintCallable, Category = "ModularAbility|Tags")
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag.
	UFUNCTION(BlueprintCallable, Category = "ModularAbility|Tags")
	bool HasStatTag(FGameplayTag Tag) const;
	
	// Send a message to just this player
	// (use only for client notifications like accolades, quest toasts, etc... that can handle being occasionally lost)
	UFUNCTION(Client, Unreliable, BlueprintCallable, Category = "PlayerState")
	void ClientBroadcastMessage(const FModularVerbMessage Message);

	// @Game-Change function to listen to Experience ready for using default pawn data in the loaded Experiment
	void RegisterToExperienceLoadedToSetPawnData();
	
protected:
	virtual void OnExperienceLoaded(const UModularExperienceDefinition* CurrentExperience);

	UFUNCTION()
	void OnRep_PawnData();

	UPROPERTY(ReplicatedUsing=OnRep_PawnData)
	TObjectPtr<const UModularPawnData> PawnData;

	// @Game-Change keep track to make sure the PawnData isn't set more than once while the playerState exists
	// needed since we're no longer calling that logic in PostInitializeComponents(); which only happened once per playerState
	bool bRegisteredToExperienceLoaded;
	
private:
	UPROPERTY(Replicated)
	EModularPlayerConnectionType MyPlayerConnectionType;

	UPROPERTY(Replicated)
	FGameplayTagStackContainer StatTags;

	UPROPERTY(Replicated)
	FRotator ReplicatedViewRotation;
};
