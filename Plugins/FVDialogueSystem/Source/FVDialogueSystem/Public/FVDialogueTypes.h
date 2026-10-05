#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FVCoreNames.h"
#include "GameplayTagContainer.h"
#include "FVDialogueTypes.generated.h"

class UFVCharacterDefinition;
class USoundBase;

USTRUCT(BlueprintType)
struct FVDIALOGUESYSTEM_API FFVDialogueLine
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<UFVCharacterDefinition> Speaker;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<AActor> SpeakerActor;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FText Text;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<USoundBase> Voice;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FGameplayTag Mood;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	float Duration = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	bool bSkippable = true;

	/** Waits for Advance instead of finishing after Duration. */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	bool bWaitForInput = false;
};

USTRUCT(BlueprintType)
struct FVDIALOGUESYSTEM_API FFVDialogueChoice
{
	GENERATED_BODY()

	/** Index to pass to UFVDialogueSubsystem::Choose. */
	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	int32 Index = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FText Text;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	bool bAvailable = true;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FText LockedReason;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Dialogue"))
class FVDIALOGUESYSTEM_API UFVDialogueSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

	/** Line duration when there is no voice: max(Min, characters * PerCharacter). */
	UPROPERTY(Config, EditAnywhere, Category = "Timing", meta = (ClampMin = 0, Units = "s"))
	float MinLineDuration = 1.5f;

	UPROPERTY(Config, EditAnywhere, Category = "Timing", meta = (ClampMin = 0, Units = "s"))
	float SecondsPerCharacter = 0.06f;

	/** Extra time after a voiced line ends. */
	UPROPERTY(Config, EditAnywhere, Category = "Timing", meta = (ClampMin = 0, Units = "s"))
	float VoicePadding = 0.3f;
};
