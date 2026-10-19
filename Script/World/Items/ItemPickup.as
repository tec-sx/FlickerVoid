// A world item the player can pick up: shows the World Mesh of the item its Pickup component holds.
// Dropped items spawn as a Blueprint of this (Inventory > Default Pickup Class); the Blueprint sets the
// interactable definition and looks.
class AItemPickup : AItem
{
    UPROPERTY(DefaultComponent)
    UFVPickupComponent Pickup;

    UFUNCTION(BlueprintOverride)
    void ConstructionScript()
    {
        ApplyItemMesh();
    }

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        Pickup.OnItemChanged.AddUFunction(this, n"ApplyItemMesh");
        ApplyItemMesh();
    }

    UFUNCTION()
    private void ApplyItemMesh()
    {
        UFVItemDefinition Item = Pickup.GetItem();
        UStaticMesh WorldMesh = Item != nullptr ? Item.LoadWorldMesh() : nullptr;
        if (WorldMesh != nullptr)
        {
            Mesh.SetStaticMesh(WorldMesh);
        }
    }
}
