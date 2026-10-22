#include "Items/FVHeldItemComponent.h"

#include "Animation/AnimSequenceBase.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Equipment/FVEquipmentComponent.h"
#include "FVGameplayModule.h"
#include "FVGameplayTags.h"
#include "GameFramework/Actor.h"
#include "Inventory/FVInventoryComponent.h"
#include "Items/FVHeldItemBehaviourComponent.h"
#include "Items/FVItemDefinition.h"
#include "Items/FVItemLibrary.h"
#include "Pickup/FVPickupComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVHeldItemComponent)

namespace FVHeldItems
{
	void SetShown(AActor* HeldActor, UStaticMeshComponent* Prop, const bool bShown)
	{
		if (IsValid(HeldActor))
		{
			HeldActor->SetActorHiddenInGame(!bShown);
		}

		if (IsValid(Prop))
		{
			Prop->SetVisibility(bShown);
		}
	}
}

void UFVHeldItemComponent::PutAway()
{
	++BusyCount;
	RefreshPresentation();
}

void UFVHeldItemComponent::TakeOut()
{
	BusyCount = FMath::Max(0, BusyCount - 1);
	RefreshPresentation();
}

bool UFVHeldItemComponent::UseHeld(const bool bRightHand)
{
	if (!Equipment || BusyCount > 0)
	{
		return false;
	}

	AActor* HeldActor = bRightHand ? RightActor : LeftActor;
	if (UFVHeldItemBehaviourComponent* Behaviour = IsValid(HeldActor) ? HeldActor->FindComponentByClass<UFVHeldItemBehaviourComponent>() : nullptr)
	{
		Behaviour->Use(GetOwner());
		return true;
	}

	const FGameplayTag Slot = UFVItemLibrary::GetHandSlot(bRightHand);
	UFVItemDefinition* Item = Equipment->GetEquipped(Slot);
	if (!Item || !Item->IsUsable())
	{
		return false;
	}

	return UFVItemLibrary::SendItemEvent(GetOwner(), FVGameplayTags::Event_Item_Use, Item, Equipment->GetEquippedInstance(Slot), nullptr, nullptr, 1);
}

UAnimSequenceBase* UFVHeldItemComponent::GetHoldPose(const bool bRightHand) const
{
	const UFVItemDefinition* Item = Equipment && BusyCount == 0 ? Equipment->GetEquipped(UFVItemLibrary::GetHandSlot(bRightHand)) : nullptr;
	return Item ? UFVItemLibrary::FindHoldPose(GetOwner(), Item) : nullptr;
}

void UFVHeldItemComponent::BeginPlay()
{
	Super::BeginPlay();

	Equipment = UFVEquipmentComponent::Find(GetOwner());
	if (!Equipment)
	{
		UE_LOG(LogFVGameplay, Warning, TEXT("%s: held items need an equipment component with hand slots."), *GetNameSafe(GetOwner()));
		return;
	}

	Equipment->OnEquipmentChanged.AddDynamic(this, &UFVHeldItemComponent::HandleEquipmentChanged);
	Equipment->OnEquipmentReset.AddDynamic(this, &UFVHeldItemComponent::HandleEquipmentReset);

	RefreshHand(true);
	RefreshHand(false);
}

void UFVHeldItemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Equipment)
	{
		Equipment->OnEquipmentChanged.RemoveAll(this);
		Equipment->OnEquipmentReset.RemoveAll(this);
	}

	ClearHand(true);
	ClearHand(false);

	Super::EndPlay(EndPlayReason);
}

void UFVHeldItemComponent::HandleEquipmentChanged(const FGameplayTag Slot, UFVItemDefinition* OldItem, UFVItemDefinition* NewItem)
{
	if (Slot == UFVItemLibrary::GetHandSlot(true))
	{
		RefreshHand(true);
	}
	else if (Slot == UFVItemLibrary::GetHandSlot(false))
	{
		RefreshHand(false);
	}
}

void UFVHeldItemComponent::HandleEquipmentReset()
{
	RefreshHand(true);
	RefreshHand(false);
}

void UFVHeldItemComponent::RefreshHand(const bool bRightHand)
{
	ClearHand(bRightHand);

	const FGameplayTag Slot = UFVItemLibrary::GetHandSlot(bRightHand);
	UFVItemDefinition* Item = Equipment->GetEquipped(Slot);
	USkeletalMeshComponent* CharacterMesh = UFVItemLibrary::GetCharacterMesh(GetOwner());

	if (Item && CharacterMesh)
	{
		const FName Socket = UFVItemLibrary::GetHandSocket(bRightHand);
		if (AActor* HeldActor = SpawnHeldActor(Item, Slot, CharacterMesh, Socket))
		{
			SetHand(bRightHand, HeldActor, nullptr);
		}
		else
		{
			SetHand(bRightHand, nullptr, UFVItemLibrary::AttachProp(GetOwner(), Item, Socket));
		}
	}

	RefreshPresentation();
}

AActor* UFVHeldItemComponent::SpawnHeldActor(UFVItemDefinition* Item, const FGameplayTag& Slot, USkeletalMeshComponent* CharacterMesh, const FName Socket) const
{
	UClass* ActorClass = UFVItemLibrary::LoadActorClass(Item);
	UWorld* World = GetWorld();
	if (!ActorClass || !World)
	{
		return nullptr;
	}

	const FTransform SpawnTransform = CharacterMesh->GetSocketTransform(Socket);
	AActor* HeldActor = World->SpawnActorDeferred<AActor>(ActorClass, SpawnTransform, GetOwner(), nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!HeldActor)
	{
		return nullptr;
	}

	if (UFVPickupComponent* Pickup = HeldActor->FindComponentByClass<UFVPickupComponent>())
	{
		FFVItemStack Unit;
		const UFVInventoryComponent* Inventory = UFVInventoryComponent::Find(GetOwner());
		if (!Inventory || !Inventory->GetStack(Equipment->GetEquippedInstance(Slot), Unit))
		{
			Unit.Item = Item;
		}

		Unit.Quantity = 1;
		Pickup->SetStack(Unit);
	}

	HeldActor->FinishSpawning(SpawnTransform);
	HeldActor->SetActorEnableCollision(false);

	USceneComponent* Root = HeldActor->GetRootComponent();
	if (!Root)
	{
		return HeldActor;
	}

	if (UPrimitiveComponent* Body = Cast<UPrimitiveComponent>(Root))
	{
		Body->SetSimulatePhysics(false);
	}

	Root->AttachToComponent(CharacterMesh, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepRelative, false), Socket);
	Root->SetRelativeTransform(UFVItemLibrary::GetGripTransform(Item, Cast<UStaticMeshComponent>(Root)));
	return HeldActor;
}

void UFVHeldItemComponent::SetHand(const bool bRightHand, AActor* HeldActor, UStaticMeshComponent* Prop)
{
	if (bRightHand)
	{
		RightActor = HeldActor;
		RightProp = Prop;
	}
	else
	{
		LeftActor = HeldActor;
		LeftProp = Prop;
	}
}

void UFVHeldItemComponent::ClearHand(const bool bRightHand)
{
	AActor* HeldActor = bRightHand ? RightActor : LeftActor;
	if (IsValid(HeldActor))
	{
		HeldActor->Destroy();
	}

	UFVItemLibrary::DetachProp(bRightHand ? RightProp : LeftProp);
	SetHand(bRightHand, nullptr, nullptr);
}

void UFVHeldItemComponent::RefreshPresentation()
{
	const bool bShown = BusyCount == 0;
	FVHeldItems::SetShown(RightActor, RightProp, bShown);
	FVHeldItems::SetShown(LeftActor, LeftProp, bShown);

	OnHeldItemsChanged.Broadcast();
}
