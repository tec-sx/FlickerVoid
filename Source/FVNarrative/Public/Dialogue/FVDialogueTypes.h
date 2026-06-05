// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"

#include "FVDialogueTypes.generated.h"

class USoundBase;

/**
 * EFVDialogueLineType
 * Type of dialogue line for UI handling
 */
UENUM(BlueprintType)
enum class EFVDialogueLineType : uint8
{
	/** Standard NPC speech */
	Speech,
	/** Internal thought/narration */
	Thought,
	/** Whisper or quiet speech */
	Whisper,
	/** Shouting or loud speech */
	Shout,
	/** Radio/phone communication */
	Communication,
	/** Environmental/ambient dialogue */
	Ambient,
	/** Bark (short contextual line) */
	Bark
};

/**
 * EFVConditionOperator
 * How to combine multiple conditions
 */
UENUM(BlueprintType)
enum class EFVConditionOperator : uint8
{
	/** All conditions must be true */
	And,
	/** Any condition must be true */
	Or
};

/**
 * EFVConditionComparison
 * Comparison type for numeric conditions
 */
UENUM(BlueprintType)
enum class EFVConditionComparison : uint8
{
	Equal,
	NotEqual,
	GreaterThan,
	GreaterThanOrEqual,
	LessThan,
	LessThanOrEqual
};

/**
 * FFVDialogueContext
 * Context data for evaluating dialogue conditions
 */
USTRUCT(BlueprintType)
struct FLICKERVOIDNARRATIVE_API FFVDialogueContext
{
	GENERATED_BODY()

	/** The speaker's ID */
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FName SpeakerId;

	/** The speaker's actor (if available) */
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TWeakObjectPtr<AActor> SpeakerActor;

	/** The listener's actor (usually the player) */
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TWeakObjectPtr<AActor> ListenerActor;

	/** Additional context tags */
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	FGameplayTagContainer ContextTags;

	/** Custom context values */
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TMap<FName, float> CustomValues;

	/** Custom context strings */
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TMap<FName, FString> CustomStrings;
};

/**
 * FFVDialogueCondition
 * Single condition that can be evaluated
 * Designed to be easily editable in DataTable/CSV
 */
USTRUCT(BlueprintType)
struct FLICKERVOIDNARRATIVE_API FFVDialogueCondition
{
	GENERATED_BODY()

	/** Unique ID for this condition (for reuse across lines) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FName ConditionId;

	/** Type of condition to check */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FName ConditionType; // "WorldState", "Relationship", "Attribute", "Quest", "Memory", "Tag"

	/** The tag, attribute name, or ID to check */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FString TargetKey;

	/** For relationship/attribute checks: the target NPC or attribute */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FString TargetId;

	/** Comparison operator for numeric values */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	EFVConditionComparison Comparison = EFVConditionComparison::GreaterThanOrEqual;

	/** Value to compare against (for numeric conditions) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float CompareValue = 0.0f;

	/** For tag conditions: should tag be present (true) or absent (false) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	bool bRequireTagPresent = true;

	/** If true, invert the condition result */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	bool bNegate = false;

	/** Priority weight for this condition when ranking responses (higher = more specific) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	float Weight = 1.0f;

	FFVDialogueCondition()
		: ConditionId(NAME_None)
		, ConditionType(NAME_None)
		, Comparison(EFVConditionComparison::GreaterThanOrEqual)
		, CompareValue(0.0f)
		, bRequireTagPresent(true)
		, bNegate(false)
		, Weight(1.0f)
	{}
};

/**
 * FFVDialogueEffect
 * Effect to apply after dialogue line is spoken
 */
USTRUCT(BlueprintType)
struct FLICKERVOIDNARRATIVE_API FFVDialogueEffect
{
	GENERATED_BODY()

	/** Type of effect: "AddTag", "RemoveTag", "ModifyRelationship", "ModifyAttribute", "StartQuest", "UnlockMemory" */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FName EffectType;

	/** Target of the effect (tag name, NPC ID, attribute name, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FString TargetKey;

	/** Secondary target (e.g., NPC ID for relationship changes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FString TargetId;

	/** Numeric value for the effect (delta for modifications) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float Value = 0.0f;

	/** Delay in seconds before applying (0 = immediate) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float Delay = 0.0f;

	FFVDialogueEffect()
		: EffectType(NAME_None)
		, Value(0.0f)
		, Delay(0.0f)
	{}
};

/**
 * FFVDialogueLine
 * DataTable row for a single dialogue line
 * This is what you'll edit in Excel/CSV
 */
USTRUCT(BlueprintType)
struct FLICKERVOIDNARRATIVE_API FFVDialogueLine : public FTableRowBase
{
	GENERATED_BODY()

	// ========================================================================
	// IDENTIFICATION
	// ========================================================================

	/** Unique line ID (used for references and localization) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName LineId;

	/** Signal/context that triggers this line (e.g., "Greeting", "Quest_Accept", "Bark_Suspicious") */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName SignalName;

	/** Speaker ID (NPC identifier or "Player" for protagonist thoughts) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName SpeakerId;

	/** Group ID for organizing related lines */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName GroupId;

	// ========================================================================
	// CONTENT
	// ========================================================================

	/** The dialogue text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	FText Text;

	/** Alternative text when condition partially met (e.g., low sanity version) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	FText AlternativeText;

	/** Type of line (affects UI presentation) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	EFVDialogueLineType LineType = EFVDialogueLineType::Speech;

	// ========================================================================
	// AUDIO
	// ========================================================================

	/** Voice audio for this line */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	TSoftObjectPtr<USoundBase> VoiceAudio;

	/** Subtitle duration override (0 = calculate from audio/text) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	float DurationOverride = 0.0f;

	// ========================================================================
	// CONDITIONS (CSV-friendly format)
	// ========================================================================

	/** Condition IDs that must be met (references FFVDialogueCondition entries) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	TArray<FName> RequiredConditionIds;

	/** How to combine conditions */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	EFVConditionOperator ConditionOperator = EFVConditionOperator::And;

	/** 
	 * Inline conditions (CSV format: "Type:Key:Comparison:Value")
	 * Examples: 
	 * - "WorldState:Chapter1:Present"
	 * - "Relationship:NPC_Barkeeper:>=:0.5"
	 * - "Attribute:Sanity:>=:0.3"
	 * - "Quest:MainQuest_01:Completed"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	TArray<FString> InlineConditions;

	/** Priority/specificity score (higher = preferred when multiple match) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	int32 Priority = 0;

	/** Cooldown before this line can be used again (in game hours, 0 = no cooldown) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	float CooldownHours = 0.0f;

	/** Maximum times this line can be used (0 = unlimited) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	int32 MaxUseCount = 0;

	// ========================================================================
	// EFFECTS
	// ========================================================================

	/**
	 * Inline effects (CSV format: "Type:Key:Value")
	 * Examples:
	 * - "AddTag:WorldState.MetBarkeeper"
	 * - "ModifyRelationship:NPC_Barkeeper:0.1"
	 * - "ModifyAttribute:Sanity:-0.05"
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<FString> InlineEffects;

	// ========================================================================
	// FLOW
	// ========================================================================

	/** Next signal to trigger after this line (for conversation flow) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	FName NextSignal;

	/** Delay before triggering next signal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	float NextSignalDelay = 0.0f;

	/** If true, this line ends the conversation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	bool bEndsConversation = false;

	// ========================================================================
	// PRESENTATION
	// ========================================================================

	/** Animation montage to play */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	TSoftObjectPtr<UAnimMontage> AnimationMontage;

	/** Camera angle/shot identifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	FName CameraAngle;

	/** Facial expression override */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	FName Expression;

	FFVDialogueLine()
		: LineId(NAME_None)
		, SignalName(NAME_None)
		, SpeakerId(NAME_None)
		, GroupId(NAME_None)
		, LineType(EFVDialogueLineType::Speech)
		, DurationOverride(0.0f)
		, ConditionOperator(EFVConditionOperator::And)
		, Priority(0)
		, CooldownHours(0.0f)
		, MaxUseCount(0)
		, NextSignalDelay(0.0f)
		, bEndsConversation(false)
	{}
};

/**
 * FFVDialogueConditionRow
 * DataTable row for reusable conditions
 */
USTRUCT(BlueprintType)
struct FLICKERVOIDNARRATIVE_API FFVDialogueConditionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
	FFVDialogueCondition Condition;
};

/**
 * FFVSpeakerInfo
 * DataTable row for speaker configuration
 */
USTRUCT(BlueprintType)
struct FLICKERVOIDNARRATIVE_API FFVSpeakerInfo : public FTableRowBase
{
	GENERATED_BODY()

	/** Display name for subtitles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
	FText DisplayName;

	/** Color for subtitle text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
	FLinearColor SubtitleColor = FLinearColor::White;

	/** Default voice pitch modifier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
	float VoicePitch = 1.0f;

	/** Character data asset for attribute lookups */
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
	//TSoftObjectPtr<class UFVCharacterDataAsset> CharacterData;

	/** Tags associated with this speaker */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Speaker")
	FGameplayTagContainer SpeakerTags;
};

/**
 * FFVDialogueResponse
 * Player response option in conversations
 */
USTRUCT(BlueprintType)
struct FLICKERVOIDNARRATIVE_API FFVDialogueResponse : public FTableRowBase
{
	GENERATED_BODY()

	/** Unique response ID */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName ResponseId;

	/** Signal that presents this response option */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FName ParentSignal;

	/** Display text for the response */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	FText ResponseText;

	/** Short preview text (for response wheel) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	FText ShortText;

	/** Response type tag (for UI styling: Aggressive, Friendly, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Content")
	FGameplayTag ResponseType;

	/** Conditions for availability (same format as dialogue lines) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	TArray<FString> InlineConditions;

	/** If true, show grayed out when unavailable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	bool bShowWhenUnavailable = false;

	/** Reason text when unavailable */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Conditions")
	FText UnavailableReason;

	/** Skill check info (optional) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillCheck")
	FName SkillCheckType; // "Persuasion", "Intimidation", "Seduction"

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SkillCheck")
	float SkillCheckDifficulty = 0.0f;

	/** Signal to trigger when this response is chosen */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	FName ResultSignal;

	/** Signal for skill check failure */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Flow")
	FName FailureSignal;

	/** Effects when chosen (same format as dialogue lines) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TArray<FString> InlineEffects;

	/** Priority for response ordering */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Presentation")
	int32 SortOrder = 0;

	FFVDialogueResponse()
		: ResponseId(NAME_None)
		, ParentSignal(NAME_None)
		, bShowWhenUnavailable(false)
		, SkillCheckDifficulty(0.0f)
		, ResultSignal(NAME_None)
		, FailureSignal(NAME_None)
		, SortOrder(0)
	{}
};