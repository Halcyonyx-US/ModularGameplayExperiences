// Copyright Chronicler.


#include "Player/ModularExperiencePlayerState.h"

#include "Engine/World.h"
#include "ModularGameplayExperiencesLogs.h"
#include "ActorComponent/ModularExperienceComponent.h"
#include "ActorComponent/ModularPawnComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameMode/ModularExperienceGameMode.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularExperiencePlayerState)

AModularExperiencePlayerState::AModularExperiencePlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	  , MyPlayerConnectionType(EModularPlayerConnectionType::Player), ReplicatedViewRotation()
{
}

void AModularExperiencePlayerState::SetPawnData(const UModularPawnData* InPawnData)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(LogModularGameplayExperiences, Error,
			TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."),
			*GetNameSafe(InPawnData),
			*GetNameSafe(this),
			*GetNameSafe(PawnData));
		return;
	}

	MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, PawnData, this);
	PawnData = InPawnData;

	ForceNetUpdate();
}

FRotator AModularExperiencePlayerState::GetReplicatedViewRotation() const
{
	// Could replace this with custom replication
	return ReplicatedViewRotation;
}

/**
 * Sets the replicated view rotation, only valid on the server.
 *
 * @param NewRotation
 */
void AModularExperiencePlayerState::SetReplicatedViewRotation(const FRotator& NewRotation)
{
	if (NewRotation != ReplicatedViewRotation)
	{
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, ReplicatedViewRotation, this);
		ReplicatedViewRotation = NewRotation;
	}
}

void AModularExperiencePlayerState::OnRep_PawnData()
{
}

/**
 * Required for replication.
 *
 * @param OutLifetimeProps
 *
 * @see https://unrealcommunity.wiki/replication-vyrv8r37
 */
void AModularExperiencePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, PawnData, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MyPlayerConnectionType, SharedParams)

	SharedParams.Condition = ELifetimeCondition::COND_SkipOwner;
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, ReplicatedViewRotation, SharedParams);

	DOREPLIFETIME(ThisClass, StatTags);
}

void AModularExperiencePlayerState::OnExperienceLoaded(const UModularExperienceDefinition* CurrentExperience)
{
	if (const AModularExperienceGameMode* GameMode = GetWorld()->GetAuthGameMode<AModularExperienceGameMode>())
	{
		if (const UModularPawnData* NewPawnData = GameMode->GetPawnDataForController(GetOwningController()))
		{
			SetPawnData(NewPawnData);
		}
		else
		{
			UE_LOG(LogModularGameplayExperiences, Error, TEXT("AModularExperiencePlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!"), *GetNameSafe(this));
		}
	}
}

void AModularExperiencePlayerState::ClientInitialize(AController* Controller)
{
	Super::ClientInitialize(Controller);

	if (UModularPawnComponent* PawnComponent = UModularPawnComponent::FindModularPawnComponent(GetPawn()))
	{
		PawnComponent->CheckDefaultInitialization();
	}
}

void AModularExperiencePlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	//@TODO: Copy stats
}


void AModularExperiencePlayerState::OnDeactivated()
{
	bool bDestroyDeactivatedPlayerState;
	switch (GetPlayerConnectionType())
	{
	case EModularPlayerConnectionType::Player:
	case EModularPlayerConnectionType::InactivePlayer:
		//@TODO: Ask the experience if we should destroy disconnecting players immediately or leave them around
		// (e.g., for long running servers where they might build up if lots of players cycle through)
		bDestroyDeactivatedPlayerState = true;
		break;
	default:
		bDestroyDeactivatedPlayerState = true;
		break;
	}

	SetPlayerConnectionType(EModularPlayerConnectionType::InactivePlayer);

	if (bDestroyDeactivatedPlayerState)
	{
		Destroy();
	}
}

void AModularExperiencePlayerState::OnReactivated()
{
	if (GetPlayerConnectionType() == EModularPlayerConnectionType::InactivePlayer)
	{
		SetPlayerConnectionType(EModularPlayerConnectionType::Player);
	}
}

void AModularExperiencePlayerState::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

void AModularExperiencePlayerState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (const UWorld* World = GetWorld(); World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
	{
		const AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);
		UModularExperienceComponent* ExperienceComponent = GameState->FindComponentByClass<UModularExperienceComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnModularExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}
}

void AModularExperiencePlayerState::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void AModularExperiencePlayerState::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 AModularExperiencePlayerState::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool AModularExperiencePlayerState::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

void AModularExperiencePlayerState::SetPlayerConnectionType(EModularPlayerConnectionType NewType)
{
	MyPlayerConnectionType = NewType;
}

void AModularExperiencePlayerState::ClientBroadcastMessage_Implementation(const FModularVerbMessage Message)
{
	// This check is needed to prevent running the action when in standalone mode
	if (GetNetMode() == NM_Client)
	{
		UGameplayMessageSubsystem::Get(this).BroadcastMessage(Message.Verb, Message);
	}
}

/** @Game-Change start delay OnExperienceLoaded to wait until the pawn is set, and so is their pawn data in their Pawn Extension Component
** for pawns that don't want to use the Pawn Data from the experience component. **/
void AModularExperiencePlayerState::RegisterToExperienceLoadedToSetPawnData()
{
	if (bRegisteredToExperienceLoaded) return;
	
	const UWorld* World = GetWorld();
	if (World && World->IsGameWorld() && World->GetNetMode() != NM_Client)
	{
		const AGameStateBase* GameState = GetWorld()->GetGameState();
		check(GameState);

		UModularExperienceComponent* ExperienceComponent = GameState->FindComponentByClass<UModularExperienceComponent>();
		check(ExperienceComponent);
		ExperienceComponent->CallOrRegister_OnExperienceLoaded(FOnModularExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
	}

	bRegisteredToExperienceLoaded = true; // only set this once. In the player's case the PlayerState is persistant while it gets destroyed with the AIs that don't have a player 
}
/** @Game-Change end **/