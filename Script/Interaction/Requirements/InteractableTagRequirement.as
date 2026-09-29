// class UInteractableTagRequirement : UInteractionRequirement
// {
//     UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Requirement")
// 	FName Tag;

//     UFUNCTION(BlueprintOverride)
//     bool IsMet(
// 		FGameplayTag ActionTag,
// 		const UInteractorComponent Interactor,
//         const UInteractableComponent Interactable) const
//     {
//         if (IsValid(Interactable))
//         {
//             return Interactable.GetOwner().Tags.Contains(Tag);
//         }

//         return false;
//     }
// }