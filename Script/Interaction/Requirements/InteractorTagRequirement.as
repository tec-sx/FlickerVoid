class UInteractorTagRequirement : UInteractionRequirement
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
	FGameplayTag Tag;

    UFUNCTION(BlueprintOverride)
    bool IsMet(
		FGameplayTag ActionTag,
		const UInteractorComponent Interactor,
        const UInteractableComponent Interactable) const
    {
        if (IsValid(Interactor))
        {
            UAbilitySystemComponent ASC = Interactor.GetOwner().GetComponentByClass(UAbilitySystemComponent);

            if (IsValid(ASC))
            {
                FGameplayTagContainer OwnedTags;
                ASC.GetOwnedGameplayTags(OwnedTags);

                return OwnedTags.HasTagExact(Tag);
            }
        }

        return false;
    }
}