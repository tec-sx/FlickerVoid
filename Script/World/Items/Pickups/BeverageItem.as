class ABeverageItem : AItem
{
    UPROPERTY(DefaultComponent)
    UInteractableResponseComponent_Consume ConsumeComponent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUsed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Amount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UsageReduction = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool InfiniteAmound = false;
    
    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        // TODO: Somehow Handle Usage with the drink offer on the interactable component.
    }

    UFUNCTION()
    private void Drink()
    {
        if (InfiniteAmound == true)
        {
            return;
        }

        Amount -= UsageReduction;
        Print("Amount Left: " + Amount);

        if (Amount <= 0)
        {
            bIsUsed = true;
        }
    }
}