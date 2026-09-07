class UBoolRequirement : UInteractionRequirement
{
    UPROPERTY(BlueprintReadWrite, Category = "Interaction|Configuration")
    bool bRequiredValue;

    UFUNCTION(BlueprintOverride)
    bool IsMet(FGameplayTag ActionTag, const UInteractorComponent Interactor, const UInteractableComponent Interactable) const
    {
        return bRequiredValue;
    }
}