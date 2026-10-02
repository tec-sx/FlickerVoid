#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "FVFactSettings.generated.h"

/** Declares a fact with its default value, optional range and value labels. */
USTRUCT(BlueprintType)
struct FVCORERUNTIME_API FFVFactDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fact", meta = (Categories = "Fact"))
	FGameplayTag Tag;

	/** Value written when the database is initialized or reset. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fact")
	int32 DefaultValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fact", meta = (InlineEditConditionToggle))
	bool bClamp = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fact", meta = (EditCondition = "bClamp"))
	int32 Min = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fact", meta = (EditCondition = "bClamp"))
	int32 Max = 0;

	/** Optional labels for enum-like facts. Index = value. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fact")
	TArray<FName> ValueNames;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Fact", meta = (MultiLine = true))
	FString Description;
};

/** Default facts, stored in Config/DefaultFacts.ini. */
UCLASS(Config = Facts, DefaultConfig, meta = (DisplayName = "Facts"))
class FVCORERUNTIME_API UFVFactSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("FlickerVoid"); }

	const FFVFactDefinition* FindDefinition(const FGameplayTag& Tag) const;
	FName GetValueName(const FGameplayTag& Tag, int32 Value) const;

	UPROPERTY(Config, EditAnywhere, Category = "Facts", meta = (TitleProperty = "Tag"))
	TArray<FFVFactDefinition> Definitions;
};
