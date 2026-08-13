class UFVPickupAbility : UFVGameplayAbility
{
UPROPERTY(EditDefaultsOnly, Category = "Pickup")
UAnimMontage PickupMontage;

UFUNCTION(BlueprintOverride)
void ActivateAbility(
FGameplayAbilitySpecHandle Handle,
FGameplayAbilityActorInfo ActorInfo,
FGameplayAbilityActivationInfo ActivationInfo,
FGameplayEventData TriggerEventData)
{
if (PickupMontage == nullptr)
{
FinishPickup(Handle, ActorInfo, ActivationInfo);
return;
}

UAbilityTask_PlayMontageAndWait MontageTask =
UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, n"Pickup", PickupMontage);

MontageTask.OnCompleted.AddUFunction(this, n"HandleMontageFinished");
MontageTask.OnInterrupted.AddUFunction(this, n"HandleMontageFinished");
MontageTask.OnCancelled.AddUFunction(this, n"HandleMontageFinished");
MontageTask.OnBlendOut.AddUFunction(this, n"HandleMontageFinished");
MontageTask.ReadyForActivation();
}

UFUNCTION()
void HandleMontageFinished()
{
FinishPickup(CurrentAbilitySpecHandle, CurrentActorInfo, CurrentActivationInfo);
}

private void FinishPickup(
FGameplayAbilitySpecHandle Handle,
FGameplayAbilityActorInfo ActorInfo,
FGameplayAbilityActivationInfo ActivationInfo)
{
AActor Instigator = ActorInfo.AvatarActor;

UFVInteractionOfferComponent Offers =
Cast<UFVInteractionOfferComponent>(Instigator.GetComponentByClass(UFVInteractionOfferComponent));

if (Offers != nullptr)
{
UFVInteractionTargetComponent Target = Offers.GetEngagedTarget();

if (Target != nullptr)
{
AFVItemPickup Pickup = Cast<AFVItemPickup>(Target.GetOwner());
UFVInventoryComponent Inventory =
Cast<UFVInventoryComponent>(Instigator.GetComponentByClass(UFVInventoryComponent));

if (Pickup != nullptr && Inventory != nullptr)
{
Pickup.ExecutePickup(Inventory);
}
}
}

EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}
}
