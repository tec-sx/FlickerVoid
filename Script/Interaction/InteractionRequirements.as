UCLASS(DisplayName = "Tag Requirement", meta = (DisplayThumbnail = "false"))
class UFVTagRequirement : UFVInteractionRequirement
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite,Category = "Requirement")
	FGameplayTag Tag;

    UFUNCTION(BlueprintOverride)
    bool IsMet(AActor Instigator) const
    {
        if (!Instigator)
	    {
	    	return false;
	    }

	    UAbilitySystemComponent ASC = AbilitySystem::GetAbilitySystemComponent(Instigator);

	    if (!ASC)
	    {
	    	return false;
	    }

	    return ASC->HasMatchingGameplayTag(Tag);
    }

    UFUNCTION(BlueprintOverride)
    FText GetUnmetReason(AActor Instigator) const 
    { 
        return NSLOCTEXT("FVInteraction", "TagReqUnmet", "Missing required skill or status.");
    }
}

UCLASS(DisplayName = "Attribute Requirement", meta = (DisplayThumbnail = "false"))
class UFVAttributeRequirement : UFVInteractionRequirement
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
    FGameplayAttribute Attribute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
    float MinimumValue = 0.f;

    UFUNCTION(BlueprintOverride)
    bool IsMet(AActor Instigator) const
    {
        if (!Instigator)
        {
        	return false;
        }

        UAbilitySystemComponent ASC = AbilitySystem::GetAbilitySystemComponent(Instigator);

        if (!ASC)
        {
        	return false;
        }

        bool bFound = false;
	    float Value = ASC->GetGameplayAttributeValue(Attribute, bFound);
        bool result = bFound && Value >= MinimumValue;

        return result;
    }

    UFUNCTION(BlueprintOverride)
    FText GetUnmetReason(AActor Instigator) const 
    { 
        return NSLOCTEXT("FVInteraction", "AttributeReqUnmet", "Attribute level too low."); 
    }
}

UCLASS(Abstract, DisplayName = "Item Requirement", meta = (DisplayThumbnail = "false"))
class UFVItemRequirement : UFVInteractionRequirement
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
    TSubclassOf<AFVItem> ItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirement")
    int32 MinimumQuantity = 1;

    UFUNCTION(BlueprintOverride)
    bool IsMet(AActor Instigator) const
    {
        if (!Instigator)
        {
        	return false;
        }

        UFVInventoryComponent Inventory = Instigator->GetComponentByClass(UFVInventoryComponent);

        if (!Inventory)
        {
        	return false;
        }

        int32 Quantity = Inventory->GetItemQuantity(ItemClass);
        return Quantity >= MinimumQuantity;
    }

    UFUNCTION(BlueprintOverride)
    FText GetUnmetReason(AActor Instigator) const 
    { 
        return NSLOCTEXT("FVInteraction", "ItemReqUnmet", "Missing required item(s)."); 
    }
}