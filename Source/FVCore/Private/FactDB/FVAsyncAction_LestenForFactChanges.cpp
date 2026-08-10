#include "FactDB/FVAsyncAction_LestenForFactChanges.h"

#include "GameplayTagContainer.h"
#include "FactDB/FVFactSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAsyncAction_LestenForFactChanges)

UFVAsyncAction_LestenForFactChanges* UFVAsyncAction_LestenForFactChanges::ListenForFactChanges(
	UObject* WorldContextObject,
	FGameplayTag Tag)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		UFVAsyncAction_LestenForFactChanges* Action = NewObject<UFVAsyncAction_LestenForFactChanges>();
		Action->WorldPtr = World;
		Action->Tag = Tag;
		Action->RegisterWithGameInstance(World);

		return Action;
	}

	return nullptr;
}

void UFVAsyncAction_LestenForFactChanges::Activate()
{
	Super::Activate();

	if (const UWorld* World = WorldPtr.Get())
	{
		UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(World);
		FactSubsystem.GetOnFactValueChangedDelegate(Tag).AddUObject(this, &ThisClass::HandleFactValueChanged);
		FactSubsystem.GetOnFactBecameDefinedDelegate(Tag).AddUObject(this, &ThisClass::HandleFactBecameDefined);
		return;
	}

	SetReadyToDestroy();
}

void UFVAsyncAction_LestenForFactChanges::SetReadyToDestroy()
{
	if (const UWorld* World = WorldPtr.Get())
	{
		UFVFactSubsystem& FactSubsystem = UFVFactSubsystem::Get(World);
		FactSubsystem.GetOnFactValueChangedDelegate(Tag).RemoveAll(this);
		FactSubsystem.GetOnFactBecameDefinedDelegate(Tag).RemoveAll(this);
	}

	Super::SetReadyToDestroy();
}

void UFVAsyncAction_LestenForFactChanges::HandleFactValueChanged(int32 CurrentValue)
{
	if (OnFactValueChanged.IsBound() && OnFactBecameDefined.IsBound())
	{
		OnFactValueChanged.Broadcast(CurrentValue);
		return;
	}

	SetReadyToDestroy();
}

void UFVAsyncAction_LestenForFactChanges::HandleFactBecameDefined(int32 CurrentValue)
{
	if (OnFactValueChanged.IsBound() && OnFactBecameDefined.IsBound())
	{
		OnFactBecameDefined.Broadcast(CurrentValue);
		OnFactBecameDefined.Broadcast(CurrentValue);
		return;
	}

	SetReadyToDestroy();
}
