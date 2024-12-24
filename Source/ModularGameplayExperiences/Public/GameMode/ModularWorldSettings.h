// Copyright Chronicler.


#pragma once

#include "ModularExperienceDefinition.h"
#include "ModularWorldSettings.generated.h"

/**
 * Extends default world settings to include Modular Gameplay Experiences.
 *
 * Set 'Engine - General Settings/Default Classes/World Settings Class' to this class to use it.
 */
UCLASS()
class MODULARGAMEPLAYEXPERIENCES_API AModularWorldSettings : public AWorldSettings
{
	GENERATED_BODY()

public:
	explicit AModularWorldSettings(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void CheckForErrors() override;
#endif
	
public:
	/**
	 * Returns the Default GameplayExperience for the level.
	 *
	 * Can be overridden by the user-facing experience.
	 */
	FPrimaryAssetId GetDefaultGameplayExperience() const;

protected:
	/**
	 * The Gameplay Experience replaces much of the GameMode functionality with more flexible and modular systems.
	 */
	UPROPERTY(EditDefaultsOnly, Category="GameMode")
	TSoftClassPtr<UModularExperienceDefinition> DefaultGameplayExperience;

public:

#if WITH_EDITORONLY_DATA
	// Is this level part of a front-end or other standalone experience?
	// When set, the net mode will be forced to Standalone when you hit Play in the editor
	UPROPERTY(EditDefaultsOnly, Category = PIE)
	bool ForceStandaloneNetMode = false;
#endif
};
