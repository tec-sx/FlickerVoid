class AItem : AActor
{
    UPROPERTY(BlueprintReadWrite, Meta=(MakeEditWidget = true))
    FVector InteractionPointOffset;


    UFUNCTION(BlueprintPure)
    FVector GetInteractionPoint()
    {
        return GetActorLocation() - InteractionPointOffset;
    }
}