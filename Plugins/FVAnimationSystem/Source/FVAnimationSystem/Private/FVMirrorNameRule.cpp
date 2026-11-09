#include "FVMirrorNameRule.h"

#include "FVAnimationSettings.h"

namespace
{
	/** Keeps an all-caps match all caps (_L becomes _R). */
	FString MatchCase(const FString& Replacement, const FString& Matched)
	{
		const bool bUpper = Matched.ToUpper() == Matched && Matched.ToLower() != Matched;
		return bUpper ? Replacement.ToUpper() : Replacement;
	}
}

FFVMirrorNameRule::FFVMirrorNameRule(TArray<FFVSidePair> InPairs)
	: Pairs(MoveTemp(InPairs))
{
	Pairs.RemoveAll([](const FFVSidePair& Pair) { return Pair.Left.IsEmpty() || Pair.Right.IsEmpty(); });
}

FFVMirrorNameRule FFVMirrorNameRule::FromSettings()
{
	return FFVMirrorNameRule(UFVAnimationSettings::Get().SidePairs);
}

bool FFVMirrorNameRule::MatchSuffix(const FString& Name, FMatch& OutMatch) const
{
	for (const FFVSidePair& Pair : Pairs)
	{
		for (const EFVBodySide Side : { EFVBodySide::Left, EFVBodySide::Right })
		{
			const FString& Suffix = Side == EFVBodySide::Left ? Pair.Left : Pair.Right;
			if (Name.Len() > Suffix.Len() && Name.EndsWith(Suffix, ESearchCase::IgnoreCase))
			{
				const FString& Other = Side == EFVBodySide::Left ? Pair.Right : Pair.Left;
				OutMatch.Side = Side;
				OutMatch.Length = Suffix.Len();
				OutMatch.Replacement = MatchCase(Other, Name.Right(Suffix.Len()));
				return true;
			}
		}
	}
	return false;
}

bool FFVMirrorNameRule::MatchToken(const FString& Name, int32 Index, FMatch& OutMatch) const
{
	for (const FFVSidePair& Pair : Pairs)
	{
		for (const EFVBodySide Side : { EFVBodySide::Left, EFVBodySide::Right })
		{
			const FString& Token = Side == EFVBodySide::Left ? Pair.Left : Pair.Right;
			const int32 End = Index + Token.Len();
			const bool bStartsAtBoundary = Token.StartsWith(TEXT("_")) || Index == 0 || Name[Index - 1] == TEXT('_');
			const bool bEndsAtBoundary = End == Name.Len() || (End < Name.Len() && Name[End] == TEXT('_'));
			if (End <= Name.Len() && bStartsAtBoundary && bEndsAtBoundary
				&& FCString::Strnicmp(*Name + Index, *Token, Token.Len()) == 0)
			{
				const FString& Other = Side == EFVBodySide::Left ? Pair.Right : Pair.Left;
				OutMatch.Side = Side;
				OutMatch.Length = Token.Len();
				OutMatch.Replacement = MatchCase(Other, Name.Mid(Index, Token.Len()));
				return true;
			}
		}
	}
	return false;
}

EFVBodySide FFVMirrorNameRule::GetBoneSide(const FString& BoneName) const
{
	FMatch Match;
	return MatchSuffix(BoneName, Match) ? Match.Side : EFVBodySide::Centre;
}

FString FFVMirrorNameRule::MirrorBoneName(const FString& BoneName) const
{
	FMatch Match;
	return MatchSuffix(BoneName, Match) ? BoneName.LeftChop(Match.Length) + Match.Replacement : FString();
}

EFVBodySide FFVMirrorNameRule::GetNameSide(const FString& Name) const
{
	bool bLeft = false;
	bool bRight = false;
	for (int32 Index = 0; Index < Name.Len(); ++Index)
	{
		FMatch Match;
		if (MatchToken(Name, Index, Match))
		{
			(Match.Side == EFVBodySide::Left ? bLeft : bRight) = true;
			Index += Match.Length - 1;
		}
	}
	return bLeft == bRight ? EFVBodySide::Centre : (bLeft ? EFVBodySide::Left : EFVBodySide::Right);
}

FString FFVMirrorNameRule::MirrorName(const FString& Name) const
{
	FString Result;
	Result.Reserve(Name.Len() + 4);
	bool bSwapped = false;
	for (int32 Index = 0; Index < Name.Len(); ++Index)
	{
		FMatch Match;
		if (MatchToken(Name, Index, Match))
		{
			Result += Match.Replacement;
			Index += Match.Length - 1;
			bSwapped = true;
		}
		else
		{
			Result.AppendChar(Name[Index]);
		}
	}
	return bSwapped ? Result : FString();
}
