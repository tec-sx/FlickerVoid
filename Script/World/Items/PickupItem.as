class APickupItem : AItem
{
    UPROPERTY(DefaultComponent)
    UInteractableComponent Interactable;
    default Interactable.Type = GameplayTags::Interactable_Item_Pickup;
    default Interactable.DetectionRadius = 120.f;
    default Interactable.FocusComponentTag = n"Interactable";
    
    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent SceneRoot;

    UPROPERTY(DefaultComponent)
    UStaticMeshComponent Mesh;

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        Interactable.OnInteractionExecuted.AddUFunction(this, n"OnInteractionExecuted");
    }

    UFUNCTION(BlueprintEvent)
    void OnInteractionExecuted(const FGameplayTag&in ActionTag, UInteractorComponent Interactor)
    {
        if (ActionTag == GameplayTags::Interaction_Action_Pickup)
        {
            Print("Pickup");
        }
    }
}