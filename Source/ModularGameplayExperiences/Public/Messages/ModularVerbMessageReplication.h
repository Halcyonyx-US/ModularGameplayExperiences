// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "ModularVerbMessage.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "ModularVerbMessageReplication.generated.h"

class UObject;
struct FModularVerbMessageReplication;
struct FNetDeltaSerializeInfo;

/**
 * Represents one verb message
 */
USTRUCT(BlueprintType)
struct FModularVerbMessageReplicationEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FModularVerbMessageReplicationEntry()
	{}

	FModularVerbMessageReplicationEntry(const FModularVerbMessage& InMessage)
		: Message(InMessage)
	{
	}

	FString GetDebugString() const;

private:
	friend FModularVerbMessageReplication;

	UPROPERTY()
	FModularVerbMessage Message;
};

/** Container of verb messages to replicate */
USTRUCT(BlueprintType)
struct FModularVerbMessageReplication : public FFastArraySerializer
{
	GENERATED_BODY()

	FModularVerbMessageReplication()
	{
	}

public:
	void SetOwner(UObject* InOwner) { Owner = InOwner; }

	// Broadcasts a message from server to clients
	void AddMessage(const FModularVerbMessage& Message);

	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FModularVerbMessageReplicationEntry, FModularVerbMessageReplication>(CurrentMessages, DeltaParms, *this);
	}

private:
	void RebroadcastMessage(const FModularVerbMessage& Message);

private:
	// Replicated list of gameplay tag stacks
	UPROPERTY()
	TArray<FModularVerbMessageReplicationEntry> CurrentMessages;

	// Owner (for a route to a world)
	UPROPERTY()
	TObjectPtr<UObject> Owner = nullptr;
};

template<>
struct TStructOpsTypeTraits<FModularVerbMessageReplication> : public TStructOpsTypeTraitsBase2<FModularVerbMessageReplication>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
