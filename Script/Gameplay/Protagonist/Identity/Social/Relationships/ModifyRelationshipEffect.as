/** Changes the player's relationship with a character. Empty Character means the character of the context Target. */
class UModifyRelationshipEffect : UFVEffect
{
    UPROPERTY(EditAnywhere, Category = "Effect")
    UFVCharacterDefinition Character;

    UPROPERTY(EditAnywhere, Category = "Effect")
    int Delta = 0;

    UFUNCTION(BlueprintOverride)
    void Apply(FFVConditionContext Context) const
    {
        UFVCharacterDefinition Resolved = Character != nullptr ? Character : UFVIdentityComponent::GetActorDefinition(Context.Target);
        UFVScaleDefinition Relationship = FVSocial::GetRelationshipScale(Resolved);
        if (Relationship != nullptr)
        {
            FVScale::ModifyScale(Relationship, Delta);
        }
    }

    UFUNCTION(BlueprintOverride)
    FText GetDescription() const
    {
        FString CharacterName = Character != nullptr ? Character.Display.Name.ToString() : "Target";
        return FText::FromString(f"Relationship with {CharacterName} {Delta}");
    }
}
