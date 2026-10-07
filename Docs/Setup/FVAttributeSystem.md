# FVAttributeSystem setup

Data-defined attributes (Perception, Stealth, Charisma, Health...), starting sets, temporary modifiers and skill checks. New attributes are new data assets; no C++ needed.

Modules: `FVAttributeSystem`, `FVAttributeSystemDebug`, `FVAttributeSystemEditor`.

## 1. Quick start

1. Make a few attribute definitions and one attribute set listing them.
2. Set that set as *FlickerVoid > Attributes > Default Attribute Set*.
3. Add **FV Attribute Component** to the player and to NPCs that need attributes.
4. Read values with `Get Value`; gate content with the **Attribute** condition or **Skill Check (Passive)**.

## 2. Data assets

| Asset | Class | Key fields |
|---|---|---|
| `DA_Attribute_Perception`, `DA_Attribute_Stealth`... | `FVAttributeDefinition` | **Category** (`Attribute.Skill`, `Attribute.Vital`...), **Default Value**, **Min**, **Max**, **Integer** (rounds the final value) |
| `DA_AttributeSet_Civilian`, `DA_AttributeSet_Guard`... | `FVAttributeSetDefinition` | **Parents** (applied first, in order) and **Attributes** (attribute + value). Build archetypes by layering sets |
| `DA_Check_Lockpicking`, `DA_Check_Persuade`... | `FVCheckDefinition` | **Attribute** tested, **Roll** (None, 2d6, d10, d20), **Modifiers** (label + conditions + bonus, e.g. +2 while wearing a uniform) |

Create category tags under `Attribute.` for grouping in UI and debug output.

## 3. Components and characters

**FV Attribute Component** on any actor with attributes. Starting values are applied in this order, later ones winning:
1. The component's **Starting Set**, or the project's **Default Attribute Set** when empty.
2. The character definition's **Attributes** fragment (set + overrides), when the actor has an FV Identity Component.
3. The component's **Starting Values**.

To give a named NPC unique stats, add the **Attributes** fragment to their `FVCharacterDefinition` rather than editing their Blueprint.

### Modifiers
`Add Modifier` takes an attribute, an operation (Add, Multiply, Override) and a **Source** tag. `Remove Modifiers From Source` removes them all, so whoever applies them (an outfit, a status, a quest) can clean up by tag. Final value = base, then Add, then Multiply; Override wins.

Base values are saved when the actor also has an **FV Saveable Component** (see FVStorySystem); modifiers are rebuilt at runtime by their owners.

## 4. Checks

```
Total = attribute value + passing modifier bonuses + roll; success when Total >= Difficulty
```
- `Roll Check` (rolls), `Preview Check` (no roll, for UI) and `Success Chance` on `UFVCheckStatics` (`FVCheck::` in AngelScript). Each returns a breakdown for the UI.
- **Skill Check (Passive)** condition: passes when the check succeeds without rolling. Use it on dialogue choices and interaction offers.

## 5. Conditions and effects

| Conditions | Effects |
|---|---|
| Attribute (compare a value; on Instigator or Target), Skill Check (Passive) | Modify Attribute (delta on the base value), Set Attribute |

## 6. Settings

*FlickerVoid > Attributes* (also *FlickerVoid menu > Attribute System*):

| Setting | Meaning |
|---|---|
| Default Attribute Set | Applied to every attribute component without its own starting set |
| Definition Directory | Folder holding attribute definitions, so debug commands can find them by name |

The menu's *Default Attribute Set* entry opens that asset.

## 7. Debug

| CVar / command | Effect |
|---|---|
| `FVCvar.Attributes.Debug.HUD 1` | The player's attributes on screen |
| `FV.Attributes.Set <AttributeIdOrAsset> <Value>` | Set a base value on the player |
| `FV.Attributes.Roll <CheckIdOrAsset> [Difficulty]` | Roll a check as the player and log the breakdown |
