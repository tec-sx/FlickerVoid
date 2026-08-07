class AItem : AInteractable
{
    UPROPERTY(DefaultComponent)
    UStaticMeshComponent Mesh;

    default InteractionZone.SetBoxExtent(FVector(100)); 
    default InteractionTargetComponent.IdentityTag = GameplayTags::Interactable_Item;
    default InteractionTargetComponent.Priority = 90;
}