#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "FVFragment.generated.h"

/** Base for data fragments composed onto definitions. Each definition type derives its own fragment base. */
USTRUCT(BlueprintType, meta = (Hidden))
struct FVCORERUNTIME_API FFVFragment
{
	GENERATED_BODY()

	virtual ~FFVFragment() = default;
};

namespace FVFragments
{
	template <typename T, typename TBase>
	const T* Find(const TArray<TInstancedStruct<TBase>>& Fragments)
	{
		for (const TInstancedStruct<TBase>& Fragment : Fragments)
		{
			if (const T* Found = Fragment.template GetPtr<T>())
			{
				return Found;
			}
		}
		return nullptr;
	}
}
