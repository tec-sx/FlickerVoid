// Hub page for the inventory (Storage, Outfit, Information). Owns the model its sections share, bound to the
// player pawn's inventory and equipment while the page is shown.
UCLASS(Abstract)
class UFVInventoryPageWidget : UFVHubPageWidget
{
    UPROPERTY(Transient)
    UFVInventoryModel Model;

    UFUNCTION(BlueprintOverride)
    void OnInitialized()
    {
        Model = NewObject(this, UFVInventoryModel);
        Super::OnInitialized();
    }

    UFUNCTION(BlueprintOverride)
    void OnPageShown()
    {
        Model.Bind(GetOwningPlayerPawn());
    }

    UFUNCTION(BlueprintOverride)
    void Destruct()
    {
        Model.Unbind();
    }
}

namespace FVInventoryUI
{
    UFVInventoryModel GetModel(UFVHubSectionWidget Section)
    {
        UFVInventoryPageWidget Page = Section != nullptr ? Cast<UFVInventoryPageWidget>(Section.GetPage()) : nullptr;
        return Page != nullptr ? Page.Model : nullptr;
    }

    FText GetItemName(UFVItemDefinition Item)
    {
        if (Item == nullptr)
        {
            return FText();
        }
        return Item.Display.Name.IsEmpty() ? FText::FromString(Item.GetName().ToString()) : Item.Display.Name;
    }
}
