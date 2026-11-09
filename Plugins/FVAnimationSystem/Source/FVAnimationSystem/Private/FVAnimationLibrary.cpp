#include "FVAnimationLibrary.h"

#include "FVMirrorNameRule.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAnimationLibrary)

namespace
{
	FName ToName(const FString& Name)
	{
		return Name.IsEmpty() ? NAME_None : FName(*Name);
	}
}

EFVBodySide UFVAnimationLibrary::GetBoneSide(FName Bone)
{
	return FFVMirrorNameRule::FromSettings().GetBoneSide(Bone.ToString());
}

FName UFVAnimationLibrary::GetMirrorBoneName(FName Bone)
{
	return ToName(FFVMirrorNameRule::FromSettings().MirrorBoneName(Bone.ToString()));
}

EFVBodySide UFVAnimationLibrary::GetNameSide(FName Name)
{
	return FFVMirrorNameRule::FromSettings().GetNameSide(Name.ToString());
}

FName UFVAnimationLibrary::GetMirrorName(FName Name)
{
	return ToName(FFVMirrorNameRule::FromSettings().MirrorName(Name.ToString()));
}
