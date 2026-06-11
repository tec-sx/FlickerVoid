#pragma once

#include "NativeGameplayTags.h"

#define UE_API FLICKERVOIDCORE_API

namespace FVCoreTags
{
	UE_API FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString = false);

	// ============================================================================
	// INPUT TAGS
	// ============================================================================
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Move);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Mouse);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Look_Stick);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Crouch);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Walk);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Sprint);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Jump);
	UE_API	UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputTag_Aim);
}

#undef UE_API