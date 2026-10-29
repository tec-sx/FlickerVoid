# FVAttributeSystem setup

Attributes (Perception, Stealth, Charisma, Sanity...) are Gameplay Ability System attributes. Their values live in attribute sets on the character's ability system component, and Gameplay Effects change them (starting values, perks, kinks, statuses, outfit bonuses). This plugin adds what GAS lacks: a definition per attribute for its name, range and UI, clamping to that range, change events by definition, saving, and skill checks.

Modules: `FVAttributeSystem`, `FVAttributeSystemDebug`, `FVAttributeSystemEditor`.

## 1. Quick start

1. Declare the attributes in an attribute set deriving from `UFVAttributeSet` (AngelScript or C++), one `FGameplayAttributeData` property per attribute. Example: `Script/Gameplay/Protagonist/Identity/Personality/SkillAttributeSet.as`.
2. Make one `FVAttributeDefinition` per attribute and pick its **Attribute** (e.g. `SkillAttributeSet.Perception`).
3. Make an instant Gameplay Effect with the starting values (`GE_Init_Skills`, Override modifiers).
4. Add the attribute set (**Attribute Sets**) and the init effect (**Gameplay Effects**) to an `FVAbilitySet`, and that ability set to the character definition's **Abilities** fragment. Attribute sets are granted before effects, so the init effect lands.
5. Read values with `FVAttribute::GetValue(Actor, Definition)`; gate content with the **Attribute** condition or **Skill Check (Passive)**.

## 2. Attributes

Add an attribute by adding one property to an attribute set and one definition asset; no C++ needed.

```
class USkillAttributeSet : UFVAttributeSet
{
    UPROPERTY(BlueprintReadOnly, Category = "Skills")
    FGameplayAttributeData Perception;
}
```

Group attributes into sets along the ontology (skills, mental state, physical state), so a character only gets the sets it needs. Sets deriving from `UFVAttributeSet` clamp base and current values to the definition's **Min**/**Max** and round when **Integer** is set; attributes without a definition are not clamped.

## 3. Data assets

| Asset | Class | Key fields |
|---|---|---|
| `DA_Attribute_Perception`, `DA_Attribute_Stealth`... | `FVAttributeDefinition` | **Attribute** (the set property), **Display**, **Category** (`Attribute.Skill`, `Attribute.Mental`...), **Min**, **Max**, **Integer** |
| `GE_Init_Skills`, `GE_Init_Guard`... | Gameplay Effect, Instant | Override modifiers with each starting value. Layer archetypes by granting several (a civilian base, then guard changes) |
| `GE_Perk_SharpEyes`, `GE_Status_Drunk`... | Gameplay Effect, Infinite or Has Duration | Add/Multiply/Override modifiers, stacking, tag requirements, periodic changes |
| `DA_Check_Lockpicking`, `DA_Check_Persuade`... | `FVCheckDefinition` | **Attribute** tested, **Roll** (None, 2d6, d10, d20), **Modifiers** (label + conditions + bonus, e.g. +2 while wearing a uniform) |

Definitions are found through the asset manager: `Config/DefaultGame.ini` scans `/Game` for the `FVAttributeDefinition` primary asset type and always cooks them. Two definitions for the same attribute log a warning; the first wins.

Create category tags under `Attribute.` for grouping in UI and debug output.

## 4. Components and characters

The character needs an ability system component (`UFVAbilitySystemComponent` on the player). Values, conditions and checks read it directly.

**FV Attribute Component** is optional. Add it where something listens for changes (HUD, character page) or where base values must be saved:
- **On Attribute Changed** (definition, old value, new value) fires for every defined attribute the ability system holds.
- Base values are saved when the actor also has an **FV Saveable Component** (see FVStorySystem) and restored after loading. Active Gameplay Effects are not saved; whoever applies them (equipment, quests, statuses) re-applies them.

`UFVAttributeSubsystem::Get().GetDefinitions()` lists every definition, for UI lists.

## 5. Checks

```
Total = attribute value + passing modifier bonuses + roll; success when Total >= Difficulty
```
- `Roll Check` (rolls), `Preview Check` (no roll, for UI) and `Success Chance` on `UFVCheckLibrary` (`FVCheck::` in AngelScript). Each returns a breakdown for the UI.
- The attribute is the instigator's current value, with active Gameplay Effects applied.
- **Skill Check (Passive)** condition: passes when the check succeeds without rolling. Use it on dialogue choices and interaction offers.

## 6. Conditions and effects

| Conditions | Effects |
|---|---|
| Attribute (compare the current value; on Instigator or Target), Skill Check (Passive) | Modify Attribute (delta on the base value), Set Attribute (base value), Apply Gameplay Effect (class + level), Remove Gameplay Effect (every active instance of a class) |

Use **Apply Gameplay Effect** for anything temporary or removable (perks, kinks, statuses); **Modify Attribute** for permanent growth, such as a skill raised by a quest.

## 7. AngelScript

| Call | Does |
|---|---|
| `FVAttribute::GetValue(Actor, Definition)`, `GetBaseValue`, `HasAttribute` | Read through the actor's ability system |
| `FVAttribute::SetBaseValue(Actor, Definition, Value)`, `AddToBaseValue` | Permanent base change, clamped by the set |
| `UFVAttributeSubsystem::Get().FindDefinition(Attribute)` | Definition for a `FGameplayAttribute` |

## 8. Debug

| CVar / command | Effect |
|---|---|
| `FVCvar.Attributes.Debug.HUD 1` | The player's defined attributes on screen (value and base) |
| `FV.Attributes.Set <AttributeIdOrAsset> <Value>` | Set a base value on the player |
| `FV.Attributes.Roll <CheckIdOrAsset> [Difficulty]` | Roll a check as the player and log the breakdown |

The engine's `showdebug abilitysystem` lists every attribute and active effect on the player.

## 9. Moving from the old attribute component (one time)

The old data-asset attributes are gone: `FVAttributeSetDefinition`, the **Attributes** character fragment, *FlickerVoid > Attributes* settings, `Add Modifier` and `Remove Modifiers From Source`.

1. Declare each existing attribute in a script attribute set (start from `SkillAttributeSet.as`).
2. Open each `FVAttributeDefinition`: pick its **Attribute**; **Default Value** is gone, so move it into an init Gameplay Effect.
3. Replace each attribute set definition with an instant init Gameplay Effect, and each character's **Attributes** fragment with an ability set on its **Abilities** fragment. Then delete the old set assets.
4. Re-save check definitions and anything holding attribute conditions or effects.
