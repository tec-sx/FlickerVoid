#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Nodes/FlowNode.h"
#include "FVCallOutFlowNodes.generated.h"

class UAudioComponent;
class USoundBase;

/** One bark line. Filtered by speaker/player tags and a FlickerVoid condition set. */
USTRUCT(BlueprintType)
struct FVSTORYSYSTEM_API FFVCallOutTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CallOut") FText Text;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CallOut") TSoftObjectPtr<USoundBase> Voice;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CallOut") float DisplayDuration = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CallOut") bool bShowTalkIcon = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter") FGameplayTag IdentityTag;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter") FGameplayTagContainer RequiredTags;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter") FGameplayTagContainer BlockingTags;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Filter") FFVConditionSet Conditions;
};

/** Broadcast on the Dialogue.CallOut gameplay message channel. */
USTRUCT(BlueprintType)
struct FVSTORYSYSTEM_API FFVDialogueCallOutMessage
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CallOut") FText Text;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CallOut") FName SpeakerID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CallOut") TWeakObjectPtr<AActor> OwnerActor;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CallOut") TSoftObjectPtr<USoundBase> Voice;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CallOut") float DisplayDuration = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "CallOut") bool bShowTalkIcon = true;
};

/** Flow node: picks a valid, non-repeating bark for the flow owner, broadcasts it and optionally waits for it. */
UCLASS(NotBlueprintable, meta = (DisplayName = "Play CallOut"))
class FVSTORYSYSTEM_API UFVFlowNode_CallOut : public UFlowNode
{
	GENERATED_BODY()

public:
	UFVFlowNode_CallOut();

	UPROPERTY(EditAnywhere, Category = "CallOut", meta = (RequiredAssetDataTags = "RowStructure=/Script/FVStorySystem.FVCallOutTableRow"))
	TObjectPtr<UDataTable> CallOutDatabase;

	UPROPERTY(EditAnywhere, Category = "CallOut")
	bool bWaitForDuration = true;

	virtual void ExecuteInput(const FName& PinName) override;
	virtual void Cleanup() override;

#if WITH_EDITOR
	virtual FString GetNodeDescription() const override;
	virtual EDataValidationResult ValidateNode() override;
#endif

private:
	bool PickRow(AActor* Owner, AActor* Player, FName& OutRowName, const FFVCallOutTableRow*& OutRow) const;
	bool IsRowValid(const FFVCallOutTableRow& Row, const FGameplayTagContainer& CombinedTags, const FGameplayTagContainer& OwnerTags) const;
	float PlayVoice(const FFVCallOutTableRow& Row, AActor* Owner);
	void Broadcast(const FFVCallOutTableRow& Row, AActor* Owner) const;
	AActor* ResolvePlayerActor() const;
	static void GatherTags(const AActor* Actor, FGameplayTagContainer& OutTags);

	UFUNCTION()
	void OnCallOutFinished();

	FTimerHandle CallOutTimerHandle;
	TWeakObjectPtr<UAudioComponent> PlayingVoice;

	static TMap<TWeakObjectPtr<AActor>, FName> LastPlayedRowByOwner;
};
