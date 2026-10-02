#include "Save/FVSaveableComponent.h"

#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

UFVSaveableComponent::UFVSaveableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFVSaveableComponent::WriteActorData(TArray<uint8>& OutData) const
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr)
	{
		return;
	}

	FMemoryWriter Writer(OutData, true);
	FObjectAndNameAsStringProxyArchive Archive(Writer, false);
	Archive.ArIsSaveGame = true;
	Owner->Serialize(Archive);
}

void UFVSaveableComponent::ReadActorData(const TArray<uint8>& InData) const
{
	AActor* Owner = GetOwner();
	if (Owner == nullptr || InData.IsEmpty())
	{
		return;
	}

	FMemoryReader Reader(InData, true);
	FObjectAndNameAsStringProxyArchive Archive(Reader, true);
	Archive.ArIsSaveGame = true;
	Owner->Serialize(Archive);
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
