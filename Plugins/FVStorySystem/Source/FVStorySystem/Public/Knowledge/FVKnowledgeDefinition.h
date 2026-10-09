#pragma once

#include "CoreMinimal.h"
#include "Data/FVDefinition.h"
#include "Conditions/FVEffect.h"
#include "FVKnowledgeDefinition.generated.h"

UENUM(BlueprintType)
enum class EFVKnowledgeKind : uint8
{
	Clue,
	Topic,
	Thought
};

/** A piece of knowledge. Id is the fact tag written when learned (e.g. Knowledge.Clue.BloodOnCoat). */
UCLASS(BlueprintType)
class FVSTORYSYSTEM_API UFVKnowledgeDefinition : public UFVDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knowledge")
	EFVKnowledgeKind Kind = EFVKnowledgeKind::Clue;

	/** Knowledge that must be known before this can be learned. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knowledge")
	TArray<TObjectPtr<UFVKnowledgeDefinition>> Prerequisites;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Knowledge")
	FFVEffectList OnLearned;

protected:
	virtual bool RequiresId() const override { return true; }
};
