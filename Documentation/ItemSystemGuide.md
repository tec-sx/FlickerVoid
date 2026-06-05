# FlickerVoid Item System Guide

## Overview

The FlickerVoid item system is a **tag-driven, effect-based architecture** designed for maximum flexibility and designer empowerment. It combines hierarchical gameplay tags for categorization with composable, Blueprint-extensible effects for type-specific behaviors.

---

## Core Architecture

### 1. Item Data Asset (UFVoidItemDataAsset)

**Location**: Content Browser ? Right-click ? Blueprint ? Data Asset ? `FVoidItemDataAsset`

This is what designers interact with. Create one for each item type in your game.

**Contains**:
- Item template data (FFVoidItemDataTemplate)
- Blueprint events for custom behavior (OnItemUsed, OnItemInspected)

### 2. Item Template (FFVoidItemDataTemplate)

The core data structure containing:

**Basic Properties**:
- Item ID (FName)
- Display name and description
- Icon (UI) and 3D mesh (world)
- Weight, value, max stack size

**Tags** (hierarchical, flexible):
```
Item.Category.Consumable.Food.FastFood
Item.Category.Consumable.Beverage.Alcoholic
Item.Category.Consumable.Drug.Prescription
Item.Category.Clothing.Shirt.Formal
Item.Rarity.Common / Rare / Legendary
Item.Property.Stackable / Tradeable / Questable
```

**Effects** (array of UFVoidItemEffect):
- Food effects (nutrition, energy, physique impact)
- Beverage effects (hydration, intoxication, addiction)
- Drug effects (sanity, perception, addiction)
- Equipment effects (durability, armor, granted tags)

**Actions** (modular):
- Consume, Equip, Drop, Inspect, etc.
- Each action has a tag, icon, and consumption flag

### 3. Item Instance (FFVoidItemInstance)

Runtime representation of an item in inventory:
- Reference to the item template
- Current quantity
- Unique instance ID (GUID)
- Instance-specific tags (e.g., "Instance.Enchanted", "Instance.Damaged")
- Durability (0.0 - 1.0)

---

## Item Effect System

### Base Effect Class (UFVoidItemEffect)

All effects inherit from this. It's **Blueprint-extensible**!

```cpp
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class UFVoidItemEffect : public UObject
{
    // Effect name and description (for UI)
    // Effect tags (for querying and filtering)
    
    // Blueprint implementable events:
    UFUNCTION(BlueprintNativeEvent)
    void ApplyEffect(AActor* User, int32 ItemQuantity);
    
    UFUNCTION(BlueprintNativeEvent, BlueprintPure)
    bool CanApplyEffect(AActor* User) const;
};
```

### Available Effect Types

#### 1. Food Effect (UFVoidFoodEffect)

Affects player physique through nutrition:

```cpp
- Energy (float): Calories, restores stamina
- Fat (float): Affects weight gain/loss over time
- Sugar (float): Quick energy, affects metabolism
- Protein (float): Muscle mass, recovery
```

**Example**: Fast food burger
- High energy (500 calories)
- High fat (affects long-term physique)
- Moderate sugar (quick energy spike)
- Low protein

#### 2. Beverage Effect (UFVoidBeverageEffect)

Hydration and optional intoxication:

```cpp
- bIsAlcoholic (bool): Flags as alcoholic beverage
- Energy (float): For energy drinks, sugary beverages
- Hydration (float): Hydration restored
- IntoxicationStrength (float): Alcohol level, affects sanity/perception
- AddictionRisk (float): Cumulative addiction chance
```

**Example**: Beer
- Is alcoholic: true
- Intoxication strength: 20.0
- Addiction risk: 5.0
- Hydration: 30.0 (lower than water)

#### 3. Drug Effect (UFVoidDrugEffect)

Affects sanity, perception, and mental state:

```cpp
- IntoxicationStrength (float): High strength, affects behavior
- AddictionRisk (float): High cumulative risk
- EffectDuration (float): How long effects last (seconds)
- bProvidesMentalClarity (bool): Temporary focus boost
- SanityModifier (float): Positive or negative sanity change
```

**Example**: Prescription medication
- Intoxication strength: 30.0
- Addiction risk: 40.0
- Effect duration: 600.0 (10 minutes)
- Provides mental clarity: true
- Sanity modifier: -5.0 (slight decrease)

#### 4. Equipment Effect (UFVoidEquipmentEffect)

For wearable items with durability:

```cpp
- EquipSocket (FName): "Head", "Chest", "Hands", etc.
- MaxDurability (float): Total durability
- WearRate (float): Durability loss per use/time
- ArmorValue (float): Protection value
- GrantedTags (FGameplayTagContainer): Tags given to wearer
```

**Example**: Formal suit
- Equip socket: "Chest"
- Granted tags: "Character.Outfit.Formal"
- Armor value: 0.0 (no protection)
- Max durability: 100.0
- Wear rate: 0.5 per day

---

## Creating Items

### In C++ (Effect Types)

If you need a new effect type with specific properties:

1. Create a new class inheriting from `UFVoidItemEffect`
2. Add UPROPERTY fields for your custom data
3. Optionally override `ApplyEffect_Implementation` for C++ logic

```cpp
UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class UFVoidMedicineEffect : public UFVoidConsumableEffect
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Medicine")
    float HealthRestored = 50.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Medicine")
    bool bCuresSickness = false;
};
```

### In Blueprint (Custom Effects)

1. Create a Blueprint class inheriting from `FVoidItemEffect` or any subclass
2. Add custom variables
3. Implement `Apply Effect` event for behavior

### Creating an Item Asset

1. **Content Browser**: Right-click ? Miscellaneous ? Data Asset ? `FVoidItemDataAsset`
2. **Name it**: `DA_Item_CoffeeMug`
3. **Configure**:
   - Item ID: `CoffeeMug`
   - Name: "Coffee Mug"
   - Description: "A warm cup of coffee. Provides energy."
   - Icon: Select texture
   - World Mesh: Select static mesh
   - Max Stack Size: 10
   - Weight: 0.5
   - Value: 5

4. **Add Tags**:
   - `Item.Category.Consumable.Beverage`
   - `Item.Rarity.Common`
   - `Item.Property.Stackable`

5. **Add Effects**:
   - Click "+" on Item Effects array
   - Select `FVoidBeverageEffect`
   - Configure:
     - Energy: 100.0
     - Hydration: 50.0
     - Is Alcoholic: false

6. **Add Actions**:
   - Action Name: "Drink"
   - Action Tag: `Item.Action.Consume`
   - Consumes Item: true

---

## Querying Items

### By Category (Tags)

```cpp
// Check if item is food
FGameplayTag FoodTag = FGameplayTag::RequestGameplayTag(FName("Item.Category.Consumable.Food"));
bool bIsFood = ItemTemplate.IsInCategory(FoodTag);

// Get primary category
FGameplayTag Category = ItemTemplate.GetPrimaryCategoryTag();

// Get rarity
FGameplayTag Rarity = ItemTemplate.GetRarityTag();
```

### By Effect Type

```cpp
// Get all food effects
TArray<UFVoidFoodEffect*> FoodEffects = ItemTemplate.GetEffectsOfType<UFVoidFoodEffect>();

// Check for specific effect
TArray<UFVoidDrugEffect*> DrugEffects = ItemTemplate.GetEffectsOfType<UFVoidDrugEffect>();
if (DrugEffects.Num() > 0)
{
    // Item has drug effects
    float AddictionRisk = DrugEffects[0]->AddictionRisk;
}
```

### Complex Queries

```cpp
// Find all alcoholic items in inventory
for (const FFVoidItemInstance& Item : InventoryItems)
{
    if (Item.ItemTemplate)
    {
        TArray<UFVoidBeverageEffect*> BevEffects = 
            Item.ItemTemplate->ItemData.GetEffectsOfType<UFVoidBeverageEffect>();
        
        for (UFVoidBeverageEffect* BevEffect : BevEffects)
        {
            if (BevEffect->bIsAlcoholic)
            {
                // This item is alcoholic
                float IntoxAmount = BevEffect->IntoxicationStrength * Item.Quantity;
                // Apply to sanity system...
            }
        }
    }
}
```

---

## Integration with Gameplay Systems

### With Sanity System

Items affect sanity through:
- **Drug effects**: Direct sanity modifiers
- **Alcohol**: Intoxication affects perception and decision-making
- **Food**: Nutrition affects long-term mental health

### With Physique System

- **Food effects**: Energy, fat, sugar, protein modify body composition
- **Long-term tracking**: Accumulate nutrition data over time
- **Visual changes**: Player model adapts to physique

### With Addiction System

- **Addiction risk**: Cumulative from drugs and alcohol
- **Threshold tracking**: When addiction develops, restrict progression
- **Rehabilitation**: Side quests to overcome addiction

### With Equipment System

- **Granted tags**: `Character.Outfit.Formal` enables certain dialogue options
- **Social perception**: NPCs react to clothing choices
- **Durability**: Items degrade over time, require repair/replacement

---

## Best Practices

### Tag Hierarchy

Use clear, hierarchical tags:
```
? Good: Item.Category.Consumable.Food.FastFood
? Good: Item.Category.Clothing.Shirt.Formal
? Bad:  Item.Food (too vague)
? Bad:  FastFood (not hierarchical)
```

### Effect Composition

Items can have multiple effects:
```cpp
// Energy drink with addiction risk
- UFVoidBeverageEffect (Energy: 150, Hydration: 20)
- UFVoidDrugEffect (AddictionRisk: 10, SanityModifier: -2)
```

### Blueprint vs C++

- **C++**: Core effect types (Food, Beverage, Drug, Equipment)
- **Blueprint**: Custom effects for specific items or mod support
- **Data Assets**: All item instances, configured by designers

### Performance

- Effects are lightweight UObjects
- No runtime instantiation (EditInlineNew, DefaultToInstanced)
- Tag queries are fast (O(1) hash lookups)

---

## Future Extensions

The system is designed for easy expansion:

1. **New Effect Types**: Inherit from UFVoidItemEffect
2. **Custom Actions**: Add new action tags and handlers
3. **Mod Support**: Designers can create new items and effects in Blueprint
4. **Gameplay Effect Integration**: Apply effects via GAS Gameplay Effects

---

## Summary

The FlickerVoid item system provides:

? **Flexibility**: Tag-driven categorization  
? **Extensibility**: Blueprint-based effect creation  
? **Composability**: Multiple effects per item  
? **Designer-friendly**: Data Asset–based workflow  
? **Type-safe**: C++ effects with proper data structures  
? **Performance**: Lightweight, data-oriented design  

This architecture supports the game's psychological systems, narrative progression, and long-term scalability without requiring C++ changes for new item types.
