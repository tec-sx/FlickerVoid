#pragma once

#include "CoreMinimal.h"
#include "Data/FVDefinition.h"
#include "Data/FVFragment.h"
#include "FVCharacterDefinition.generated.h"

/** Base for character fragments. Feature plugins add their own (social, dialogue, ...). */
USTRUCT(BlueprintType, meta = (Hidden))
struct FVCORERUNTIME_API FFVCharacterFragment : public FFVFragment
{
	GENERATED_BODY()
};

/** A character (player, named NPC or NPC archetype). Feature data is composed through fragments. */
UCLASS(BlueprintType)
class FVCORERUNTIME_API UFVCharacterDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character", meta = (ExcludeBaseStruct))
	TArray<TInstancedStruct<FFVCharacterFragment>> Fragments;

	template <typename T>
	const T* FindFragment() const { return FVFragments::Find<T>(Fragments); }
};
