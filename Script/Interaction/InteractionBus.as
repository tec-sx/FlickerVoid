struct FInteractionTarget
{
    UPROPERTY()
    FGameplayTag InteractableType;

    UPROPERTY()
    int Priority;

    UPROPERTY()
    AActor Actor;

    int opCmp(const FInteractionTarget& Other) const
    {
        if (Priority < Other.Priority)
            return -1;
        if (Priority > Other.Priority)
            return 1;
        return 0;
    }

    int opEquals(const FInteractionTarget& Other) const
    {
        if (InteractableType == Other.InteractableType && Actor.GetName() == Other.Actor.GetName())
            return 1;
        else
            return 0;
    }

    bool IsValid() const
    {
        return InteractableType.IsValid() && Actor != nullptr;
    }
}

event void FInteractionZoneEvent(FGameplayTag EventType, FInteractionTarget Payload);

class UInteractionBus : UGameInstanceSubsystem
{
    UPROPERTY()
    FInteractionZoneEvent OnInteractionZoneEvent;
}