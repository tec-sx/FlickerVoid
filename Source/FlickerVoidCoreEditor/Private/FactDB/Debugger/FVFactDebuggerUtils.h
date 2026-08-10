#pragma once

#include "CoreMinimal.h"

using FFVFactTreeItemPtr = TSharedPtr<struct FFVFactTreeItem>;
struct FGameplayTagNode;

namespace Utils
{
	struct FFVFilterOptions
	{
		TArray<FString> SearchToggleStrings;
		TArray<FString> SearchBarStrings;

		bool bIsPlaying;
		bool bShowOnlyDefinedFacts;
		bool bShowFavoritesInMainTree;
	};

	void FilterFavoriteFactItemChildren(const TArray<FFVFactTreeItemPtr>& SourceArray,
	                                    TArray<FFVFactTreeItemPtr>& OutDestArray,
	                                    const FFVFilterOptions& Options);
	void FilterMainFactItemChildren(const TArray<FFVFactTreeItemPtr>& SourceArray,
	                                TArray<FFVFactTreeItemPtr>& OutDestArray,
	                                const FFVFilterOptions& Options);

	void GetLeafTags(const TSharedPtr<FGameplayTagNode>& Node, TArray<TSharedPtr<FGameplayTagNode>>& OutLeafTagNodes);
}
