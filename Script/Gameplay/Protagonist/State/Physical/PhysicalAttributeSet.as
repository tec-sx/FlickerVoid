/**
 * Physical state of the protagonist and NPCs. Each property is one GAS attribute; give each an FV Attribute
 * Definition for its name, range and UI. Starting values come from an instant init Gameplay Effect.
 */
class UPhysicalAttributeSet : UFVAttributeSet
{
    UPROPERTY(BlueprintReadOnly, Category = "Physical")
    FGameplayAttributeData Health;

    UPROPERTY(BlueprintReadOnly, Category = "Physical")
    FGameplayAttributeData Intoxication;
}
