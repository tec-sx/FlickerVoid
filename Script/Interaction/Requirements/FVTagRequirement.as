// class UFVTagRequirement : UFVInteractionRequirement
// {
//     UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Requirement")
// 	FGameplayTag Tag;

//     UFUNCTION(BlueprintOverride)
//     bool IsMet(AActor Instigator) const
//     {
//         if (!Instigator)
// 	    {
// 	    	return false;
// 	    }

// 	    UAbilitySystemComponent ASC = AbilitySystem::GetAbilitySystemComponent(Instigator);

// 	    if (!ASC)
// 	    {
// 	    	return false;
// 	    }

// 	    return ASC.HasMatchingGameplayTag(Tag);
//     }

//     UFUNCTION(BlueprintOverride)
//     FText GetUnmetReason(AActor Instigator) const 
//     { 
//         return NSLOCTEXT("FVInteraction", "TagReqUnmet", "Missing required skill or status.");
//     }
// }