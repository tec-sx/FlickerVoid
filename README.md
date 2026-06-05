# FlickerVoid

Developed with Unreal Engine 5.7


# Project Context

## 1. Project Overview

This is a **AAA-style, third-person, story-driven single-player game** built with **Unreal Engine 5.7**.

The project is written **primarily in C++**, with C++ providing robust, scalable systems and **Blueprints used strictly for game design and content assembly**, not complex logic. The goal is to minimize blueprint complexity while maintaining designer-friendly workflows.

### Core Gameplay Pillars

- Combat is **NOT** an important game mechanic
- Strong focus on:
  - narrative progression
  - psychological systems
  - player choice and consequence
- Progression is **state-based**, not power-based

---

## 2. Narrative & Player Experience

The story follows a **young adult woman suffering from total memory loss**.

The central mystery revolves around:
- who she is
- who she was
- who she may become

Story progression is driven by:
- character interactions
- dialogue and communication
- information gathering
- infiltration and social manipulation
- performing morally ambiguous or “shady” errands to earn trust, access, or resources

---

## 3. Psychological State & Sanity System

A core gameplay pillar is the **psychological state of the protagonist**, represented through a **Sanity system**.

- Player choices directly affect sanity
- Substance use (e.g. alcohol, drugs) can:
  - temporarily alter perception or social outcomes
  - lead to **addiction states**
- Addiction or unstable sanity can:
  - block story progression
  - restrict dialogue options
  - force personal rehabilitation before narrative continuation

### Progression Philosophy

Progression often requires:
- side quests
- repetitive or mundane real-life tasks
- social repair and trust rebuilding
- meeting explicit psychological or narrative thresholds

The design intentionally mirrors real-life effort, dependency, and consequence.

---

## 4. Key Systems & Architecture

### Ability & Input System

- **FVoidAbilitySystemComponent**
  - Based on the Lyra Starter Game
  - Modified and extended for project-specific needs
  - Built on Unreal’s Gameplay Ability System (GAS)

- **FVoidAbilitySet (Data Asset)**
  - Defines which abilities are granted to an actor

- **FVoidInputConfig (Data Asset)**
  - Maps input actions to Gameplay Tags
  - Each input action corresponds to a gameplay ability

Input handling is **fully tag-driven**.

---

### Player & Character Architecture

- **FVoidPlayerState**
  - Owns and configures the Ability System
  - Stores player-related persistent data

- **FVoidCharacter**
  - Base character class
  - Currently responsible for movement via **UE5 MoverComponent**
  - Intended as the base for most characters

- **FVoidProtagonistComponent** *(mandatory for the player character)*
  - Configures player input mappings
  - Forwards input to `FVoidCharacter`
  - Acts as the glue between player intent and character behavior

---

### Inventory & Interaction

- **FVoidInventoryComponent**
  - Manages items, resources, and progression-relevant objects
  - Stores item instances with runtime data (quantity, durability, custom tags)

- **FVoidItemInteractionComponent**
  - Handles item-based and world interactions
  - Used for narrative gating and trust-based progression

#### Item System Architecture

The inventory system uses a **tag-driven, effect-based architecture** that combines the flexibility of tags with type-specific properties:

**Core Components:**

- **UFVoidItemDataAsset** (Data Asset)
  - Designer-friendly wrapper for item templates
  - Created in Content Browser for each item type
  - Contains `FFVoidItemDataTemplate` with all item data

- **FFVoidItemDataTemplate** (Struct)
  - Core item data (ID, name, icon, mesh, weight, value)
  - Tag-based categorization (Item.Category.*, Item.Rarity.*, Item.Property.*)
  - Array of `UFVoidItemEffect` for type-specific behaviors
  - Modular actions system (Consume, Equip, Inspect, etc.)

- **FFVoidItemInstance** (Struct)
  - Runtime instance of an item in inventory
  - Tracks quantity, durability, unique ID
  - Can have instance-specific tags (Instance.Enchanted, Instance.Damaged)

**Item Effect System:**

Items use **composable, Blueprint-extensible effects** to define their behavior:

- **UFVoidItemEffect** (Base class, Blueprint-extensible)
  - Abstract base for all item effects
  - Can be subclassed in Blueprint for custom effects
  - Applied when item is used/consumed/equipped

- **UFVoidConsumableEffect** (Base consumable)
  - Charges, effect strength

- **UFVoidFoodEffect** (Affects player physique)
  - Energy (calories, restores stamina)
  - Fat content (affects weight gain/loss)
  - Sugar (quick energy, metabolism)
  - Protein (muscle mass, recovery)

- **UFVoidBeverageEffect** (Hydration and intoxication)
  - Hydration amount
  - Is alcoholic flag
  - Energy (for energy drinks)
  - Intoxication strength
  - Addiction risk

- **UFVoidDrugEffect** (Mental state and sanity)
  - Intoxication strength
  - Addiction risk
  - Effect duration
  - Mental clarity modifier
  - Sanity modifier

- **UFVoidEquipmentEffect** (Wearable items)
  - Equip socket (Head, Chest, Hands, etc.)
  - Max durability and wear rate
  - Armor value
  - Granted tags (Character.Outfit.Formal, Character.Outfit.Casual)

**Design Philosophy:**

- **Tag-driven categorization**: Flexible, hierarchical organization
- **Effect composition**: Items can have multiple effects (e.g., alcoholic beverage that also restores energy)
- **Blueprint extensibility**: Designers can create new effect types without C++ changes
- **Data-driven**: All item properties configured via Data Assets
- **Separation of concerns**: Tags for categorization, effects for behavior

**Example Usage:**

```cpp
// Get all food effects from an item
TArray<UFVoidFoodEffect*> FoodEffects = ItemTemplate.GetEffectsOfType<UFVoidFoodEffect>();

// Check if item is alcoholic
bool bIsAlcoholic = ItemTemplate.ItemTags.HasTag(
    FGameplayTag::RequestGameplayTag(FName("Item.Category.Consumable.Beverage.Alcoholic"))
);
```

---

## 5. Design Constraints & Principles

- **Single-player only**
  - No replication
  - No multiplayer assumptions
- **Strongly data-driven**
  - Systems configured via Data Assets
- **Gameplay Tag–centric architecture**
  - Tags drive abilities, states, conditions, and logic
- **Clear separation of concerns**
  - Input ≠ Movement ≠ Animation
- Systems should be:
  - extensible
  - debuggable
  - scalable toward AAA complexity

---

## 6. How the AI Assistant Is Used

The AI assistant is primarily used for:

- high-level architecture and system scaffolding
- validating designs against professional studio practices
- reviewing long-term maintainability and scalability
- assisting with:
  - data-oriented design
  - GAS patterns
  - psychological and narrative systems

The assistant should assume:
- senior-level engineering standards
- a long project lifespan
- iterative system expansion

---

## 7. Expectations From the AI Assistant

When responding, the AI should:

- prefer industry-standard patterns
- avoid tutorial-level solutions unless explicitly requested
- consider Unreal Engine internals and scalability
- ask clarification questions only when absolutely necessary
- optimize for clarity, structure, and long-term design health

---

## One-Line Summary

A non-combat, AAA-style narrative game in UE5 focused on psychological state, identity, and consequence-driven progression, built with a data-driven, GAS-based C++ architecture and designer-friendly Blueprints.