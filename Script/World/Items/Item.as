class AItem : AActor
{
    UPROPERTY(DefaultComponent)
    UFVInteractableComponent Interactable;
    default Interactable.Type = GameplayTags::Interactable_Item_Pickup;
    default Interactable.DetectionRadius = 120.f;
    default Interactable.FocusComponentTag = n"Interactable";

    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent SceneRoot;

    UPROPERTY(DefaultComponent)
    UStaticMeshComponent Mesh;
}