// class UFVAttributeRequirement : UFVInteractionRequirement
// {
//     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
//     FGameplayAttribute Attribute;

//     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
//     float MinimumValue = 0.f;

//     UFUNCTION(BlueprintOverride)
//     bool IsMet(AActor Instigator) const
//     {
//         if (!Instigator)
//         {
//         	return false;
//         }

//         UAbilitySystemComponent ASC = AbilitySystem::GetAbilitySystemComponent(Instigator);

//         if (!ASC)
//         {
//         	return false;
//         }

//         bool bFound = false;
// 	    float Value = ASC.GetGameplayAttributeValue(Attribute, bFound);
//         bool result = bFound && Value >= MinimumValue;

//         return result;
//     }

//     UFUNCTION(BlueprintOverride)
//     FText GetUnmetReason(AActor Instigator) const 
//     { 
//         return NSLOCTEXT("FVInteraction", "AttributeReqUnmet", "Attribute level too low."); 
//     }
// }