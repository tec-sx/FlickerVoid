#include "Validation/FVRulesValidator.h"

#include "Conditions/FVCondition.h"
#include "Conditions/FVEffect.h"
#include "Engine/Blueprint.h"
#include "Engine/World.h"
#include "Misc/DataValidation.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/UObjectHash.h"
#include "UObject/UnrealType.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVRulesValidator)

#define LOCTEXT_NAMESPACE "FVRulesValidator"

namespace
{
	constexpr int32 MaxDepth = 16;

	/** Walks a value's properties for condition sets and effect lists and collects their setup errors. */
	struct FRulesWalker
	{
		TArray<FText> Errors;
		bool bFoundRules = false;

		void WalkStruct(const UStruct* Struct, const void* Data, const FString& Path, const int32 Depth)
		{
			if (Depth > MaxDepth)
			{
				return;
			}

			for (TFieldIterator<FProperty> It(Struct); It; ++It)
			{
				const FProperty* Property = *It;
				for (int32 Index = 0; Index < Property->ArrayDim; ++Index)
				{
					const void* Value = Property->ContainerPtrToValuePtr<void>(Data, Index);
					const FString Name = Property->GetName();
					const FString ChildPath = Path.IsEmpty() ? Name : FString::Printf(TEXT("%s.%s"), *Path, *Name);
					WalkProperty(Property, Value, ChildPath, Depth + 1);
				}
			}
		}

		void WalkProperty(const FProperty* Property, const void* Value, const FString& Path, const int32 Depth)
		{
			if (const FArrayProperty* Array = CastField<FArrayProperty>(Property))
			{
				FScriptArrayHelper Helper(Array, Value);
				for (int32 Index = 0; Index < Helper.Num(); ++Index)
				{
					const FString ElementPath = FString::Printf(TEXT("%s[%d]"), *Path, Index);
					WalkProperty(Array->Inner, Helper.GetRawPtr(Index), ElementPath, Depth);
				}
				return;
			}

			const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (!StructProperty)
			{
				return;
			}

			const UScriptStruct* Struct = StructProperty->Struct;
			if (Struct == FFVConditionSet::StaticStruct())
			{
				const FText Error = static_cast<const FFVConditionSet*>(Value)->GetSetupError();
				Report(Path, Error);
				return;
			}

			if (Struct == FFVEffectList::StaticStruct())
			{
				const FText Error = static_cast<const FFVEffectList*>(Value)->GetSetupError();
				Report(Path, Error);
				return;
			}

			if (Struct == TBaseStructure<FInstancedStruct>::Get())
			{
				const FInstancedStruct* Instanced = static_cast<const FInstancedStruct*>(Value);
				const UScriptStruct* InstancedType = Instanced->GetScriptStruct();
				const uint8* InstancedMemory = Instanced->GetMemory();
				if (InstancedType && InstancedMemory)
				{
					WalkStruct(InstancedType, InstancedMemory, Path, Depth);
				}
				return;
			}

			WalkStruct(Struct, Value, Path, Depth);
		}

		void Report(const FString& Path, const FText& Error)
		{
			bFoundRules = true;
			if (!Error.IsEmpty())
			{
				Errors.Add(FText::Format(LOCTEXT("Error", "{0}: {1}"), FText::FromString(Path), Error));
			}
		}

		void WalkObject(const UObject* Object, const FString& Prefix)
		{
			WalkStruct(Object->GetClass(), Object, Prefix, 0);
		}
	};

	/** The objects whose properties are checked: the asset (or a Blueprint's class defaults) and every object inside it. */
	void GatherObjects(UObject* Asset, TArray<UObject*>& OutObjects)
	{
		UObject* Root = Asset;
		if (const UBlueprint* Blueprint = Cast<UBlueprint>(Asset))
		{
			if (!Blueprint->GeneratedClass)
			{
				return;
			}
			OutObjects.Add(Blueprint->GeneratedClass->GetDefaultObject());
			Root = Blueprint->GeneratedClass;
		}
		else
		{
			OutObjects.Add(Asset);
		}

		TArray<UObject*> Inner;
		GetObjectsWithOuter(Root, Inner, true);
		OutObjects.Append(Inner);
	}
}

bool UFVRulesValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject && !InObject->IsA<UWorld>();
}

EDataValidationResult UFVRulesValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	TArray<UObject*> Objects;
	GatherObjects(InAsset, Objects);

	FRulesWalker Walker;
	for (const UObject* Object : Objects)
	{
		const FString Prefix = Object == InAsset ? FString() : Object->GetName();
		Walker.WalkObject(Object, Prefix);
	}

	if (!Walker.bFoundRules)
	{
		return EDataValidationResult::NotValidated;
	}

	for (const FText& Error : Walker.Errors)
	{
		AssetFails(InAsset, Error);
	}

	if (Walker.Errors.IsEmpty())
	{
		AssetPasses(InAsset);
		return EDataValidationResult::Valid;
	}
	return EDataValidationResult::Invalid;
}

#undef LOCTEXT_NAMESPACE
