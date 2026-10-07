#pragma once

#include "CoreMinimal.h"
#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Facts/FVFactConditions.h"
#include "FVSocialTypes.h"
#include "FVSocialConditions.generated.h"

class UFVCharacterDefinition;
class UFVFactionDefinition;
class UFVTitleDefinition;

USTRUCT(BlueprintType, meta = (DisplayName = "Faction Standing"))
struct FVSOCIALSYSTEM_API FFVCondition_Standing : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVFactionDefinition> Faction;

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVFactCompare Compare = EFVFactCompare::GreaterOrEqual;

	UPROPERTY(EditAnywhere, Category = "Condition")
	int32 Value = 0;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Faction Notoriety"))
struct FVSOCIALSYSTEM_API FFVCondition_Notoriety : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVFactionDefinition> Faction;

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVFactCompare Compare = EFVFactCompare::GreaterOrEqual;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = 0, ClampMax = 100))
	int32 Value = 50;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

/** How widely the player is talked about. Checks what an onlooker reads, so a disguise hides most of it. */
USTRUCT(BlueprintType, meta = (DisplayName = "Fame"))
struct FVSOCIALSYSTEM_API FFVCondition_Fame : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVFactCompare Compare = EFVFactCompare::GreaterOrEqual;

	UPROPERTY(EditAnywhere, Category = "Condition")
	int32 Value = 0;

	/** Ignore any disguise and test the real value. */
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bIgnoreDisguise = false;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

/** How wanted the player is overall, across every faction. */
USTRUCT(BlueprintType, meta = (DisplayName = "Global Notoriety"))
struct FVSOCIALSYSTEM_API FFVCondition_GlobalNotoriety : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVFactCompare Compare = EFVFactCompare::GreaterOrEqual;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = 0, ClampMax = 100))
	int32 Value = 50;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

/** The player holds a title. */
USTRUCT(BlueprintType, meta = (DisplayName = "Has Title"))
struct FVSOCIALSYSTEM_API FFVCondition_HasTitle : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVTitleDefinition> Title;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

/** Player's relationship with a character. Empty Character = the character of the context Target. */
USTRUCT(BlueprintType, meta = (DisplayName = "Relationship"))
struct FVSOCIALSYSTEM_API FFVCondition_Relationship : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVCharacterDefinition> Character;

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVFactCompare Compare = EFVFactCompare::GreaterOrEqual;

	UPROPERTY(EditAnywhere, Category = "Condition")
	int32 Value = 0;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

/**
 * Attitude of an observer's faction toward the other context actor. In dialogue the observer is the
 * Target (NPC); in an NPC StateTree it is the Instigator (the NPC itself).
 */
USTRUCT(BlueprintType, meta = (DisplayName = "Attitude"))
struct FVSOCIALSYSTEM_API FFVCondition_Attitude : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVContextActor Observer = EFVContextActor::Target;

	/** Overrides the observer's own faction. */
	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVFactionDefinition> Faction;

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVAttitude AtLeast = EFVAttitude::Neutral;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

/** The Instigator (or Target) passes as a member of the faction. */
USTRUCT(BlueprintType, meta = (DisplayName = "Is Disguised As"))
struct FVSOCIALSYSTEM_API FFVCondition_IsDisguised : public FFVConditionBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Condition")
	TObjectPtr<UFVFactionDefinition> Faction;

	UPROPERTY(EditAnywhere, Category = "Condition")
	EFVContextActor Subject = EFVContextActor::Instigator;

	virtual FText GetDescription() const override;

protected:
	virtual bool EvaluateImpl(const FFVConditionContext& Context) const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Modify Standing"))
struct FVSOCIALSYSTEM_API FFVEffect_ModifyStanding : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UFVFactionDefinition> Faction;

	UPROPERTY(EditAnywhere, Category = "Effect")
	int32 Delta = 0;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Modify Notoriety"))
struct FVSOCIALSYSTEM_API FFVEffect_ModifyNotoriety : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UFVFactionDefinition> Faction;

	UPROPERTY(EditAnywhere, Category = "Effect")
	int32 Delta = 0;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Modify Fame"))
struct FVSOCIALSYSTEM_API FFVEffect_ModifyFame : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect")
	int32 Delta = 0;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};

USTRUCT(BlueprintType, meta = (DisplayName = "Modify Global Notoriety"))
struct FVSOCIALSYSTEM_API FFVEffect_ModifyGlobalNotoriety : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect")
	int32 Delta = 0;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};

/** Empty Character = the character of the context Target. */
USTRUCT(BlueprintType, meta = (DisplayName = "Modify Relationship"))
struct FVSOCIALSYSTEM_API FFVEffect_ModifyRelationship : public FFVEffectBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UFVCharacterDefinition> Character;

	UPROPERTY(EditAnywhere, Category = "Effect")
	int32 Delta = 0;

	virtual void Apply(const FFVConditionContext& Context) const override;
	virtual FText GetDescription() const override;
};
