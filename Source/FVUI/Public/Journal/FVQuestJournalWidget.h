#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Quest/FVQuest.h"
#include "FVQuestJournalWidget.generated.h"

/** One objective row as the journal should show it. */
USTRUCT(BlueprintType)
struct FLICKERVOIDUI_API FFVJournalObjective
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Journal") FText Description;
	UPROPERTY(BlueprintReadOnly, Category = "Journal") bool bComplete = false;
	UPROPERTY(BlueprintReadOnly, Category = "Journal") bool bOptional = false;
};

/** Base for the quest journal. Reads UFVQuestSubsystem and refreshes on quest changes. */
UCLASS(Abstract, Blueprintable)
class FLICKERVOIDUI_API UFVQuestJournalWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Journal")
	TArray<UFVQuestDefinition*> GetQuests(EFVQuestState State) const;

	UFUNCTION(BlueprintPure, Category = "Journal")
	TArray<FFVJournalObjective> GetVisibleObjectives(const UFVQuestDefinition* Quest) const;

	UFUNCTION(BlueprintPure, Category = "Journal")
	EFVQuestState GetQuestState(const UFVQuestDefinition* Quest) const;

	UFUNCTION(BlueprintCallable, Category = "Journal")
	void SelectQuest(UFVQuestDefinition* Quest);

	UFUNCTION(BlueprintPure, Category = "Journal")
	UFVQuestDefinition* GetSelectedQuest() const { return SelectedQuest; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** Rebuild lists here. Called on construct, on quest changes and on selection change. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Journal")
	void OnJournalChanged();

private:
	UFUNCTION()
	void HandleQuestsChanged();

	UFVQuestSubsystem* GetQuestSubsystem() const;

	UPROPERTY(Transient)
	TObjectPtr<UFVQuestDefinition> SelectedQuest;
};