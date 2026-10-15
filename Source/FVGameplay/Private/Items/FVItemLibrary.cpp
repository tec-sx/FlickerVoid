#include "Items/FVItemLibrary.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/CapsuleComponent.h"
#include "Conditions/FVConditionLibrary.h"
#include "Data/FVCharacterDefinition.h"
#include "Data/FVInteractableDefinition.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "FVGameplayModule.h"
#include "FVGameplayTags.h"
#include "FVSocialLibrary.h"
#include "GameFramework/Character.h"
#include "GameplayPrediction.h"
#include "Identity/FVIdentityComponent.h"
#include "Items/FVItemAnimationFragment.h"
#include "Items/FVItemDefinition.h"
#include "Items/FVItemEventPayload.h"
#include "Items/FVItemFragments.h"
#include "Items/FVItemSettings.h"
#include "Pickup/FVPickupComponent.h"
#include "Sound/SoundBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVItemLibrary)

namespace FVItems
{
	template <typename TFragment>
	bool CopyFragment(const UFVItemDefinition* Item, TFragment& OutFragment)
	{
		const TFragment* Fragment = Item ? Item->FindFragment<TFragment>() : nullptr;
		if (!Fragment)
		{
			return false;
		}
		OutFragment = *Fragment;
		return true;
	}

	/** The character's entry for the style or its closest parent, else the settings default's. */
	template <typename TAsset>
	TAsset* FindByStyle(const TMap<FGameplayTag, TObjectPtr<TAsset>>* CharacterEntries, const TMap<FGameplayTag, TSoftObjectPtr<TAsset>>& DefaultEntries, const FGameplayTag& Style)
	{
		if (CharacterEntries)
		{
			for (FGameplayTag Tag = Style; Tag.IsValid(); Tag = Tag.RequestDirectParent())
			{
				if (TAsset* Asset = CharacterEntries->FindRef(Tag).Get())
				{
					return Asset;
				}
			}
		}

		for (FGameplayTag Tag = Style; Tag.IsValid(); Tag = Tag.RequestDirectParent())
		{
			if (const TSoftObjectPtr<TAsset>* Asset = DefaultEntries.Find(Tag); Asset && !Asset->IsNull())
			{
				return Asset->LoadSynchronous();
			}
		}
		return nullptr;
	}

	const FFVCharacterFragment_ItemAnimations* FindCharacterAnimations(const AActor* Character)
	{
		const UFVCharacterDefinition* Definition = UFVIdentityComponent::GetActorDefinition(Character);
		return Definition ? Definition->FindFragment<FFVCharacterFragment_ItemAnimations>() : nullptr;
	}
}

bool UFVItemLibrary::GetWorldFragment(const UFVItemDefinition* Item, FFVItemFragment_World& OutFragment)
{
	return FVItems::CopyFragment(Item, OutFragment);
}

bool UFVItemLibrary::GetAnimationFragment(const UFVItemDefinition* Item, FFVItemFragment_Animation& OutFragment)
{
	return FVItems::CopyFragment(Item, OutFragment);
}

bool UFVItemLibrary::GetThrowableFragment(const UFVItemDefinition* Item, FFVItemFragment_Throwable& OutFragment)
{
	return FVItems::CopyFragment(Item, OutFragment);
}

UStaticMesh* UFVItemLibrary::LoadWorldMesh(const UFVItemDefinition* Item)
{
	const FFVItemFragment_World* World = Item ? Item->FindFragment<FFVItemFragment_World>() : nullptr;
	return World ? World->Mesh.LoadSynchronous() : nullptr;
}

USoundBase* UFVItemLibrary::LoadSound(const TSoftObjectPtr<USoundBase>& Sound)
{
	return Sound.LoadSynchronous();
}

UFVInteractableDefinition* UFVItemLibrary::FindInteractable(const UFVItemDefinition* Item)
{
	if (!Item)
	{
		return nullptr;
	}

	const FFVItemFragment_World* World = Item->FindFragment<FFVItemFragment_World>();
	if (World && World->Interactable)
	{
		return World->Interactable;
	}

	const UFVItemSettings& Settings = UFVItemSettings::Get();
	for (FGameplayTag Category = Item->Category; Category.IsValid(); Category = Category.RequestDirectParent())
	{
		if (const TSoftObjectPtr<UFVInteractableDefinition>* Definition = Settings.InteractablesByCategory.Find(Category); Definition && !Definition->IsNull())
		{
			return Definition->LoadSynchronous();
		}
	}
	return Settings.DefaultInteractable.LoadSynchronous();
}

UAnimMontage* UFVItemLibrary::FindMontage(const AActor* Character, const FGameplayTag Style)
{
	const FFVCharacterFragment_ItemAnimations* Animations = FVItems::FindCharacterAnimations(Character);
	return FVItems::FindByStyle<UAnimMontage>(Animations ? &Animations->Montages : nullptr, UFVItemSettings::Get().DefaultMontages, Style);
}

UAnimMontage* UFVItemLibrary::FindUseMontage(const AActor* Character, const UFVItemDefinition* Item)
{
	FGameplayTag Style = FVGameplayTags::Animation_Item_Use;
	if (const FFVItemFragment_Animation* Animation = Item ? Item->FindFragment<FFVItemFragment_Animation>() : nullptr)
	{
		if (UAnimMontage* Override = Animation->UseMontage.LoadSynchronous())
		{
			return Override;
		}
		if (Animation->UseStyle.IsValid())
		{
			Style = Animation->UseStyle;
		}
	}
	return FindMontage(Character, Style);
}

UAnimSequenceBase* UFVItemLibrary::FindHoldPose(const AActor* Character, const UFVItemDefinition* Item)
{
	FGameplayTag Style = FVGameplayTags::Animation_Item_Hold;
	const FFVItemFragment_Animation* Animation = Item ? Item->FindFragment<FFVItemFragment_Animation>() : nullptr;
	if (Animation && Animation->HoldStyle.IsValid())
	{
		Style = Animation->HoldStyle;
	}

	const FFVCharacterFragment_ItemAnimations* Animations = FVItems::FindCharacterAnimations(Character);
	return FVItems::FindByStyle<UAnimSequenceBase>(Animations ? &Animations->HoldPoses : nullptr, UFVItemSettings::Get().DefaultHoldPoses, Style);
}

FGameplayTag UFVItemLibrary::GetPickupStyle(const AActor* Character, const FVector ItemLocation)
{
	if (!Character)
	{
		return FVGameplayTags::Animation_Item_Pickup_Mid;
	}

	double FeetZ = Character->GetActorLocation().Z;
	const ACharacter* AsCharacter = Cast<ACharacter>(Character);
	if (AsCharacter && AsCharacter->GetCapsuleComponent())
	{
		FeetZ -= AsCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	}

	const double Height = ItemLocation.Z - FeetZ;
	const UFVItemSettings& Settings = UFVItemSettings::Get();
	if (Height < Settings.PickupLowHeight)
	{
		return FVGameplayTags::Animation_Item_Pickup_Low;
	}
	if (Height > Settings.PickupHighHeight)
	{
		return FVGameplayTags::Animation_Item_Pickup_High;
	}
	return FVGameplayTags::Animation_Item_Pickup_Mid;
}

FName UFVItemLibrary::GetHandSocket(const bool bRightHand)
{
	const UFVItemSettings& Settings = UFVItemSettings::Get();
	return bRightHand ? Settings.RightHandSocket : Settings.LeftHandSocket;
}

bool UFVItemLibrary::CanUseItem(const UFVItemDefinition* Item, AActor* User)
{
	const FFVItemFragment_Usable* Usable = Item ? Item->FindFragment<FFVItemFragment_Usable>() : nullptr;
	return Usable && User && Usable->UseConditions.Evaluate(UFVConditionLibrary::MakeContext(User, User));
}

bool UFVItemLibrary::ApplyUseEffects(const UFVItemDefinition* Item, AActor* User)
{
	if (!CanUseItem(Item, User))
	{
		return false;
	}

	Item->FindFragment<FFVItemFragment_Usable>()->Effects.Apply(UFVConditionLibrary::MakeContext(User, User));
	return true;
}

bool UFVItemLibrary::CanGiveItem(const AActor* Giver, const AActor* Receiver, const UFVItemDefinition* Item, const int32 Quantity, const int32 Money)
{
	const bool bGivesItem = Item && Quantity > 0;
	if (!Giver || !Receiver || (!bGivesItem && Money <= 0))
	{
		return false;
	}

	const UFVInventoryComponent* From = UFVInventoryComponent::Find(Giver);
	if (!From || (bGivesItem && !From->HasItem(Item, Quantity)) || (Money > 0 && !From->CanAfford(Money)))
	{
		return false;
	}

	const UFVInventoryComponent* To = UFVInventoryComponent::Find(Receiver);
	return !bGivesItem || !To || To->CanAddItem(Item, Quantity);
}

bool UFVItemLibrary::GiveItem(AActor* Giver, AActor* Receiver, UFVItemDefinition* Item, const int32 Quantity, const int32 Money, const bool bAsGift)
{
	if (!CanGiveItem(Giver, Receiver, Item, Quantity, Money))
	{
		return false;
	}

	UFVInventoryComponent* From = UFVInventoryComponent::Find(Giver);
	UFVInventoryComponent* To = UFVInventoryComponent::Find(Receiver);
	FGameplayTagContainer GiftTags;

	if (Item && Quantity > 0)
	{
		if (!From->RemoveItem(Item, Quantity))
		{
			return false;
		}
		if (To)
		{
			To->AddItem(Item, Quantity);
		}
		GiftTags.AppendTags(Item->Tags);
		GiftTags.AddTag(Item->Category);
	}

	if (Money > 0)
	{
		if (To)
		{
			From->TransferMoney(To, Money);
		}
		else
		{
			From->RemoveMoney(Money);
		}
	}

	if (bAsGift)
	{
		UFVSocialLibrary::ReceiveGift(Giver, Receiver, GiftTags);
	}
	return true;
}

AActor* UFVItemLibrary::SpawnWorldItem(UObject* WorldContext, const FFVItemStack& Stack, const FTransform& Transform)
{
	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContext, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (!World || !Stack.Item || Stack.Quantity <= 0)
	{
		return nullptr;
	}

	const FFVItemFragment_World* Fragment = Stack.Item->FindFragment<FFVItemFragment_World>();
	UClass* ActorClass = Fragment ? Fragment->ActorClass.LoadSynchronous() : nullptr;
	if (!ActorClass)
	{
		ActorClass = UFVItemSettings::Get().WorldItemClass.LoadSynchronous();
	}
	if (!ActorClass)
	{
		UE_LOG(LogFVGameplay, Warning, TEXT("Can't spawn %s in the world: no World Item Class in the Items settings."), *GetNameSafe(Stack.Item));
		return nullptr;
	}

	AActor* Actor = World->SpawnActorDeferred<AActor>(ActorClass, Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!Actor)
	{
		return nullptr;
	}

	FFVItemStack WorldStack = Stack;
	WorldStack.InstanceId.Invalidate();

	// Native and script components exist before construction; components added in a Blueprint only after it.
	UFVPickupComponent* Pickup = Actor->FindComponentByClass<UFVPickupComponent>();
	if (Pickup)
	{
		Pickup->SetStack(WorldStack);
	}

	Actor->FinishSpawning(Transform);

	if (!Pickup)
	{
		Pickup = Actor->FindComponentByClass<UFVPickupComponent>();
		if (Pickup)
		{
			Pickup->SetStack(WorldStack);
		}
		else
		{
			UE_LOG(LogFVGameplay, Warning, TEXT("World item %s has no pickup component; %s can't be picked up again."), *GetNameSafe(ActorClass), *GetNameSafe(Stack.Item));
		}
	}
	return Actor;
}

bool UFVItemLibrary::SendItemEvent(AActor* Actor, const FGameplayTag EventTag, UFVItemDefinition* Item, const FGuid InstanceId, AActor* WorldActor, AActor* OtherActor, const int32 Quantity)
{
	if (!Actor)
	{
		return false;
	}

	UFVItemEventPayload* Payload = NewObject<UFVItemEventPayload>(Actor);
	Payload->Item = Item;
	Payload->InstanceId = InstanceId;
	Payload->WorldActor = WorldActor;
	Payload->OtherActor = OtherActor;
	Payload->Quantity = Quantity;
	return SendItemPayload(Actor, EventTag, Payload);
}

bool UFVItemLibrary::SendItemPayload(AActor* Actor, const FGameplayTag EventTag, UFVItemEventPayload* Payload)
{
	UAbilitySystemComponent* AbilitySystem = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor);
	if (!AbilitySystem || !EventTag.IsValid())
	{
		return false;
	}

	AActor* Target = nullptr;
	if (Payload)
	{
		Target = Payload->WorldActor ? Payload->WorldActor.Get() : Payload->OtherActor.Get();
	}

	FGameplayEventData EventData;
	EventData.EventTag = EventTag;
	EventData.Instigator = Actor;
	EventData.Target = Target;
	EventData.OptionalObject = Payload;

	FScopedPredictionWindow PredictionWindow(AbilitySystem, true);
	return AbilitySystem->HandleGameplayEvent(EventTag, &EventData) > 0;
}

UFVItemEventPayload* UFVItemLibrary::GetItemPayload(const FGameplayEventData& EventData)
{
	return const_cast<UFVItemEventPayload*>(Cast<UFVItemEventPayload>(EventData.OptionalObject.Get()));
}

const UFVItemSettings* UFVItemLibrary::GetSettings()
{
	return GetDefault<UFVItemSettings>();
}
