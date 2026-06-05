# FVoid Ability System

GAS-based attribute and ability framework built on modified Lyra Starter Game patterns.

## Overview

The ability system handles:

- Character Attributes: Social, personality, mood, physical stats
- Protagonist Attributes: Sanity, addiction, memory/identity
- Gameplay Effects: Attribute modification, status changes
- Ability Management: Granting and activating abilities

## Flowcharts

- [FC_AbilitySystem_Architecture.md](FC_AbilitySystem_Architecture.md) - Overall system architecture
- [FC_AbilitySystem_OwnershipPattern.md](FC_AbilitySystem_OwnershipPattern.md) - ASC ownership and initialization
- [FC_AbilitySystem_AttributeFlow.md](FC_AbilitySystem_AttributeFlow.md) - Attribute modification processing
- [FC_AbilitySystem_CharacterAttributes.md](FC_AbilitySystem_CharacterAttributes.md) - Base character attribute structure
- [FC_AbilitySystem_ProtagonistAttributes.md](FC_AbilitySystem_ProtagonistAttributes.md) - Player psychological attributes
- [FC_AbilitySystem_GameplayEffects.md](FC_AbilitySystem_GameplayEffects.md) - Gameplay effect patterns
- [FC_AbilitySystem_DataAssetBridge.md](FC_AbilitySystem_DataAssetBridge.md) - Data asset to GAS bridge

## Architecture

### Ownership Pattern

The Ability System Component (ASC) is owned by AFVoidPlayerState for the player character. This allows attributes to persist across pawn changes.

### Key Classes

| Class | Location | Purpose |
|-------|----------|---------|
| UFVoidAbilitySystemComponent | AbilitySystem/ | Core ASC with project extensions |
| UFVoidAbilitySet | AbilitySystem/ | Data asset defining granted abilities |
| UFVoidCharacterAttributeSet | AbilitySystem/Attributes/ | Base character attributes |
| UFVoidProtagonistAttributeSet | AbilitySystem/Attributes/ | Player psychological attributes |
| UCharacterAttributeComponent | Character/ | Bridge between data assets and GAS |
| UFVoidCharacterDataAsset | Character/ | Designer-editable character profile |

## Character Attributes (UFVoidCharacterAttributeSet)

Used by all characters including NPCs.

### Social Attributes

| Attribute | Range | Description |
|-----------|-------|-------------|
| Charisma | 0-1 | Social charm and likability |
| Intimidation | 0-1 | Ability to threaten/coerce |
| Deception | 0-1 | Skill at lying |
| Persuasion | 0-1 | Convincing others |
| Empathy | 0-1 | Understanding emotions |

### Personality Attributes

| Attribute | Range | Description |
|-----------|-------|-------------|
| Aggression | 0-1 | Tendency toward violence |
| Courage | 0-1 | Bravery in danger |
| Patience | 0-1 | Tolerance for waiting |
| Openness | 0-1 | Receptiveness to new ideas |
| Conscientiousness | 0-1 | Attention to detail |
| Neuroticism | 0-1 | Emotional instability |
| Libido | 0-1 | Sexual drive |

### Mood Attributes

| Attribute | Range | Description |
|-----------|-------|-------------|
| Happiness | 0-1 | Current joy level |
| Stress | 0-1 | Current stress level |
| Arousal | 0-1 | Sexual arousal |
| Fatigue | 0-1 | Tiredness level |
| Hunger | 0-1 | Need for food |

### Physical Attributes

| Attribute | Range | Description |
|-----------|-------|-------------|
| Intoxication | 0-1 | Current intoxication |
| Alertness | 0-1 | Awareness level |

## Protagonist Attributes (UFVoidProtagonistAttributeSet)

Player-only psychological system.

### Sanity Attributes

| Attribute | Range | Description |
|-----------|-------|-------------|
| Sanity | 0-1 | Current mental stability (0 = broken) |
| MaxSanity | 0-1 | Maximum capacity (trauma reduces) |
| SanityRecoveryRate | 0+ | Natural recovery per game hour |
| SanityResistance | 0-1 | Resistance to sanity loss |

### Sanity States

| Tag | Range | Effects |
|-----|-------|---------|
| Sanity.State.Stable | 0.8+ | Normal function |
| Sanity.State.Anxious | 0.6-0.8 | Minor effects |
| Sanity.State.Disturbed | 0.4-0.6 | Dialogue changes |
| Sanity.State.Unstable | 0.2-0.4 | Quest blocks |
| Sanity.State.Breaking | 0.1-0.2 | Severe impairment |
| Sanity.State.Broken | 0.1 or less | Major blocks |

### Addiction Attributes

| Attribute | Range | Description |
|-----------|-------|-------------|
| AlcoholDependency | 0-1 | Alcohol addiction level |
| SedativeDependency | 0-1 | Sedative addiction level |
| StimulantDependency | 0-1 | Stimulant addiction level |
| WithdrawalSeverity | 0-1 | Current withdrawal intensity |
| DaysClean | 0+ | Days since last substance use |

### Addiction States

| Tag | Description |
|-----|-------------|
| Addiction.State.Clean | No dependency (all below 0.1) |
| Addiction.State.Casual | Low dependency (0.1-0.3) |
| Addiction.State.Dependent | Moderate (0.3-0.6) |
| Addiction.State.Addicted | High dependency (above 0.6) |
| Addiction.State.Withdrawal | Active withdrawal symptoms |
| Addiction.State.Recovering | In recovery process |

### Memory Attributes

| Attribute | Range | Description |
|-----------|-------|-------------|
| IdentityRecovery | 0-1 | Progress toward full identity |
| MemoryFragmentsFound | 0+ | Total fragments discovered |
| MemoryClarity | 0-1 | Current memory clarity |

## Attribute Declaration Pattern

All attributes follow the standard GAS pattern using FGameplayAttributeData with ATTRIBUTE_ACCESSORS macro which generates:

- GetAttributeNameAttribute() - Returns the FGameplayAttribute
- GetAttributeName() - Returns current value
- SetAttributeName() - Sets value directly
- InitAttributeName() - Sets base value

## Attribute Processing

### PreAttributeChange

Called before any attribute change. Used for clamping values to valid ranges.

### PostGameplayEffectExecute

Called after a gameplay effect modifies attributes. Used for:

- Processing meta attributes (like damage)
- Triggering side effects
- Updating state tags based on thresholds

## Character Data Asset (UFVoidCharacterDataAsset)

Designer-editable data asset for character profiles.

| Field | Type | Description |
|-------|------|-------------|
| CharacterId | FName | Unique identifier |
| DisplayName | FText | Character name |
| DefaultAttributes | Map | Initial attribute values |
| PersonalityTags | TagContainer | Personality traits |
| AbilitySets | Array | Abilities to grant |

## Designer Workflow

### Creating a Character Profile

1. Create UFVoidCharacterDataAsset in Content Browser
2. Set CharacterId and DisplayName
3. Configure DefaultAttributes map with initial values
4. Add relevant PersonalityTags
5. Assign AbilitySets for character abilities
6. Reference from UFVoidPawnData

### Creating Gameplay Effects

1. Create Blueprint subclass of UGameplayEffect
2. Add attribute modifiers
3. Configure duration (instant, duration, infinite)
4. Set tags for conditions and effects
5. Apply via AbilitySystemComponent

## See Also

- Story System Documentation: ../StorySystem/README.md
- Dialogue System Documentation: ../DialogueSystem/README.md

