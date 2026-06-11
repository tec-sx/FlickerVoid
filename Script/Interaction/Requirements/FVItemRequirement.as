// class UFVItemRequirement : UFVInteractionRequirement
// {
//     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
//     TSubclassOf<AFVItem> ItemClass;

//     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
//     int32 MinimumQuantity = 1;

//     UFUNCTION(BlueprintOverride)
//     bool IsMet(AActor Instigator) const
//     {
//         if (!Instigator)
//         {
//         	return false;
//         }

//         UFVInventoryComponent Inventory = Instigator.GetComponentByClass(UFVInventoryComponent);

//         if (!Inventory)
//         {
//         	return false;
//         }

//         int32 Quantity = Inventory.GetItemQuantity(ItemClass);
//         return Quantity >= MinimumQuantity;
//     }

//     UFUNCTION(BlueprintOverride)
//     FText GetUnmetReason(AActor Instigator) const 
//     { 
//         return NSLOCTEXT("FVInteraction", "ItemReqUnmet", "Missing required item(s)."); 
//     }
// }