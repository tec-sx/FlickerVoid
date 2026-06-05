# FVCharacter Module Reorganization

## Date: May 18, 2026

## Objective
Reorganized FVCharacter module from **function-based** structure to **context-based** structure for better maintainability and discoverability.

## New Structure

### ✅ Context-Based Organization

```
FVCharacter/
├── Public/
│   ├── Animation/           [All animation-related]
│   │   ├── FVAnimationConfigData.h
│   │   ├── FVAnimationTypes.h
│   │   ├── FVCharacterAnimInstance.h
│   │   ├── FVCharacterRigSettings.h
│   │   └── FVLocomotionStyleData.h
│   │
│   ├── Character/           [All core character-related]
│   │   ├── FVCharacter.h
│   │   ├── FVCharacterArchetypeData.h
│   │   ├── FVCharacterAttributeComponent.h
│   │   ├── FVCharacterDataAsset.h
│   │   ├── FVCharacterStateManager.h
│   │   └── FVCharacterTags.h
│   │
│   ├── Movement/            [All movement-related]
│   │   ├── FVCharacterMovementComponent.h
│   │   ├── FVMovementHandlerBase.h
│   │   ├── FVMovementHandlerConfigBase.h
│   │   ├── FVMovementHandlerData.h
│   │   ├── FVMovementHandlerInfo.h
│   │   └── FVMovementTypes.h
│   │
│   └── RigUnits/            [Control rig units]
│       ├── FVRigUnit_ArmSwingPhase.h
│       ├── FVRigUnit_HipSwing.h
│       └── FVRigUnit_LegStepWidth.h
│
└── Private/
    ├── Animation/
    ├── Character/
    ├── Movement/
    └── RigUnits/
```

## Include Path Changes

### Character Context

| Old Path | New Path |
|----------|----------|
| `#include "FVCharacter.h"` | `#include "Character/FVCharacter.h"` |
| `#include "FVCharacterStateManager.h"` | `#include "Character/FVCharacterStateManager.h"` |
| `#include "FVCharacterTags.h"` | `#include "Character/FVCharacterTags.h"` |
| `#include "FVCharacterDataAsset.h"` | `#include "Character/FVCharacterDataAsset.h"` |
| `#include "Data/FVCharacterArchetypeData.h"` | `#include "Character/FVCharacterArchetypeData.h"` |
| `#include "Components/FVCharacterAttributeComponent.h"` | `#include "Character/FVCharacterAttributeComponent.h"` |

### Movement Context

| Old Path | New Path |
|----------|----------|
| `#include "Components/FVCharacterMovementComponent.h"` | `#include "Movement/FVCharacterMovementComponent.h"` |
| `#include "Data/FVMovementHandlerData.h"` | `#include "Movement/FVMovementHandlerData.h"` |

### Animation Context (No Changes)

Animation files were already in context-based structure:
- `#include "Animation/FVAnimationConfigData.h"` ✅
- `#include "Animation/FVAnimationTypes.h"` ✅
- `#include "Animation/FVCharacterAnimInstance.h"` ✅

## Benefits of Context-Based Structure

### 1. **Better Discoverability**
```
❌ OLD: "Where is the movement data asset?"
   → Could be in Data/, Components/, Movement/

✅ NEW: "Where is the movement data asset?"
   → Obviously in Movement/
```

### 2. **Logical Grouping**
```
Character/
  - Core character class
  - Character data assets
  - Character state management
  - Character-specific components

Movement/
  - Movement component
  - Movement handlers
  - Movement data assets
  - Movement types/configs

Animation/
  - Animation instance
  - Animation data assets
  - Animation configs
  - Animation types
```

### 3. **Easier Navigation**
- All character-related code in one place
- All movement-related code in one place
- All animation-related code in one place
- No need to jump between Components/, Data/, etc.

### 4. **Scalability**
Easy to add new contexts:
```
Combat/          (future)
  - FVCombatComponent
  - FVWeaponData
  - FVCombatTypes

Inventory/       (future)
  - FVInventoryComponent
  - FVItemData
  - FVItemTypes
```

### 5. **Module Boundaries**
Clear separation makes it easier to:
- Extract subsystems into separate modules
- Identify dependencies
- Refactor without breaking everything

## Files Moved

### Character Context (11 files)
```
FVCharacter.h/cpp
FVCharacterStateManager.h/cpp
FVCharacterTags.h/cpp
FVCharacterDataAsset.h/cpp
FVCharacterArchetypeData.h/cpp  (from Data/)
FVCharacterAttributeComponent.h/cpp  (from Components/)
```

### Movement Context (4 files)
```
FVCharacterMovementComponent.h/cpp  (from Components/)
FVMovementHandlerData.h/cpp  (from Data/)
```

### Animation Context (0 files moved)
```
Already in correct structure ✅
```

## Migration Checklist

- [x] Create new folder structure
- [x] Move Character-context files to Character/
- [x] Move Movement-context files to Movement/
- [x] Remove empty Data/ and Components/ folders
- [ ] Update all #include paths (automated next)
- [ ] Verify compilation
- [ ] Update documentation references

## Next Steps

1. **Automated Include Path Update**
   - Search all source files
   - Replace old include paths with new paths
   - Verify no broken includes

2. **Test Compilation**
   - Clean build
   - Verify all includes resolve correctly
   - Fix any remaining path issues

3. **Update Documentation**
   - Update architecture docs
   - Update README files
   - Update onboarding guides

## Reverting (If Needed)

To revert to old structure:
```powershell
# Move Character files back
Move-Item "Character/*" "."
Move-Item "Character/FVCharacterArchetypeData.*" "Data/"
Move-Item "Character/FVCharacterAttributeComponent.*" "Components/"

# Move Movement files back
Move-Item "Movement/FVCharacterMovementComponent.*" "Components/"
Move-Item "Movement/FVMovementHandlerData.*" "Data/"

# Remove Character folder
Remove-Item "Character"
```

## Conclusion

**Rationale:**
- Context-based structure is more intuitive
- Easier to find related code
- Better matches how developers think
- Industry standard (Unity, Godot, most engines use context-based)

**Impact:**
- 15 files reorganized
- Zero functional changes
- Only include paths need updating
- Compilation should succeed immediately after path updates

**This is a quality-of-life improvement that pays dividends as the project scales!** 🚀

