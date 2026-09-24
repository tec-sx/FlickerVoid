class AItem : AActor
{
    UPROPERTY(DefaultComponent)
    UFVInteractableComponent Interactable;
    default Interactable.Type = GameplayTags::Interactable_Item_Pickup;
    default Interactable.AddCompatibleInteractorTag(GameplayTags::Interactor_Tag_Player);
    default Interactable.AddCompatibleInteractorTag(GameplayTags::Interactor_Tag_AI);

    UPROPERTY(DefaultComponent, RootComponent)
    USceneComponent SceneRoot;

    UPROPERTY(DefaultComponent)
    UStaticMeshComponent Mesh;
}