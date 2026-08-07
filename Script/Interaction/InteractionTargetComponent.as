class UInteractionTargetComponent : UActorComponent
{   
    UPROPERTY(Category = Interaction)
    FGameplayTag IdentityTag = GameplayTags::Interactable;

    UPROPERTY(Category = Interaction)
    int Priority = 1;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        UShapeComponent InteractionZone = GetOwner().FindComponentByTag(
			UShapeComponent,
			GameplayTags::Flow_Common_InteractionZone.GetTagName());
        
        if (IsValid(InteractionZone))
        {
            InteractionZone.OnComponentBeginOverlap.AddUFunction(this, n"OnInteractionZoneEnter");
            InteractionZone.OnComponentEndOverlap.AddUFunction(this, n"OnInteractionZoneExit");
        }
    }

    UFUNCTION(BlueprintOverride)
    void EndPlay(EEndPlayReason EndPlayReason)
    {
        UShapeComponent InteractionZone = GetOwner().FindComponentByTag(
			UShapeComponent,
			GameplayTags::Flow_Common_InteractionZone.GetTagName());

        if (IsValid(InteractionZone))
        {
            InteractionZone.OnComponentBeginOverlap.Clear();
            InteractionZone.OnComponentEndOverlap.Clear();
        }
    }

    UFUNCTION()
    private void OnInteractionZoneEnter(
		UPrimitiveComponent OverlappedComponent,
		AActor OtherActor,
        UPrimitiveComponent OtherComp,
		int OtherBodyIndex,
        bool bFromSweep,
		const FHitResult&in SweepResult)
    {
        if (OtherActor != GetOwner())
        {
            FInteractionTarget Payload;
            Payload.InteractableType = IdentityTag;
            Payload.Priority = Priority;
            Payload.Actor = GetOwner();

            UInteractionBus InteractionBus = UInteractionBus::Get();
            InteractionBus.OnInteractionZoneEvent.Broadcast(GameplayTags::Interaction_Event_InteractionZoneEnter, Payload);
        }
    }

    UFUNCTION()
    private void OnInteractionZoneExit(
		UPrimitiveComponent OverlappedComponent,
		AActor OtherActor,
        UPrimitiveComponent OtherComp,
		int OtherBodyIndex)
    {
        if (OtherActor != GetOwner())
        {
            FInteractionTarget Payload;
            Payload.InteractableType = IdentityTag;
            Payload.Priority = Priority;
            Payload.Actor = GetOwner();

            UInteractionBus InteractionBus = UInteractionBus::Get();
            InteractionBus.OnInteractionZoneEvent.Broadcast(GameplayTags::Interaction_Event_InteractionZoneExit, Payload);
        }
    }
}