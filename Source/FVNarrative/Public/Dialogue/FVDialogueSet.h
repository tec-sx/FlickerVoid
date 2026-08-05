// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "AttributeSet.h"
#include "FVDialogueSet.generated.h"

class USUDSScript;
class ULevelSequence;
class UFVDialogueContextualAction;

UENUM(BlueprintType)
enum class EDialogueCompareOp : uint8
{
	GreaterOrEqual,
	Greater,
	LessOrEqual,
	Less,
	Equal
};

USTRUCT(BlueprintType)
struct FDialogueChoiceMeta
{
	GENERATED_BODY()

	/** Stable id used from UI and optionally matched to SUDS choice text / event */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FName ChoiceId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayText;

	/** If set, this choice is shown grayed when requirement fails */
	UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bHasRequirement = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bHasRequirement"))
	FGameplayAttribute RequiredAttribute;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bHasRequirement"))
	EDialogueCompareOp CompareOp = EDialogueCompareOp::GreaterOrEqual;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bHasRequirement"))
	float RequiredValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditCondition = "bHasRequirement"))
	FText RequirementTooltip;	// "Depravity > 20"
};

USTRUCT(BlueprintType)
struct FDialogueCameraShot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly) FName ShotName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) FTransform RelativeTransform;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float FOV = 45.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) float BlendTime = 0.6f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bLookAtSpeaker = true;
};

UCLASS(BlueprintType)
class FLICKERVOIDNARRATIVE_API UFVDialogueSet : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Script")
	TSoftObjectPtr<USUDSScript> Script;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Script")
	FName FullConversationLabel = TEXT("FullTalk");

	/** Optional locked-choice previews (grayed + tooltip). Real branches still use [if] in .sud */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Choices")
	TArray<FDialogueChoiceMeta> ChoiceMeta;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic")
	TSoftObjectPtr<ULevelSequence> EnterConversationSequence;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic")
	TArray<FDialogueCameraShot> CameraLibrary;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic")
	FName DefaultPlayerShot = TEXT("PlayerOTS");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cinematic")
	FName DefaultNPCShot = TEXT("NPCOTS");
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<FName, TSoftClassPtr<UFVDialogueContextualAction>> ContextualActions;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variables")
	TArray<FGameplayAttribute> AttributesToPush;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Variables")
	TArray<FName> VariablesToPersist;
};
