#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Data/FVDefinition.h"
#include "Engine/DeveloperSettings.h"
#include "FVCoreNames.h"
#include "FVScanDefinition.generated.h"

/** One line of scan information, shown only when its conditions pass (e.g. a Perception check or known clue). */
USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVScanEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scan")
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scan", meta = (MultiLine = true))
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scan")
	FFVConditionSet VisibleWhen;
};

/** What a scannable reveals. Display is the scan title; effects run once on the first completed scan. */
UCLASS(BlueprintType)
class FLICKERVOIDGAMEPLAY_API UFVScanDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	/** Highlight style, see Scanner settings. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scan", meta = (Categories = "Scan.Category"))
	FGameplayTag Category;

	/** Seconds of holding scan, unaffected by time dilation. 0 = reveal on focus. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scan", meta = (ClampMin = 0, Units = "s"))
	float ScanDuration = 1.f;

	/** Instigator = scanner owner, Target = scanned actor. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scan")
	FFVConditionSet ScannableWhen;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scan", meta = (TitleProperty = "Label"))
	TArray<FFVScanEntry> Entries;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scan")
	FFVEffectList OnScanned;
};

USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVScanCategory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scan", meta = (Categories = "Scan.Category"))
	FGameplayTag Category;

	/** Custom depth stencil read by the scan post process material. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scan", meta = (ClampMin = 1, ClampMax = 255))
	int32 Stencil = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scan")
	FLinearColor Color = FLinearColor::White;
};

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Scanner"))
class FLICKERVOIDGAMEPLAY_API UFVScannerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return FV::Names::SettingsCategory; }

	const FFVScanCategory* FindCategory(const FGameplayTag& Category) const;

	UPROPERTY(Config, EditAnywhere, Category = "Scanner", meta = (TitleProperty = "Category"))
	TArray<FFVScanCategory> Categories;

	/** Global time dilation while scan mode is on (1 = no slow motion). */
	UPROPERTY(Config, EditAnywhere, Category = "Scanner", meta = (ClampMin = 0.05, ClampMax = 1))
	float ScanModeTimeDilation = 1.f;
};
