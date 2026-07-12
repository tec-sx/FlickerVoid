struct FFVCharacterIdentity
{
    UPROPERTY(EditDefaultsOnly)
    FString Name;

    UPROPERTY(EditDefaultsOnly)
    FName SpeakerId;

    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UFVFaction> Affiliation;

    UPROPERTY()
    FGameplayTagContainer Tags;
}