/**
 * Skills of the protagonist and NPCs. Each property is one GAS attribute; give each an FV Attribute
 * Definition for its name, range and UI. Starting values come from an instant init Gameplay Effect.
 */
class USkillAttributeSet : UFVAttributeSet
{
    UPROPERTY(BlueprintReadOnly, Category = "Skills")
    FGameplayAttributeData Perception;

    UPROPERTY(BlueprintReadOnly, Category = "Skills")
    FGameplayAttributeData Stealth;

    UPROPERTY(BlueprintReadOnly, Category = "Skills")
    FGameplayAttributeData Charisma;
}
