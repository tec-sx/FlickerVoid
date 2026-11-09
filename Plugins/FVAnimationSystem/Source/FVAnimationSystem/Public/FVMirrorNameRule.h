#pragma once

#include "CoreMinimal.h"
#include "FVAnimationTypes.h"

/**
 * Left/right naming. Bones carry their side as a suffix (thigh_l); pose and curve names carry it as a whole
 * token followed by the end or an underscore (upperarm_l_up_90), so _lower or _root never count.
 */
struct FVANIMATIONSYSTEM_API FFVMirrorNameRule
{
	explicit FFVMirrorNameRule(TArray<FFVSidePair> InPairs);

	/** The rule built from the project's Animation System settings. */
	static FFVMirrorNameRule FromSettings();

	EFVBodySide GetBoneSide(const FString& BoneName) const;

	/** The bone name with its side suffix swapped; empty for a centre bone. */
	FString MirrorBoneName(const FString& BoneName) const;

	/** Centre when the name has no side token, or tokens of both sides. */
	EFVBodySide GetNameSide(const FString& Name) const;

	/** The name with every side token swapped; empty when it has none. */
	FString MirrorName(const FString& Name) const;

private:
	struct FMatch
	{
		EFVBodySide Side = EFVBodySide::Centre;
		int32 Length = 0;
		FString Replacement;
	};

	bool MatchSuffix(const FString& Name, FMatch& OutMatch) const;
	bool MatchToken(const FString& Name, int32 Index, FMatch& OutMatch) const;

	TArray<FFVSidePair> Pairs;
};
