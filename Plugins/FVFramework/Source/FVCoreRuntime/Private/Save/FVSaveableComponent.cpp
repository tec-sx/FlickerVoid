#include "Save/FVSaveableComponent.h"

#include "GameFramework/Actor.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVSaveableComponent)

namespace FVSaveable
{
	static TArray<uint8> Write(UObject* Object)
	{
		TArray<uint8> Data;
		FMemoryWriter Writer(Data, true);
		FObjectAndNameAsStringProxyArchive Archive(Writer, false);
		Archive.ArIsSaveGame = true;
		Object->Serialize(Archive);
		return Data;
	}

	static void Read(UObject* Object, const TArray<uint8>& Data)
	{
		if (Data.IsEmpty())
		{
			return;
		}

		FMemoryReader Reader(Data, true);
		FObjectAndNameAsStringProxyArchive Archive(Reader, true);
		Archive.ArIsSaveGame = true;
		Object->Serialize(Archive);
	}
}

UFVSaveableComponent::UFVSaveableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

UFVSaveableComponent* UFVSaveableComponent::Find(const AActor* Actor)
{
	return Actor ? Actor->FindComponentByClass<UFVSaveableComponent>() : nullptr;
}

void UFVSaveableComponent::WriteActorData(TArray<uint8>& OutData) const
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	OutData.Reset();
	FMemoryWriter Writer(OutData, true);

	TArray<uint8> OwnerData = FVSaveable::Write(Owner);
	Writer << OwnerData;

	TInlineComponentArray<UActorComponent*> Components(Owner);
	int32 Count = Components.Num();
	Writer << Count;

	for (UActorComponent* Component : Components)
	{
		FString Name = Component->GetName();
		TArray<uint8> ComponentData = FVSaveable::Write(Component);
		Writer << Name;
		Writer << ComponentData;
	}
}

void UFVSaveableComponent::ReadActorData(const TArray<uint8>& InData)
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || InData.IsEmpty())
	{
		return;
	}

	FMemoryReader Reader(InData, true);

	TArray<uint8> OwnerData;
	Reader << OwnerData;
	FVSaveable::Read(Owner, OwnerData);

	int32 Count = 0;
	Reader << Count;

	TInlineComponentArray<UActorComponent*> Components(Owner);
	for (int32 Index = 0; Index < Count && !Reader.IsError(); ++Index)
	{
		FString Name;
		TArray<uint8> ComponentData;
		Reader << Name;
		Reader << ComponentData;

		UActorComponent* const* Found = Components.FindByPredicate([&Name](const UActorComponent* Component) { return Component->GetName() == Name; });
		if (Found != nullptr)
		{
			FVSaveable::Read(*Found, ComponentData);
		}
	}

	OnActorDataLoaded.Broadcast();
}

#if WITH_EDITOR
void UFVSaveableComponent::OnComponentCreated()
{
	Super::OnComponentCreated();

	if (!SaveId.IsValid())
	{
		SaveId = FGuid::NewGuid();
	}
}

void UFVSaveableComponent::PostEditImport()
{
	Super::PostEditImport();
	SaveId = FGuid::NewGuid();
}
#endif
