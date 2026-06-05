// Copyright FlickerVoid. All Rights Reserved.

#include "Character/FVCharacterArchetypeData.h"
#include "Movement/FVMovementTags.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UFVCharacterArchetypeData::UFVCharacterArchetypeData()
	: MovementSpeedModifier(1.0f)
	, JumpHeightModifier(1.0f)
	, AccelerationModifier(1.0f)
	, BaseHealth(-1.0f)
	, BaseStamina(-1.0f)
	, bCanSprint(true)
	, bCanCrouch(true)
	, bCanJump(true)
	, bCanTraverse(true)
{
}

FPrimaryAssetId UFVCharacterArchetypeData::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(TEXT("CharacterArchetype"), GetFName());
}

FGameplayTagContainer UFVCharacterArchetypeData::GetAllInitialTags() const
{
	FGameplayTagContainer AllTags = PermanentTags;
	AllTags.AppendTags(InitialTags);
	AllTags.AppendTags(AnimationStyleTags);
	return AllTags;
}

bool UFVCharacterArchetypeData::IsTagBlocked(const FGameplayTag& Tag) const
{
	return BlockedTags.HasTag(Tag);
}

FGameplayTagContainer UFVCharacterArchetypeData::GetAbilityRestrictionTags() const
{
	FGameplayTagContainer Restrictions;
	const FFVMovementTags& MovementTags = FFVMovementTags::Get();

	if (!bCanSprint)
	{
		Restrictions.AddTag(MovementTags.Movement_Restriction_NoSprint);
	}

	if (!bCanCrouch)
	{
		Restrictions.AddTag(MovementTags.Movement_Restriction_NoCrouch);
	}

	if (!bCanJump)
	{
		Restrictions.AddTag(MovementTags.Movement_Restriction_NoJump);
	}

	if (!bCanTraverse)
	{
		Restrictions.AddTag(MovementTags.Movement_Restriction_NoTraversal);
	}

	return Restrictions;
}

#if WITH_EDITOR
EDataValidationResult UFVCharacterArchetypeData::IsDataValid(TArray<FText>& ValidationErrors)
{
	EDataValidationResult Result = Super::IsDataValid(ValidationErrors);

	if (ArchetypeId.IsNone())
	{
		ValidationErrors.Add(FText::FromString(TEXT("ArchetypeId must be set")));
		Result = EDataValidationResult::Invalid;
	}

	if (DisplayName.IsEmpty())
	{
		ValidationErrors.Add(FText::FromString(TEXT("DisplayName must be set")));
		Result = EDataValidationResult::Invalid;
	}

	if (MovementSpeedModifier <= 0.0f)
	{
		ValidationErrors.Add(FText::FromString(TEXT("MovementSpeedModifier must be greater than 0")));
		Result = EDataValidationResult::Invalid;
	}

	// Check for conflicting tags
	FGameplayTagContainer Conflicts = PermanentTags.Filter(BlockedTags);
	if (Conflicts.Num() > 0)
	{
		ValidationErrors.Add(FText::FromString(FString::Printf(
			TEXT("PermanentTags and BlockedTags have conflicts: %s"), 
			*Conflicts.ToStringSimple())));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif

