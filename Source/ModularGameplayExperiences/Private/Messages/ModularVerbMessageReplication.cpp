// Copyright Epic Games, Inc. All Rights Reserved.

#include "Messages/ModularVerbMessageReplication.h"

#include "GameFramework/GameplayMessageSubsystem.h"
#include "Messages/ModularVerbMessage.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ModularVerbMessageReplication)

//////////////////////////////////////////////////////////////////////
// FModularVerbMessageReplicationEntry

FString FModularVerbMessageReplicationEntry::GetDebugString() const
{
	return Message.ToString();
}

//////////////////////////////////////////////////////////////////////
// FModularVerbMessageReplication

void FModularVerbMessageReplication::AddMessage(const FModularVerbMessage& Message)
{
	FModularVerbMessageReplicationEntry& NewStack = CurrentMessages.Emplace_GetRef(Message);
	MarkItemDirty(NewStack);
}

void FModularVerbMessageReplication::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	// 	for (int32 Index : RemovedIndices)
	// 	{
	// 		const FGameplayTag Tag = CurrentMessages[Index].Tag;
	// 		TagToCountMap.Remove(Tag);
	// 	}
}

void FModularVerbMessageReplication::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		const FModularVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		RebroadcastMessage(Entry.Message);
	}
}

void FModularVerbMessageReplication::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		const FModularVerbMessageReplicationEntry& Entry = CurrentMessages[Index];
		RebroadcastMessage(Entry.Message);
	}
}

void FModularVerbMessageReplication::RebroadcastMessage(const FModularVerbMessage& Message)
{
	check(Owner);
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(Owner);
	MessageSystem.BroadcastMessage(Message.Verb, Message);
}