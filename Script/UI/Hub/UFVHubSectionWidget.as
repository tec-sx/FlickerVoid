// One section of a hub page (Storage, Identity, Quests...). sections are placed by hand in a page, or in a tab view inside one.
UCLASS(Abstract)
class UFVHubSectionWidget : UUserWidget
{
    UPROPERTY(EditDefaultsOnly, Category = "Section")
    FText SectionTitle;

    private UFVHubPageWidget OwningPage;

    void InitializeSection(UFVHubPageWidget Page)
    {
        OwningPage = Page;
        OnSectionInitialized();
    }

    UFUNCTION(BlueprintPure, Category = "FV|UI")
    UFVHubPageWidget GetPage() const
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
