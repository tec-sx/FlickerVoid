// One section of a hub page (Storage, Identity, Quests...). Pages create their sections from a class list, show them
// side by side or as tabs, and call Refresh whenever the page is shown.
UCLASS(Abstract)
class UFVHubMenuSectionWidget : UUserWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Section")
    FText SectionTitle;

    private UFVHubMenuPageWidget OwningPage;

    void InitializeSection(UFVHubMenuPageWidget Page)
    {
        OwningPage = Page;
        OnSectionInitialized();
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    UFVHubMenuPageWidget GetPage() const
    {
        return OwningPage;
    }

    // Rebuilds the section from its data source.
    UFUNCTION(BlueprintCallable, Category = "FV|UI")
    void Refresh()
    {
        OnRefresh();
    }

    // The widget the gamepad or keyboard focus should land on when this section is shown.
    UFUNCTION(BlueprintEvent, Category = "FV|UI")
    UWidget GetFocusTarget()
    {
        return nullptr;
    }

    // Called once the page has created the section; bind to data sources here.
    UFUNCTION(BlueprintEvent, Category = "FV|UI")
    void OnSectionInitialized() {}

    UFUNCTION(BlueprintEvent, Category = "FV|UI")
    void OnRefresh() {}

    // Called when the hub closes or the page is hidden; unbind from data sources that fire often.
    UFUNCTION(BlueprintEvent, Category = "FV|UI")
    void OnSectionHidden() {}
}
